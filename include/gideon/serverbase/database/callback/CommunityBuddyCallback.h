#pragma once

#include <gideon/cs/shared/data/BuddyInfos.h>

namespace gideon { namespace serverbase {

/***
 * @class CommunityBuddyCallback
 ***/
class CommunityBuddyCallback
{
public:
	virtual void buddiesReceived(AccountId accountId,
        const BuddyInfos& buddyInfos, const BlockInfos& blockInfos) = 0;
};


}} // namespace gideon { namespace serverbase {