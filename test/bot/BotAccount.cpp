#include "BotPCH.h"
#include "BotAccount.h"
#include "BotPlayer.h"
#include "BotCommunity.h"
#include "BotCallback.h"
#include <gideon/clientnet/ServerProxy.h>
#include <iostream>

namespace gideon { namespace bot {

IMPLEMENT_SRPC_EVENT_DISPATCHER(BotAccount);

BotAccount::BotAccount(BotAccountCallback& callback,
    clientnet::ServerProxy& serverProxy) :
    callback_(callback),
    botPlayer_(std::make_unique<BotPlayer>(serverProxy)),
    botCommunity_(std::make_unique<BotCommunity>(serverProxy)),
    isRequestEnterWorld_(false)
{
    serverProxy.registerRpcForwarder(*this);
    serverProxy.registerRpcReceiver(*this);
}


void BotAccount::initialize(const AccountInfo& accountInfo)
{
	accountInfo_ = accountInfo;

    botCommunity_->initialize(accountInfo_.userId_);
}


void BotAccount::setFullUserInfo(const FullUserInfo& userInfo)
{
    userInfo_ = userInfo;
}


void BotAccount::reset()
{
    userInfo_.reset();
    botPlayer_->reset();
    botCommunity_->reset();
    isRequestEnterWorld_ =false;
}


void BotAccount::addCharacter(const FullCharacterInfo& fullCharacterInfo)
{
    userInfo_.characters_.insert(
        FullCharacterInfos::value_type(fullCharacterInfo.objectId_, fullCharacterInfo));

    if (! isValidObjectId(userInfo_.lastAccessCharacterId_)) {
        userInfo_.lastAccessCharacterId_ = fullCharacterInfo.objectId_;
    }
}


void BotAccount::prepareCharacter(ObjectId characterId)
{
    FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
    assert(characterInfo != nullptr);

    botPlayer_->initialize(accountInfo_.userId_, *characterInfo);
}


bool BotAccount::isCharacterSelected() const
{
    return botPlayer_->isInitialized();
}


bool BotAccount::isEnteredIntoWorld() const
{
    return botPlayer_->isEnteredIntoWorld();
}


bool BotAccount::isEnterWorld() const
{
    return botPlayer_->isEnterWorld();
}


bool BotAccount::isReadyToPlay() const
{
    return botPlayer_->isReadyToPlay();
}

bool BotAccount::shouldMove() const
{
    return botPlayer_->shouldMove();
}


void BotAccount::moveTo(const Position& destination)
{
    return botPlayer_->moveTo(destination);
}


void BotAccount::move()
{
    return botPlayer_->move();
}


bool BotAccount::canCastManaSkill() const
{
    return botPlayer_->canCastManaSkill();
}


void BotAccount::castManaSkill()
{
    botPlayer_->castManaSkill();
}


// = sne::srpc::RpcForwarder overriding

void BotAccount::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    std::wcout << L"BotAccount::onForwarding(" << rpcId.get() << L"," <<
        rpcId.getMethodName() << L")\n";
}

// = sne::srpc::RpcReceiver overriding

void BotAccount::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    std::wcout << L"BotAccount::onReceiving(" << rpcId.get() << L"," <<
        rpcId.getMethodName() << L")\n";
}

// = rpc::AccountRpc overriding

FORWARD_SRPC_METHOD_1(BotAccount, selectCharacter,
    ObjectId, characterId);


FORWARD_SRPC_METHOD_0(BotAccount, enterWorld);


RECEIVE_SRPC_METHOD_2(BotAccount, onSelectCharacter,
    ErrorCode, errorCode, FullCharacterInfo, characterInfo)
{
    if (isFailed(errorCode)) {
        std::wcout << L"* " << accountInfo_.userId_ <<
            L": Failed to select character(E" << errorCode << L")\n";
        return;
    }

    std::wcout << L"* " << accountInfo_.userId_ <<
        L": Character(" << characterInfo.objectId_ << L") selected.\n";
}


RECEIVE_SRPC_METHOD_4(BotAccount, onEnterWorld,
    ErrorCode, errorCode, ObjectPosition, spawnPosition,
    MapCode, mapCode, WorldTime, worldTime)
{
    mapCode;

    if (isFailed(errorCode)) {
        std::wcout << L"* " << accountInfo_.userId_ <<
            L": Failed to enter world(E" << errorCode << L")\n";
        return;
    }

    isRequestEnterWorld_ = false;
    botPlayer_->worldEntered(spawnPosition);

    std::wcout << L"* " << accountInfo_.userId_ <<
        L": Character entered in the world(time:" <<
        int(worldTime) << ".\n";
}


RECEIVE_SRPC_METHOD_1(BotAccount, evExpelled,
    ExpelReason, reason)
{
    callback_.expelledFromServer(reason);
}

}} // namespace gideon { namespace bot {
