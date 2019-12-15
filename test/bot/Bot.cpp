#include "BotPCH.h"
#include "Bot.h"
#include "Config.h"
#include "BotAccount.h"
#include "BotPlayer.h"
#include <gideon/clientnet/ServerProxyFactory.h>
#include <gideon/clientnet/ServerProxyConfig.h>
#include <gideon/clientnet/ServerProxy.h>
#include <gideon/3d/3d.h>
#include <iostream>

namespace gideon { namespace bot {

namespace
{

inline uint32_t getTickInterval()
{
    const uint32_t minTickInterval = 1000;
    const uint32_t maxTickInterval = 2000;

    return minTickInterval +
        (rand() % (maxTickInterval - minTickInterval));
}


inline void changeDirection(int8_t& direction, float& dest)
{
    const float boundary = 10000.0f;

    if (dest > boundary) {
        direction = -1;
    }
    else if (dest < -boundary) {
        direction = 1;
    }
    else {
        direction = (rand() % 2) ? -1 : 1;
    }
}

} // namespace

#pragma warning(push)
#pragma warning(disable : 4355)

Bot::Bot(const Config& config, const UserId& userId,
    const UserPassword& userPassword) :
    config_(config),
    userId_(userId),
    userPassword_(userPassword),
    lastTickInterval_(getTickInterval()),
    lastTickCount_(0),
    lastLogoutTime_(time(0)),
    zonePort_(0),
    xDirection_(1),
    yDirection_(1),
    zDirection_(1)
{
    initBot();
}

#pragma warning(pop)

Bot::~Bot()
{
}


void Bot::tick()
{
    {
        const uint32_t currentTickCount = GetTickCount();
        if ((currentTickCount - lastTickCount_) < lastTickInterval_) {
            return;
        }
        lastTickCount_ = currentTickCount;
        lastTickInterval_ = getTickInterval();

        serverProxy_->tick();
    }

    if (serverProxy_->isLoggingInCommunityServer()) {
        return;
    }

    if (serverProxy_->isLoggingInZoneServer()) {
        return;
    }

    if (shouldLoginLoginServer()) {
        login();
        return;
    }

    if (shouldLoginCommunityServer()) {
        loginCommunityServer();
        return;
    }

    if (shouldLoginZoneServer()) {
        loginZoneServer();
        return;
    }

    //if (shouldLogout()) {
    //    std::wcout << L"* " << userId_ <<
    //        L": Logging out...\n";

    //    loggedOut();
    //    disconnect();
    //    return;
    //}

    if (shouldEnterWorld()) {
        enterWorld();        
        return;
    }

    if (shouldMove()) {
        if (botAccount_->shouldMove()) {
            botAccount_->move();
        }
        else {
            setNextDestination();
            botAccount_->moveTo(destination_);
        }

        if (botAccount_->canCastManaSkill()) {
            botAccount_->castManaSkill();
        }
        return;

    }

    // TODO:
}


void Bot::disconnect()
{
    serverProxy_->disconnect();
}


void Bot::initBot()
{
    clientnet::ServerProxyConfig serverProxyConfig;
    serverProxyConfig.loginServerAddress_ = config_.getServerAddress();
    serverProxyConfig.loginServerPort_ = config_.getServerPort();

    serverProxy_ = clientnet::ServerProxyFactory::create(*this, serverProxyConfig);

    botAccount_ = std::make_unique<BotAccount>(*this, *serverProxy_);
}


void Bot::login()
{
    {
        std::wcout << L"* " << userId_ << L": Authenticating(" <<
            config_.getServerAddress().c_str() << L":" << config_.getServerPort() <<
            L")...\n";

        loginResult_ = serverProxy_->logInLoginServer(userId_, userPassword_);
        if (isFailed(loginResult_.errorCode_)) {
            std::wcout << L"* " << userId_ << L": Failed to login(E:" <<
			    loginResult_.errorCode_ << L").\n";
		    loginResult_.reset();
		    zoneIp_.clear();
		    zonePort_ = 0;
            return;
        }

        botAccount_->initialize(loginResult_.accountInfo_);

        std::wcout << L"* " << userId_ << L": logged in LoginServer. " <<L"\n";
    }

    {
        std::wcout << L"* " << userId_ << L": Querying Shard Info.\n";

        const clientnet::QueryShardInfoResult infoResult = serverProxy_->queryShardInfo();
        if (isFailed(infoResult.errorCode_)) {
            std::wcout << L"* " << userId_ << L": Failed to query shard info(E:" <<
                infoResult.errorCode_ << L").\n";
            return;
        }

        shardInfos_ = infoResult.shardInfos_;
    }

    {
        ShardId selectedShardId = invalidShardId;
        const ShardInfo* lastAccessShardInfo =
            getShardInfo(shardInfos_, loginResult_.accountInfo_.lastAccessShardId_);
        if (lastAccessShardInfo != nullptr) {
            selectedShardId = loginResult_.accountInfo_.lastAccessShardId_;
        }
        else {
            if (! shardInfos_.empty()) {
                /// FYI: 첫번째 Shard 선택
                selectedShardId = (*shardInfos_.begin()).shardId_;
            }
        }

        if (! isValidShardId(selectedShardId)) {
            std::wcout << L"* " << userId_ << L": No shard\n";
            return;
        }

        const clientnet::SelectShardResult selectResult =
            serverProxy_->selectShard(selectedShardId);
        if (isFailed(selectResult.errorCode_)) {
            std::wcout << L"* " << userId_ << L": Failed to select shard(E:" <<
                selectResult.errorCode_ << L").\n";
            return;
        }

        botAccount_->setFullUserInfo(selectResult.fullUserInfo_);
    }

    {
        if (! botAccount_->hasCharacter()) {
            std::wcout << L"* " << userId_ << L": Character creating...\n";

            const Nickname nickname = botAccount_->getUserId();
            
            const clientnet::ReserveNicknameResult reserveNicknameResult =
                serverProxy_->reserveNickname(nickname);

            if (isFailed(reserveNicknameResult.errorCode_)) {
                std::wcout << L"* " << userId_ <<
                    L": Failed to reserve Nickname (E" << reserveNicknameResult.errorCode_ << L")\n";
                return;
            }

            const clientnet::CreateCharacterResult createResult =
                serverProxy_->createCharacter(CreateCharacterInfo(loginResult_.accountInfo_.accountId_, ccFighter,  
                CharacterAppearance(1, 1, 1), 51482091, nickname));

            if (isFailed(createResult.errorCode_)) {
                std::wcout << L"* " << userId_ <<
                    L": Failed to create character(E" << createResult.errorCode_ << L")\n";
                return;
            }

            botAccount_->addCharacter(createResult.fullCharacterInfo_);
            std::wcout << L"* " <<  userId_ <<
                L": Character(" << nickname << L") created.\n";
        }
    }

    {
        std::wcout << L"* " << userId_ << L": Shard selecting...\n";

        const ObjectId selectedCharacterId = botAccount_->getLastAccessCharacterId();

        const clientnet::EnterShardResult enterResult =
            serverProxy_->enterShard(selectedCharacterId);
        if (isFailed(enterResult.errorCode_)) {
            std::wcout << L"* " << userId_ << L": Failed to enter shard(E:" <<
                enterResult.errorCode_ << L").\n";
            return;
        }

        botAccount_->prepareCharacter(selectedCharacterId);

        zoneIp_ = enterResult.zoneServerInfo_.zoneServerIp_;
        zonePort_ = enterResult.zoneServerInfo_.zoneServerPort_;
    }
}


void Bot::loginCommunityServer()
{
    std::wcout << L"* " << userId_ << L": Logging in(" <<
        loginResult_.communityServerInfo_.communityServerIp_.c_str() << L":" <<
        loginResult_.communityServerInfo_.communityServerPort_ << L")...\n";

    const ErrorCode errorCode =
        serverProxy_->logInCommunityServer(
            loginResult_.communityServerInfo_.communityServerIp_,
            loginResult_.communityServerInfo_.communityServerPort_);
    if (isFailed(errorCode)) {
        std::wcout << L"* " << userId_ << L": Failed to login(E:" <<
            errorCode << L").\n";
        return;
    }

    std::wcout << L"* " << userId_ << L": Logged in(" <<
        loginResult_.communityServerInfo_.communityServerIp_.c_str() << L":" <<
        loginResult_.communityServerInfo_.communityServerPort_ << L").\n";
}


void Bot::loginZoneServer()
{
    std::wcout << L"* " << userId_ << L": Logging in(" <<
        zoneIp_.c_str() << L":" << zonePort_ << L")...\n";

    const ErrorCode errorCode =
        serverProxy_->logInZoneServer(zoneIp_, zonePort_, invalidRegionCode, invalidObjectId,
        invalidMapCode, Position());
    if (isFailed(errorCode)) {
        loginResult_.reset();
        std::wcout << L"* " << userId_ << L": Failed to login(E:" <<
            errorCode << L").\n";
        return;
    }

    std::wcout << L"* " << userId_ << L": Logged in(" <<
        zoneIp_.c_str() << L":" << zonePort_ << L").\n";
}


void Bot::enterWorld()
{
    std::wcout << L"* " << userId_ << L": World entering...\n";

    const ObjectId selectedCharacterId = botAccount_->getLastAccessCharacterId();
    assert(isValidObjectId(selectedCharacterId));
    botAccount_->setRequestEnterWorld(true);
    botAccount_->selectCharacter(selectedCharacterId);
    botAccount_->enterWorld();
}


void Bot::setNextDestination() 
{
	const ObjectPosition& position = botAccount_->getPlayer().getPosition();
    const float32_t walkDistance = 30;
    destination_.x_ = position.x_ + (esut::random(-0.6f, 0.6f) * walkDistance);
    destination_.y_ = position.y_ + (esut::random(-0.6f, 0.6f) * walkDistance);
    //destination_.x_ = spawnPosition_.x_ - (walkDistance_ / 2.0f) + (esut::randomf() * walkDistance_);
    //destination_.y_ = spawnPosition_.y_ - (walkDistance_ / 2.0f) + (esut::randomf() * walkDistance_);
    //destination_.z_ = ;

    const Position& leftTop = Position(-2048.0f, -2048.0f, 1255.0f);
    const Position& rightBottom = Position(2048.0f, 2048.0f, 1255.0f);
    clamp2d(destination_, leftTop, rightBottom);
}


void Bot::move()
{
    BotPlayer& botPlayer = botAccount_->getPlayer();
    const ObjectPosition destin(getNextPosition(botPlayer.getPosition()));
    botPlayer.move(destin);
}


void Bot::loggedOut()
{
    lastLogoutTime_ = time(0);

    botAccount_->reset();

    loginResult_.reset();
}


bool Bot::shouldLoginCommunityServer() const
{
	return (! shouldLoginLoginServer()) && (! serverProxy_->isLoggedInCommunityServer());
}


bool Bot::shouldLoginZoneServer() const
{
    return (! serverProxy_->isLoggedInZoneServer()) &&
        botAccount_->hasCharacter();
}


bool Bot::shouldEnterWorld() const
{
    return ! botAccount_->isRequestEnterWorld() && ! botAccount_->isEnterWorld();
}


bool Bot::shouldMove() const
{
    return botAccount_->isEnterWorld();
}


ObjectPosition Bot::getNextPosition(const ObjectPosition& position)
{
    const float velocity = float(rand() % 6) + 1;

    ObjectPosition dest(position);

    changeDirection(xDirection_, dest.x_);
    changeDirection(yDirection_, dest.y_);
    //changeDirection(zDirection_, dest.z_);

    dest.x_ = dest.x_ + (velocity * xDirection_);
    dest.y_ = dest.y_ + (velocity * yDirection_);
    //dest.z_ = dest.z_ + (velocity * zDirection_);
    dest.z_ = 1360;
    dest.heading_ = rand() % (maxHeading + 1);
    return dest;
}

// = BotAccountCallback overriding

void Bot::expelledFromServer(ExpelReason reason)
{
    std::wcout << L"* " << botAccount_->getUserId() <<
        L": Expelled(" << reason << L")\n";

    // 처음부터 다시
    serverProxy_->disconnect();
    loggedOut();
}

// = clientnet::ServerProxyCallback overriding

void Bot::onExpelled(ExpelReason reason)
{
    reason;
}


void Bot::onCommunityServerDisconnected()
{
    // TODO: 커뮤니티 서버에 재접해야 한다

    loggedOut();
}


void Bot::onZoneServerDisconnected()
{
    loggedOut();
}

}} // namespace gideon { namespace bot {
