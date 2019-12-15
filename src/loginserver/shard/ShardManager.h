#pragma once

#include <gideon/server/data/ShardInfo.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon {
struct FullUserInfo;
} // namespace gideon {

namespace gideon { namespace loginserver {

class LoginClientSessionCallback;

/**
 * @class ShardManager
 * Shard 목록 관리
 */
class ShardManager : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(ShardManager);

    typedef std::mutex LockType;

    typedef sne::core::HashMap<ZoneId, uint16_t> CountMap;

    /**
     * @struct SelectShardRequest
     */
    struct SelectShardRequest {
        ShardId shardId_;
        sne::server::ServerId serverId_;
        LoginClientSessionCallback& sessionCallback_;

        explicit SelectShardRequest(ShardId shardId, sne::server::ServerId serverId,
            LoginClientSessionCallback& sessionCallback) :
            shardId_(shardId),
            serverId_(serverId),
            sessionCallback_(sessionCallback) {}
    };

    /**
     * @struct CreateCharacterRequest
     */
    struct CreateCharacterRequest {
        ShardId shardId_;
        sne::server::ServerId serverId_;
        LoginClientSessionCallback& sessionCallback_;

        explicit CreateCharacterRequest(ShardId shardId, sne::server::ServerId serverId,
            LoginClientSessionCallback& sessionCallback) :
            shardId_(shardId),
            serverId_(serverId),
            sessionCallback_(sessionCallback) {}
    };

    /**
     * @struct DeleteCharacterRequest
     */
    struct DeleteCharacterRequest {
        ShardId shardId_;
        sne::server::ServerId serverId_;
        LoginClientSessionCallback& sessionCallback_;

        explicit DeleteCharacterRequest(ShardId shardId, sne::server::ServerId serverId,
            LoginClientSessionCallback& sessionCallback) :
            shardId_(shardId),
            serverId_(serverId),
            sessionCallback_(sessionCallback) {}
    };

    /**
     * @struct ReserveNicknameRequest
     */
    struct ReserveNicknameRequest {
        ShardId shardId_;
        sne::server::ServerId serverId_;
        LoginClientSessionCallback& sessionCallback_;

        explicit ReserveNicknameRequest(ShardId shardId, sne::server::ServerId serverId,
            LoginClientSessionCallback& sessionCallback) :
            shardId_(shardId),
            serverId_(serverId),
            sessionCallback_(sessionCallback) {}
    };

    typedef sne::core::HashMap<AccountId, SelectShardRequest> EnterShardRequestMap;
    typedef sne::core::HashMap<AccountId, CreateCharacterRequest> CreateCharacterRequestMap;
    typedef sne::core::HashMap<AccountId, DeleteCharacterRequest> DeleteCharacterRequestMap;
    typedef sne::core::HashMap<AccountId, ReserveNicknameRequest> ReserveNicknameRequestMap;

public:
    ShardManager() {}

    bool initialize();

public:
    ErrorCode selectShard(ShardId shardId, AccountId accountId,
        LoginClientSessionCallback& callback);

    ErrorCode reserveNickname(ShardId shardId, AccountId accountId,
        const Nickname& nickname, LoginClientSessionCallback& callback);

    ErrorCode createCharacter(ShardId shardId, 
        const CreateCharacterInfo& createCharacterInfo, LoginClientSessionCallback& callback);

    ErrorCode deleteCharacter(ShardId shardId, AccountId accountId, ObjectId characterId,
        LoginClientSessionCallback& callback);

    void cancelRequests(AccountId accountId, LoginClientSessionCallback* callback);

public:
    void zoneServerConnected(sne::server::ServerId serverId, ShardId shardId, ZoneId zoneId,
        uint16_t currentUserCount, uint16_t maxUserCount);
    void zoneServerReadied(ShardId shardId, ZoneId zoneId);

    void zoneServerDisconnected(ShardId shardId, ZoneId zoneId, sne::server::ServerId serverId);

public:
    void userLoggedIn(ShardId shardId, ZoneId zoneId);
    void userLoggedOut(ShardId shardId, ZoneId zoneId);

public:
    void shardSelected(ErrorCode errorCode, const FullUserInfo& userInfo);

    void characterCreated(ErrorCode errorCode, AccountId accountId,
        const FullCharacterInfo& characterInfo);

    void characterDeleted(ErrorCode errorCode, AccountId accountId, ObjectId characterId);

    void nicknameReserved(ErrorCode errorCode, AccountId accountId, const Nickname& nickname);

public:
    ShardInfoList getShardInfoList() const;
    ZoneMapCodeMap getZoneMapCodeMap() const;

