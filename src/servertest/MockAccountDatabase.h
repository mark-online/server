#pragma once

#include <gideon/serverbase/database/AccountDatabase.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
* @class MockAccountDatabase
*/
class MockAccountDatabase : public serverbase::AccountDatabase,
    public sne::test::CallCounter
{
public:
    typedef core::HashMap<UserId, AccountInfo> AccountInfoMap;

    MockAccountDatabase();

    virtual bool open(server::ConfigReader& /*configReader*/) {return true;}
    virtual void close() {}
    virtual void ping() {}
    virtual void beginTransaction() {}
    virtual void commitTransaction() {}
    virtual void rollbackTransaction() {}

public:
    // = 서버 설정 관련

    virtual bool getProperties(sne::server::Properties& properties);
    virtual bool getServerSpec(sne::server::ServerSpec& spec,
        const std::string& serverName, const std::string& suffix);

    virtual bool getShardInfos(FullShardInfoMap& shardMap);


public:
    // = 계정 관련

    virtual bool authenticate(AccountId& accountId,
        const UserId& userId, const UserPassword& password);

    virtual bool getAccountInfo(AccountInfo& accountInfo, AccountId accountId);

    virtual bool getCharacterCounts(CharacterCountPerShardMap& characterCounts,
        AccountId accountId);

    virtual bool updateLoginAt(AccountId accountId);

    virtual bool updateCharacterCount(AccountId accountId, ShardId shardId,
        uint8_t characterCount);

public:
    // = 로그 관련

    virtual bool logLogout(AccountId accountId, ShardId shardId, uint32_t sessionSeconds);

public:
    //  = Ban
    virtual bool getBans(BanInfos& accountBanInfos, BanInfos& characterBanInfos, BanInfos& ipBanInfos);
    virtual bool addBan(const BanInfo& banInfo);
    virtual bool removeBan(BanMode banMode, AccountId accountId, ObjectId characterId, const std::string& ipAddress);

private:
    AccountId accountId_;
    AccountInfoMap accountMap_;
};
