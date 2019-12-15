#pragma once

#include "../PublicPartyable.h"
#include "../../../../service/party/PartyPlayerHelper.h"
#include <gideon/cs/shared/data/PartyInfo.h>
#include <sne/base/utility/AceUtil.h>

namespace gideon { namespace zoneserver { namespace go {

class Player;

/***
 * class PublicPartyAbillty
 ***/
class PublicPartyAbillity : public PublicPartyable,
    public PartyPlayerHelper
{
    typedef sne::core::HashMap<ObjectId, go::Entity*> MemberMap;
    typedef sne::base::Thread_Mutex_With_SpinLock PartyLockType;
public:
    PublicPartyAbillity();
    virtual ~PublicPartyAbillity();

protected:    
    void setOwnerInfo(const GameObjectInfo& info);

private:
    virtual ErrorCode addMember(go::Entity& player);
    virtual void removeMember(ObjectId playerId);
    virtual const GameObjectInfo& getOwnerInfo() const {
        return gameObjectInfo_;
    }

    void destory();

private:
    PartyMemberInfos::iterator getPartyMemberIterator(ObjectId playerId);

private:
    // = PartyPlayerHelper overriding
    go::Entity* getPlayer(ObjectId playerId);
    const go::Entity* getPlayer(ObjectId playerId) const;

private:
    mutable PartyLockType lockPublicPartyable_;
    MemberMap memberMap_;
    PartyMemberInfos memberInfos_;
    GameObjectInfo gameObjectInfo_;
    PartyPtr party_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {