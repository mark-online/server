#pragma once

#include "../ZoneUser.h"
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace zoneserver { namespace go {
class EntityAllocator;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

/**
 * @class ZoneUserImpl
 *
 * Zone 사용자 실제 구현
 */
class ZoneServer_Export ZoneUserImpl :
    public ZoneUser,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(ZoneUserImpl);

    using LockType = std::mutex;

public:
    ZoneUserImpl(go::EntityAllocator& playerAllocator);
    virtual ~ZoneUserImpl();

public: // 테스트 편의를 위해
    virtual void initialize(const AccountInfo& accountInfo, const FullUserInfo& userInfo,
        AuthId authId, RegionCode spawnRegionCode, ObjectId arenaId, MapCode preGlobalWorldMapCode,
        const Position& position, ZoneClientSessionCallback& sessionCallback);

    virtual void finalize();

    virtual void expelledFromServer(ExpelReason expelReason);
    virtual void reserveMigration();

    /// @return left?
    virtual bool leaveFromWorld();

public:
    virtual AccountId getAccountId() const {
        return userInfo_.accountId_;
    }

	virtual ObjectId getActivateCharacterId() const;

    virtual sne::server::ClientId getClientId() const {
        return clientId_;
    }

    virtual AuthId getAuthId() const {
        return authId_;
    }

    virtual go::Player& getActivePlayer() {
        return *activePlayer_;
    }

    virtual FullUserInfo getUserInfo() const;

    virtual bool isCharacterSelected() const;

    virtual bool hasNickname(const Nickname& nickname) const;

    virtual bool isLoggingOut() const;

private:
    virtual void characterAdded(const FullCharacterInfo& characterInfo);
    virtual void characterDeleted(ObjectId characterId);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

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
    void reset();

    ErrorCode prepareCharacter(ObjectId characterId);
    void unprepareCharacter();

    ErrorCode enterWorld(ObjectPosition& spawnPosition);

private:
    bool isValid() const {
        return sne::server::isValid(clientId_);
    }

    bool canRequest() const;

private:
    ZoneClientSessionCallback* sessionCallback_;

    sne::server::ClientId clientId_;
	AccountInfo accountInfo_;
    FullUserInfo userInfo_;

    AuthId authId_;
    RegionCode spawnRegionCode_;
	ObjectId arenaId_;
    MapCode preGlobalWorldMapCode_;
    Position position_;

    std::unique_ptr<go::Player> activePlayer_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {
