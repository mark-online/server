#include "ZoneServerPCH.h"
#include "RandomDungeonController.h"
#include "callback/partycallback.h"
#include "../model/gameobject/Entity.h"
#include "../service/party/Party.h"

namespace gideon { namespace zoneserver { namespace gc {

// = InterestAreaCallback overriding

void RandomDungeonController::entitiesAppeared(const go::EntityMap& /*entities*/)
{
}


void RandomDungeonController::entityAppeared(go::Entity& /*entity*/,
    const UnionEntityInfo& /*entityInfo*/)
{
    // TODO: 필드 상에 있는지 검사해야 한다    
}


void RandomDungeonController::entitiesDisappeared(const go::EntityMap& /*entities*/)
{
}


void RandomDungeonController::entityDisappeared(go::Entity& /*entity*/)
{
}


void RandomDungeonController::entityDisappeared(const GameObjectInfo& /*info*/)
{
}


}}} // namespace gideon { namespace zoneserver { namespace gc {