#pragma once

#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/UserId.h>

namespace gideon { namespace communityserver {

class CommunityUser;

/***
 * @class CommunityUserHelper
 ***/
class CommunityUserHelper
{
public:
    virtual AccountId getAccountId(const Nickname& nickname) const = 0;

    virtual CommunityUser* getUser(AccountId accountId) = 0;

	virtual CommunityUser* getUser(AccountId accountId, ObjectId playerId) = 0;

public:
    CommunityUser* getUser(const Nickname& nickname) {
        const AccountId accountId = getAccountId(nickname);
        if (! isValidAccountId(accountId)) {
            return nullptr;
        }
        return getUser(accountId);
    }
};

}} // namespace gideon { namespace communityserver {