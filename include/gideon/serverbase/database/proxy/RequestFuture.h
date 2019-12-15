#pragma once

#include <gideon/server/data/ShardInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/AuctionInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/database/proxy/DatabaseRequest.h>
#include <sne/base/concurrent/Future.h>
#include <sne/server/common/Property.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace serverbase {

/**
 * @class RequestSyncFuture
 */
class RequestSyncFuture : public sne::base::SyncFuture
{
public:
    RequestSyncFuture() :
        errorCode_(ecOk) {}

public:
    ErrorCode errorCode_;
};


/**
 * @class RequestAsyncFuture
 */
class RequestAsyncFuture : public sne::base::AsyncFuture
{
public:
    RequestAsyncFuture() :
        errorCode_(ecOk) {}

public:
    ErrorCode errorCode_;
};


/**
 * @class GetPropertiesRequestFuture
 */
class GetPropertiesRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetPropertiesRequestFuture>
{
public:
    sne::server::Properties properties_;
};


/**
 * @class GetServerSpecRequestFuture
 */
class GetServerSpecRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetServerSpecRequestFuture>
{
public:
    sne::server::ServerSpec serverSpec_;
};


/**
 * @class GetShardInfoRequestFuture
 */
class GetShardInfoRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetShardInfoRequestFuture>
{
public:
    FullShardInfo shardInfo_;
};


/**
 * @class GetZoneInfoRequestFuture
 */
class GetZoneInfoRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetZoneInfoRequestFuture>
{
public:
    ZoneInfo zoneInfo_;
};


/**
 * @class GetMaxItemInventoryIdFutureo
 */
class GetMaxItemInventoryIdRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetMaxItemInventoryIdRequestFuture>
{
public:
    ObjectId maxInventoryId_;
};


/**
 * @class GetZoneInfoListRequestFuture
 */
class GetZoneInfoListRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetZoneInfoListRequestFuture>
{
public:
    ZoneInfos zoneInfos_;
};


/**
 * @class GetWorldTimeRequestFuture
 */
class GetWorldTimeRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetWorldTimeRequestFuture>
{
public:
    WorldTime worldTime_;
};


/**
 * @class AuthenticateRequestFuture
 */
class AuthenticateRequestFuture : public RequestAsyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<AuthenticateRequestFuture>
{
public:
    AuthenticateRequestFuture() :
        accountId_(invalidAccountId) {}

    AccountId accountId_;
};


/**
 * @class GetFullUserInfoRequestFuture
 */
class GetFullUserInfoRequestFuture : public RequestAsyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetFullUserInfoRequestFuture>
{
public:
	GetFullUserInfoRequestFuture(const AccountInfo& accountInfo) :
		accountInfo_(accountInfo) 
	{}

	AccountInfo accountInfo_;
    FullUserInfo userInfo_;
};


/**
 * @class CreateCharacterRequestFuture
 */
class CreateCharacterRequestFuture : public RequestAsyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<CreateCharacterRequestFuture>
{
public:
    FullCharacterInfo characterInfo_;
};


/**
 * @class DeleteCharacterRequestFuture
 */
class DeleteCharacterRequestFuture : public RequestAsyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<DeleteCharacterRequestFuture>
{
public:
    DeleteCharacterRequestFuture() :
        characterId_(invalidObjectId) {}

    ObjectId characterId_;
	GuildId guildId_;
};



/**
 * @class CheckDuplicateNicknameRequestFuture
 */
class CheckDuplicateNicknameRequestFuture : public RequestAsyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<CheckDuplicateNicknameRequestFuture>
{
public:
    CheckDuplicateNicknameRequestFuture() :
        accountId_(invalidObjectId) {}

    AccountId accountId_;
    Nickname nickname_;
};



/**
 * @class GetGuildInfosRequestFuture
 */
class GetGuildInfosRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetGuildInfosRequestFuture>
{
public:
    GuildInfos guildInfos_;
};


/**
 * @class GetAuctionInfosRequestFuture
 */
class GetAuctionInfosRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetAuctionInfosRequestFuture>
{
public:
    ServerAuctionInfoMap auctionMap_;
};


/**
 * @class GetMaxGuildIdRequestFuture
 */
class GetMaxGuildIdRequestFuture : public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<GetMaxGuildIdRequestFuture>
{
public:
    GuildId maxGuildId_;
};


/**
 * @class LoadCharacterPropertiesRequestFuture
 */
class LoadCharacterPropertiesRequestFuture: public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<LoadCharacterPropertiesRequestFuture>
{
public:
    LoadCharacterPropertiesRequestFuture() :
        characterId_(invalidObjectId) {}

    ObjectId characterId_;
    std::string config_;
    std::string prefs_;
};


/**
 * @class LoadBuildingRequestFuture
 */
class LoadBuildingRequestFuture: public RequestSyncFuture,
    public sne::core::ThreadSafeMemoryPoolMixin<LoadBuildingRequestFuture>
{
public:
    LoadBuildingRequestFuture() :
        maxBuildingId_(invalidObjectId) {}
    
    ObjectId maxBuildingId_;
    BuildingInfoMap buildingInfoMap_;
};

}} // namespace gideon { namespace serverbase {
