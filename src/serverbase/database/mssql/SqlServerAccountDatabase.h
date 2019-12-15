#pragma once

#include <gideon/serverbase/database/AccountDatabase.h>
#include <sne/database/ado/AdoConnection.h>
#include <sne/database/DatabaseFactory.h>
#include "SqlServerAccountDatabaseCommands.h"

namespace gideon { namespace serverbase {

/**
 * @class SqlServerAccountDatabase
 *
 * MS SQL Server에 특화된 AccountDatabase 클래스.
 */
class SqlServerAccountDatabase : public AccountDatabase
{
public:
    SqlServerAccountDatabase();

private:
    virtual bool open(sne::server::ConfigReader& configReader);
    virtual void close();
    virtual void ping() {
        getAdoDatabase().ping();
    }

private:
	virtual void beginTransaction();
	virtual void commitTransaction();
	virtual void rollbackTransaction();

	// = 서버 설정 관련
	virtual bool getProperties(sne::server::Properties& properties);
	virtual bool getServerSpec(sne::server::ServerSpec& spec,
		const std::string& serverName, const std::string& suffix);
	virtual bool getShardInfos(FullShardInfoMap& shardMap);

	// = 계정 관련

	virtual bool authenticate(AccountId& accountId,
		const UserId& userId, const UserPassword& password);

	virtual bool getAccountInfo(AccountInfo& accountInfo, AccountId accountId);

    virtual bool getCharacterCounts(CharacterCountPerShardMap& characterCounts,
        AccountId accountId);

	virtual bool updateLoginAt(AccountId accountId);

    virtual bool updateCharacterCount(AccountId accountId, ShardId shardId,
        uint8_t characterCount);

	// = 로그 관련

	virtual bool logLogout(AccountId accountId, ShardId shardId,
		uint32_t sessionSeconds);

	virtual bool getBans(BanInfos& accountBanInfos, BanInfos& characterBanInfos, BanInfos& ipBanInfos);
	virtual bool addBan(const BanInfo& banInfo);
	virtual bool removeBan(BanMode banMode, AccountId accountId, ObjectId characterId, const std::string& ipAddress);

private:
    sne::database::AdoDatabase& getAdoDatabase() {
        return connection_.getDatabase();
    }

private:
    sne::database::AdoConnection connection_;

	GetPropertiesCommand getPropertiesCommand_;
    GetServerSpecCommand getServerSpecCommand_;
    GetShardsCommand getShardsCommand_;
    GetPasswordCommand getPasswordCommand_;
    GetAccountInfoCommand getAccountInfoCommand_;
    GetCharacterCountsCommand getCharacterCountsCommand_;
    UpdateLoginAtCommand updateLoginAtCommand_;
    UpdateCharacterCountCommand updateCharacterCountCommand_;
    LogLogoutCommand logLogoutCommand_;
	GetBansCommand getBansCommand_;
	AddBanCommand addBanCommand_;
	RemoveBanCommand removeBanCommand_;
};

}} // namespace gideon { namespace serverbase {
