#include "ServerTestPCH.h"
#include "MockAccountDatabase.h"


MockAccountDatabase::MockAccountDatabase() :
    accountId_(invalidAccountId)
{
    accountMap_.insert(AccountInfoMap::value_type(L"test1",
        AccountInfo(++accountId_, L"test1")));
    accountMap_.insert(AccountInfoMap::value_type(L"test2",
        AccountInfo(++accountId_, L"test2")));
    accountMap_.insert(AccountInfoMap::value_type(L"test3",
        AccountInfo(++accountId_, L"test3")));
    accountMap_.insert(AccountInfoMap::value_type(L"test4",
        AccountInfo(++accountId_, L"test4")));
    accountMap_.insert(AccountInfoMap::value_type(L"test5",
        AccountInfo(++accountId_, L"test5")));
}

bool MockAccountDatabase::getProperties(sne::server::Properties& properties)
{
    addCallCount("getProperties");

    properties.insert(
        SNE_PROPERTIES::value_type("c2s.high_concurrent_session_count", "100"));
    properties.insert(
        SNE_PROPERTIES::value_type("c2s.max_concurrent_session_count", "100"));
    properties.insert(
        SNE_PROPERTIES::value_type("certificate.effective_period", "100"));
    properties.insert(
        SNE_PROPERTIES::value_type("zone_server.dont_check_safe_region", "1"));
    return true;
}



bool MockAccountDatabase::getServerSpec(sne::server::ServerSpec& spec,
    const std::string& serverName, const std::string& suffix)
{
    addCallCount("getServerSpec");

    spec.authenticationTimeout_ = 1000;
    spec.maxUserCount_ = 100;
    spec.sessionPoolSize_ = 100;
    serverName, suffix;
    return true;
}


bool MockAccountDatabase::getShardInfos(FullShardInfoMap& shardMap)
{
    addCallCount("getShardInfos");

    FullShardInfo shardInfo;
    shardInfo.shardId_ = 1;
    shardInfo.name_ = L"shard1";
    shardInfo.zoneServerInfoMap_.emplace(ZoneId(1), ZoneServerInfo(1, "localhost", 10000, true));
    shardInfo.zoneServerInfoMap_.emplace(ZoneId(2), ZoneServerInfo(2, "localhost", 10001, false));
    shardMap.emplace(1, shardInfo);
    return true;
}


bool MockAccountDatabase::authenticate(AccountId& accountId,
    const UserId& userId, const UserPassword& /*password*/)
{
    addCallCount("authenticate");

    AccountInfoMap::const_iterator pos = accountMap_.find(userId);
    if (pos != accountMap_.end()) {
        const AccountInfo& info = (*pos).second;
        accountId = info.accountId_;
        return true;
    }
    return false;
}


bool MockAccountDatabase::getAccountInfo(AccountInfo& accountInfo, AccountId accountId)
{
    assert(! accountInfo.isValid());

    addCallCount("getAccountInfo");

    AccountInfoMap::const_iterator pos = accountMap_.begin();
    const AccountInfoMap::const_iterator end = accountMap_.end();
    for (; pos != end; ++pos) {
        const AccountInfo& info = (*pos).second;
        if (info.accountId_ == accountId) {
            accountInfo = info;
            return true;
        }
    }
    return true;
}


bool MockAccountDatabase::getCharacterCounts(CharacterCountPerShardMap& characterCounts,
    AccountId accountId)
{
    addCallCount("getCharacterCounts");
    characterCounts, accountId;
    return true;
}


bool MockAccountDatabase::updateLoginAt(AccountId /*accountId*/)
{
    addCallCount("updateLoginAt");
    return true;
}


bool MockAccountDatabase::updateCharacterCount(AccountId accountId, ShardId shardId,
    uint8_t characterCount)
{
    addCallCount("updateCharacterCount");

    accountId, shardId, characterCount;
    return true;
}


bool MockAccountDatabase::logLogout(AccountId accountId, ShardId shardId,
    uint32_t sessionSeconds)
{
    addCallCount("logLogout");
    accountId, shardId, sessionSeconds;
    return true;
}


//  = Ban
bool MockAccountDatabase::getBans(BanInfos& accountBanInfos, BanInfos& characterBanInfos, BanInfos& ipBanInfos)
{
    addCallCount("getBans");
    accountBanInfos, characterBanInfos, ipBanInfos;
    return true;	
}


bool MockAccountDatabase::addBan(const BanInfo& banInfo)
{
    addCallCount("addBan");
    banInfo;
    return true;
}


bool MockAccountDatabase::removeBan(BanMode banMode, AccountId accountId, ObjectId characterId, const std::string& ipAddress)
{
    addCallCount("removeBan");
    banMode, accountId, characterId, ipAddress;
    return true;
}
