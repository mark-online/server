#pragma once

#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/LevelInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PartyCallback
 */
class PartyCallback
{
public:
    virtual ~PartyCallback() {}

public:
    virtual void partyMemberLevelup(ObjectId memberId, CreatureLevel level) = 0;
    virtual void partyMemberMoved(ObjectId objectId, const Position& position) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
