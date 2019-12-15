#include "CommunityServerPCH.h"
#include "CommunityBuddyManager.h"
#include "../user/CommunityUserManager.h"
#include "../user/CommunityUser.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>


namespace gideon { namespace communityserver {

namespace {

/**
 * @class DatabaseCallbackResisterTask
 */
class DatabaseCallbackResisterTask : public sne::database::DatabaseTask
{
public:
    DatabaseCallbackResisterTask(serverbase::CommunityBuddyCallback& callback) :
        callback_(callback) {}

    virtual void run(sne::database::Database& database) {
        serverbase::ProxyGameDatabase& db = 
            static_cast<serverbase::ProxyGameDatabase&>(database);
        db.registBuddyCallback(callback_);
    }

private:
    serverbase::CommunityBuddyCallback& callback_;
};

} // namespace {

bool CommunityBuddyManager::initialize()
{
	if (! COMMUNITYUSER_MANAGER) {
		return false;
	}

	DatabaseCallbackResisterTask callbackTask(*this);
	SNE_DATABASE_MANAGER->enumerate(callbackTask);

	return true;
}


void CommunityBuddyManager::buddiesReceived(AccountId accountId,
    const BuddyInfos& buddyInfos, const BlockInfos& blockInfos)
{
	CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
	if (user) {
		user->initBuddyInfo(buddyInfos, blockInfos);
	}
}

}} // namespace gideon { namespace communityserver {