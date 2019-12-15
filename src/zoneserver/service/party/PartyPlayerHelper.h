#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go


namespace gideon { namespace zoneserver {

/***
 * @class PartyPlayerHelper
 ***/
class PartyPlayerHelper
{
public:
    virtual go::Entity* getPlayer(ObjectId playerId) = 0;
    virtual const go::Entity* getPlayer(ObjectId playerId) const = 0;
};


}} // namespace gideon { namespace zoneserver