    ZoneServerInfo getZoneServerInfo(ShardId shardId, ZoneId zoneId) const;

private:
    FullShardInfo* getShardInfo(ShardId shardId);
    const FullShardInfo* getShardInfo(ShardId shardId) const;

    ZoneServerInfo* getZoneServerInfo_i(ShardId shardId, ZoneId zoneId);
    const ZoneServerInfo* getZoneServerInfo_i(ShardId shardId, ZoneId zoneId) const;

    sne::server::ServerId getMostFreeZoneServerId(const FullShardInfo& shardInfo) const;

private:
    void addSelectShardRequest(ShardId shardId, sne::server::ServerId serverId,
        AccountId accountId, LoginClientSessionCallback& callback) {
        selectShardRequestMap_.insert(
            EnterShardRequestMap::value_type(accountId,
                SelectShardRequest(shardId, serverId, callback)));
    }

    void deleteSelectShardRequest(AccountId accountId) {
        selectShardRequestMap_.erase(accountId);
    }

    SelectShardRequest* getSelectShardRequest(AccountId accountId) {
        const EnterShardRequestMap::iterator pos = selectShardRequestMap_.find(accountId);
        if (pos != selectShardRequestMap_.end()) {
            return &(*pos).second;
        }
        return nullptr;
    }

private:
    void addCreateCharacterRequest(ShardId shardId, sne::server::ServerId serverId,
        AccountId accountId, LoginClientSessionCallback& callback) {
        createCharacterRequestMap_.insert(
            CreateCharacterRequestMap::value_type(accountId,
                CreateCharacterRequest(shardId, serverId, callback)));
    }

    void deleteCreateCharacterRequest(AccountId accountId) {
        createCharacterRequestMap_.erase(accountId);
    }

    CreateCharacterRequest* getCreateCharacterRequest(AccountId accountId) {
        const CreateCharacterRequestMap::iterator pos =
            createCharacterRequestMap_.find(accountId);
        if (pos != createCharacterRequestMap_.end()) {
            return &(*pos).second;
        }
        return nullptr;
    }

private:
    void addDeleteCharacterRequest(ShardId shardId, sne::server::ServerId serverId,
        AccountId accountId, LoginClientSessionCallback& callback) {
        deleteCharacterRequestMap_.insert(
            DeleteCharacterRequestMap::value_type(accountId,
               DeleteCharacterRequest(shardId, serverId, callback)));
    }

    void deleteDeleteCharacterRequest(AccountId accountId) {
        deleteCharacterRequestMap_.erase(accountId);
    }

    DeleteCharacterRequest* getDeleteCharacterRequest(AccountId accountId) {
        const DeleteCharacterRequestMap::iterator pos =
            deleteCharacterRequestMap_.find(accountId);
        if (pos != deleteCharacterRequestMap_.end()) {
            return &(*pos).second;
        }
        return nullptr;
    }

private:
    void addReserveNicknameRequest(ShardId shardId, sne::server::ServerId serverId,
        AccountId accountId, LoginClientSessionCallback& callback) {
            reserveNicknameRequestMap_.insert(
                ReserveNicknameRequestMap::value_type(accountId,
                ReserveNicknameRequest(shardId, serverId, callback)));
    }

    void deleteReserveNicknameRequest(AccountId accountId) {
        reserveNicknameRequestMap_.erase(accountId);
    }

    ReserveNicknameRequest* getReserveNicknameRequest(AccountId accountId) {
        const ReserveNicknameRequestMap::iterator pos =
            reserveNicknameRequestMap_.find(accountId);
        if (pos != reserveNicknameRequestMap_.end()) {
            return &(*pos).second;
        }
        return nullptr;
    }

private:
    void removeAllRequest(sne::server::ServerId serverId);

private:
    FullShardInfoMap shardInfoMap_;
    ShardInfoList shardInfoList_;
    ZoneMapCodeMap zoneMapCodeMap_;

    CountMap currentUserCountMap_;
    CountMap maxUserCountMap_;

    EnterShardRequestMap selectShardRequestMap_;
    CreateCharacterRequestMap createCharacterRequestMap_;
    DeleteCharacterRequestMap deleteCharacterRequestMap_;
    ReserveNicknameRequestMap reserveNicknameRequestMap_;
    mutable LockType lock_;
};

}} // namespace gideon { namespace loginserver {

#define SHARD_MANAGER \
    gideon::loginserver::ShardManager::instance()
