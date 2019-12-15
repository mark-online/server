#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/rpc/AccountRpc.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace clientnet {
class ServerProxy;
}} // namespace gideon { namespace clientnet {

namespace gideon { namespace bot {

class BotAccountCallback;
class BotPlayer;
class BotCommunity;

/**
 * @class BotAccount
 */
class BotAccount :
    public rpc::AccountRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(BotAccount);

public:
    BotAccount(BotAccountCallback& callback, clientnet::ServerProxy& serverProxy);

	void initialize(const AccountInfo& accountInfo);

    void setFullUserInfo(const FullUserInfo& userInfo);
    void reset();

    void addCharacter(const FullCharacterInfo& fullCharacterInfo);

    void prepareCharacter(ObjectId characterId);

public:
    BotPlayer& getPlayer() {
        return *botPlayer_;
    }

    const UserId& getUserId() const {
		return accountInfo_.userId_;
    }

    ObjectId getLastAccessCharacterId() const {
        return userInfo_.lastAccessCharacterId_;
    }

    bool hasCharacter() const {
        return ! userInfo_.characters_.empty();
    }

    bool isCharacterSelected() const;

    bool isEnteredIntoWorld() const;

    bool isEnterWorld() const;

    bool isReadyToPlay() const;

    void setRequestEnterWorld(bool request) {
        isRequestEnterWorld_ = request;
    }

    bool isRequestEnterWorld() const {
        return isRequestEnterWorld_;
    }

    bool shouldMove() const;
    void moveTo(const Position& destination);
    void move();

    bool canCastManaSkill() const;
    void castManaSkill();

public:
    // = rpc::AccountRpc overriding
    OVERRIDE_SRPC_METHOD_1(selectCharacter,
        ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_0(enterWorld);

    OVERRIDE_SRPC_METHOD_2(onSelectCharacter,
        ErrorCode, errorCode, FullCharacterInfo, characterInfo);
    OVERRIDE_SRPC_METHOD_4(onEnterWorld,
        ErrorCode, errorCode, ObjectPosition, spawnPosition, MapCode, mapCode,
        WorldTime, worldTime);
    OVERRIDE_SRPC_METHOD_1(evExpelled,
        ExpelReason, reason);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    BotAccountCallback& callback_;

	AccountInfo accountInfo_;
    FullUserInfo userInfo_;

    bool isRequestEnterWorld_;

    std::unique_ptr<BotPlayer> botPlayer_;
    std::unique_ptr<BotCommunity> botCommunity_;
};

}} // namespace gideon { namespace bot {
