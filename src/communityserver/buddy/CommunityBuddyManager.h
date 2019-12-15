#pragma once

#include <gideon/serverbase/database/callback/CommunityBuddyCallback.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace communityserver {

/***
 * @class CommunityBuddyManager
 ***/
class CommunityBuddyManager : public serverbase::CommunityBuddyCallback
{
public:
	bool initialize();

private:
	virtual void buddiesReceived(AccountId accountId,
        const BuddyInfos& buddyInfos, const BlockInfos& blockInfos);
};

}} // namespace gideon { namespace communityserver {