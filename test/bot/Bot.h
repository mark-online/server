#pragma once

#include "Config.h"
#include "BotCallback.h"
#include <gideon/clientnet/ServerProxyCallback.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <ctime>

namespace gideon { namespace clientnet {
class ServerProxy;
}} // namespace gideon { namespace clientnet {

namespace gideon { namespace bot {

class Config;
class BotAccount;

/**
 * @class Bot
 */
class Bot :
    public BotAccountCallback,
    private clientnet::ServerProxyCallback
{
public:
    Bot(const Config& config,
        const UserId& userId, const UserPassword& password);

    ~Bot();

    void tick();

    void disconnect();

private:
    void initBot();

    void loggedOut();
    void move();
    void enterWorld();
    void loginCommunityServer();
    void loginZoneServer();
    void login();

private:
	bool shouldLoginLoginServer() const {
		return ! loginResult_.isValid();
	}

    bool shouldLoginCommunityServer() const;
    bool shouldLoginZoneServer() const;

    bool shouldLogout() const {
        return ((time(nullptr) - lastLogoutTime_) >
            static_cast<time_t>(config_.getLogoutInterval())) &&
            ((rand() % 100) < 1);
    }

    bool shouldEnterWorld() const;

    bool shouldMove() const;

    void setNextDestination();

    ObjectPosition getNextPosition(const ObjectPosition& position);

private:
    // = BotAccountCallback overriding
    void expelledFromServer(ExpelReason reason) override;

private:
    // = clientnet::ServerProxyCallback overriding
    void onExpelled(ExpelReason reason) override;
    void onCommunityServerDisconnected() override;
    void onZoneServerDisconnected() override;

private:
    const Config& config_;
    const UserId userId_;
    const UserPassword userPassword_;
    
    ObjectPosition destination_;

	clientnet::LoginResult loginResult_;
    ShardInfoList shardInfos_;

    uint32_t lastTickInterval_;
    uint32_t lastTickCount_;
    time_t lastLogoutTime_;

    std::string zoneIp_;
    uint16_t zonePort_;

    int8_t xDirection_;
    int8_t yDirection_;
    int8_t zDirection_;

    std::unique_ptr<clientnet::ServerProxy> serverProxy_;

    std::unique_ptr<BotAccount> botAccount_;
};

}} // namespace gideon { namespace bot {
