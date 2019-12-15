#pragma once

#include <gideon/server/data/ShardInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/server/data/BanInfo.h>
#include <sne/server/common/Property.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/database/Database.h>

namespace gideon { namespace serverbase {

/**
 * @struct AccountDatabase
 *
 * Game Database.
 */
class AccountDatabase : public sne::database::Database
{
public:
    // = Helpers

    virtual void beginTransaction() = 0;
    virtual void commitTransaction() = 0;
    virtual void rollbackTransaction() = 0;

public:
    // = 서버 설정 관련

    /// 서버 정보를 읽어온다
	virtual bool getProperties(sne::server::Properties& properties) = 0;
    virtual bool getServerSpec(sne::server::ServerSpec& spec,
        const std::string& serverName, const std::string& suffix) = 0;

	/// 서버 그룹을 가지고 온다.
	virtual bool getShardInfos(FullShardInfoMap& shardMap) = 0;

public:
    // = 계정 관련

    /// 사용자 인증을 한다
    virtual bool authenticate(AccountId& accountId,
        const UserId& userId, const UserPassword& password) = 0;

	/// 계정정보를 가지고 온다.
	virtual bool getAccountInfo(AccountInfo& accountInfo, AccountId accountId) = 0;

    /// 샤드 별 캐릭터 수를 얻는다
    virtual bool getCharacterCounts(CharacterCountPerShardMap& characterCounts,
        AccountId accountId) = 0;

    /// 사용자의 로그인 시간을 갱신한다
    virtual bool updateLoginAt(AccountId accountId) = 0;

    /// 샤드 별 캐릭터 갯수를 갱신한다
    virtual bool updateCharacterCount(AccountId accountId, ShardId shardId,
        uint8_t characterCount) = 0;

public:
    // = 로그 관련

    /// 유저별로 로그인/아웃 시간을 기록하고, 마지막 접속 ShardId를 갱신한다
    virtual bool logLogout(AccountId accountId, ShardId shardId, uint32_t sessionSeconds) = 0;

public:
	//  = Ban
	virtual bool getBans(BanInfos& accountBanInfos, BanInfos& characterBanInfos, BanInfos& ipBanInfos) = 0;
	virtual bool addBan(const BanInfo& banInfo) = 0;
	virtual bool removeBan(BanMode banMode, AccountId accountId, ObjectId characterId, const std::string& ipAddress) = 0;
};

}} // namespace gideon { namespace serverbase {
