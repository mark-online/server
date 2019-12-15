#include "LoginServerPCH.h"
#include "BanManager.h"
#include <gideon/serverbase/database/AccountDatabase.h>
#include <sne/database/DatabaseManager.h>

namespace gideon { namespace loginserver {

SNE_DEFINE_SINGLETON(BanManager);


void BanManager::initialize()
{
	accountBanInfos_.clear();
	characterBanInfos_.clear();
	ipBanInfos_.clear();

	sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);
	db->getBans(accountBanInfos_, characterBanInfos_, ipBanInfos_);
}


bool BanManager::isBanAccount(AccountId accountId) const
{
	const sec_t now = getTime();

	for (const BanInfo& info : accountBanInfos_) {
		if (info.accountId_ == accountId) {
			if (info.banExpireTime_ == 0) {
				return true;
			}
			if (now < info.banExpireTime_ ) {
				return true;
			}
			return false;
		}
	}

	return false;
}


bool BanManager::isBanCharacter(ObjectId characterId) const
{
	const sec_t now = getTime();

	for (const BanInfo& info : characterBanInfos_) {
		if (info.characterId_ == characterId) {
			if (info.banExpireTime_ == 0) {
				return true;
			}
			if (now < info.banExpireTime_ ) {
				return true;
			}
			return false;
		}
	}
	return false;
}


bool BanManager::isBanIp(const std::string& ip) const
{
	const sec_t now = getTime();

	for (const BanInfo& info : ipBanInfos_) {
		if (info.ip_ == ip) {
			if (info.banExpireTime_ == 0) {
				return true;
			}
			if (now < info.banExpireTime_ ) {
				return true;
			}
			return false;
		}
	}

	return false;
}


void BanManager::addBan(const BanInfo& banInfo)
{
	if (banInfo.banMode_ == bmAccount) {
		addBanAccount(banInfo);
	}
	else if (banInfo.banMode_ == bmCharacter) {
		addBanCharacter(banInfo);
	}
	else if (banInfo.banMode_ == bmIpAddress) {
		addBanIp(banInfo);
	}
	else {
		assert(false && "invalid ban mode");
		return;
	}

	sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);
	db->addBan(banInfo);

}


void BanManager::removeBan(BanMode banMode, AccountId accountId, ObjectId characterId, const std::string& ipAddress)
{
	if (banMode == bmAccount) {
		removeBanAccount(accountId);
	}
	else if (banMode == bmCharacter) {
		removeBanCharacter(characterId);
	}
	else if (banMode == bmIpAddress) {
		removeBanIp(ipAddress);
	}
	else {
		assert(false && "invalid ban mode");
		return;
	}
	sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);
	db->removeBan(banMode, accountId, characterId, ipAddress);
}


void BanManager::addBanAccount(const BanInfo& banInfo)
{
	removeBanAccount(banInfo.accountId_);
	accountBanInfos_.push_back(banInfo);
}


void BanManager::addBanCharacter(const BanInfo& banInfo)
{
	removeBanCharacter(banInfo.characterId_);
	characterBanInfos_.push_back(banInfo);
}


void BanManager::addBanIp(const BanInfo& banInfo)
{
	removeBanIp(banInfo.ip_);
	ipBanInfos_.push_back(banInfo);
}


void BanManager::removeBanAccount(AccountId accountId)
{
	BanInfos::iterator pos = ipBanInfos_.begin();
	BanInfos::iterator end = ipBanInfos_.end();
	for (pos; pos != end; ++pos) {
		BanInfo& info = *pos;
		if (info.accountId_ == accountId) {
			ipBanInfos_.erase(pos);
			break;;
		}
	}
}


void BanManager::removeBanCharacter(ObjectId characterId)
{
	BanInfos::iterator pos = ipBanInfos_.begin();
	BanInfos::iterator end = ipBanInfos_.end();
	for (pos; pos != end; ++pos) {
		BanInfo& info = *pos;
		if (info.characterId_ == characterId) {
			ipBanInfos_.erase(pos);
			break;;
		}
	}
}


void BanManager::removeBanIp(const std::string& ipAddress)
{
	BanInfos::iterator pos = ipBanInfos_.begin();
	BanInfos::iterator end = ipBanInfos_.end();
	for (pos; pos != end; ++pos) {
		BanInfo& info = *pos;
		if (info.ip_ == ipAddress) {
			ipBanInfos_.erase(pos);
			break;;
		}
	}
}
}} // namespace gideon { namespace loginserver {
