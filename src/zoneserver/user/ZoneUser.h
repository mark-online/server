#pragma once

#include "../zoneserver_export.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/rpc/AccountRpc.h>
#include <sne/server/session/ClientId.h>
#include <sne/server/s2s/ServerId.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

class ZoneClientSessionCallback;

namespace go {
class Player;
} // namespace go


/**
 * @class ZoneUser
 *
 * Zone 내의 사용자 클래스.
 * - 사용자 요청/응답 진입점.
 */
class ZoneServer_Export ZoneUser :
    public rpc::AccountRpc
{
public:
    virtual ~ZoneUser() {}

    virtual void initialize(const AccountInfo& accountInfo, const FullUserInfo& userInfo,
        AuthId authId, RegionCode spawnRegionCode, ObjectId arenaId, MapCode preGlobalMapCode,
        const Position& position, ZoneClientSessionCallback& sessionCallback) = 0;

    virtual void finalize() = 0;

public:
    virtual void characterAdded(const FullCharacterInfo& characterInfo) = 0;
    virtual void characterDeleted(ObjectId characterId) = 0;

public:
    /// 강제 퇴장 당했다
    virtual void expelledFromServer(ExpelReason expelReason) = 0;

    /// 존에서 퇴장한다
    /// @return 존에 입장 중이었는가?
    virtual bool leaveFromWorld() = 0;

    virtual void reserveMigration() = 0;

public:
    virtual AccountId getAccountId() const = 0;

	virtual ObjectId getActivateCharacterId() const = 0;

    virtual sne::server::ClientId getClientId() const = 0;

    virtual AuthId getAuthId() const = 0;

    virtual go::Player& getActivePlayer() = 0;

    virtual bool hasNickname(const Nickname& nickname) const = 0;

    virtual bool isLoggingOut() const = 0;

public: // for Test
    bool hasCharacter() const {
        return getUserInfo().hasCharacter();
    }

    virtual FullUserInfo getUserInfo() const = 0;

    virtual bool isCharacterSelected() const = 0;

public:
    bool isValid() const {
        return isValidAccountId(getAccountId());
    }
};

}} // namespace gideon { namespace zoneserver {
