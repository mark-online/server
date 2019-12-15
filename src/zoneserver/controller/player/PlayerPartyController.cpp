#include "ZoneServerPCH.h"
#include "PlayerPartyController.h"
#include "../EntityController.h"
#include "../callback/PublicPartyCallback.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/Player.h"
#include "../../model/gameobject/status/CreatureStatus.h"
#include "../../service/party/PartyService.h"
#include "../../service/party/Party.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>

namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerPartyController);

PlayerPartyController::PlayerPartyController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerPartyController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerPartyController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerPartyController::partyMemberMoved(ObjectId objectId, const Position& position)
{
    evPartyMemberMoved(objectId, position);
}


void PlayerPartyController::partyMemberLevelup(ObjectId memberId, CreatureLevel level)
{
    evPartyMemberLevelup(memberId, level);
}


RECEIVE_SRPC_METHOD_1(PlayerPartyController, queryPartyMemberSubInfo,
    ObjectId, playerId)
{
    go::Entity* entity = WORLD->getPlayer(playerId);
    if (! entity) {
		return;
	}
	go::Player* player = static_cast<go::Player*>(entity);
    HitPoints hitPoints = player->queryLiveable()->getCreatureStatus().getHitPoints();        
    ManaPoints manaPoints = player->queryLiveable()->getCreatureStatus().getManaPoints();        
    onPartyMemberSubInfo(playerId, player->queryCharacterClassable()->getCharacterClass(),
        static_cast<go::Creature*>(player)->getCreatureLevel(),
        hitPoints, manaPoints, player->getPosition());
}


FORWARD_SRPC_METHOD_6(PlayerPartyController, onPartyMemberSubInfo,
    ObjectId, objectId, CharacterClass, characterClass,
    CreatureLevel, level, HitPoints, hitPoints, 
    ManaPoints, manaPoints, Position, position);


FORWARD_SRPC_METHOD_2(PlayerPartyController, evPartyMemberLevelup,
    ObjectId, objectId, CreatureLevel, level);


FORWARD_SRPC_METHOD_2(PlayerPartyController, evPartyMemberMoved,
    ObjectId, objectId, Position, position);



}}} // namespace gideon { namespace zoneserver { namespace gc {