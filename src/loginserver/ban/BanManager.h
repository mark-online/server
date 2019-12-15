#pragma once

#include "../loginserver_export.h"
#include <gideon/server/data/BanInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace loginserver {

/**
 * @class BanManager
 *
 */
class LoginServer_Export BanManager
{
	typedef std::mutex LockType;

	
	SNE_DECLARE_SINGLETON(BanManager);
public:
	
	void initialize();

	bool isBanAccount(AccountId accountId) const;
	bool isBanCharacter(ObjectId characterId) const;
	bool isBanIp(const std::string& ip) const;

	void addBan(const BanInfo& banInfo);
	void removeBan(BanMode banMode, AccountId accountId, ObjectId characterId, const std::string& ipAddress);

private:
	void addBanAccount(const BanInfo& banInfo);
	void addBanCharacter(const BanInfo& banInfo);
	void addBanIp(const BanInfo& banInfo);

	void removeBanAccount(AccountId accountId);
	void removeBanCharacter(ObjectId characterId);
	void removeBanIp(const std::string& ipAddress);


public:
	BanInfos accountBanInfos_;
	BanInfos characterBanInfos_;
	BanInfos ipBanInfos_;
};

}} // namespace gideon { namespace loginserver {

#define BAN_MANAGER \
	gideon::loginserver::BanManager::instance()
