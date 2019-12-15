#include "ZoneServerPCH.h"
#include "PlayerEffectController.h"
#include "../PlayerController.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/GraveStone.h"
#include "../../model/gameobject/Player.h"
#include "../../model/gameobject/skilleffect/PassiveSkillManager.h"
#include "../../model/gameobject/ability/AggroSendable.h"
#include "../../model/gameobject/ability/FieldDuelable.h"
#include "../../model/gameobject/ability/PassiveSkillCastable.h"
#include "../../model/state/FieldDuelState.h"
#include "../../service/duel/FieldDuel.h"
#include "../../service/item/GraveStoneService.h"
#include "../../helper/SkillTableHelper.h"
#include "../../world/WorldMap.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/server/utility/Profiler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

namespace {
		
/**
 * @class PlayerGraveStoneStoodEvent
 */
class PlayerGraveStoneStoodEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerGraveStoneStoodEvent>
{
public:
    PlayerGraveStoneStoodEvent(go::GraveStone& graveStone) :
        graveStone_(graveStone) {}

private:
    virtual void call(go::Entity& entity) {
        CreatureEffectCallback* effectCallback =
            entity.getController().queryCreatureEffectCallback();
        if (effectCallback != nullptr) {
            effectCallback->playerGraveStoneStood(graveStone_);
        }
    }

private:
    go::GraveStone& graveStone_;
};


} // namespace {

// = PlayerEffectController
IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerEffectController);

void PlayerEffectController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerEffectController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerEffectController::effectApplied(go::Entity& to, go::Entity& from,
    const SkillEffectResult& skillEffectResult)
{
    if (shouldStopFieldDuel(from)) {
        stopFieldDuel();
    }

    evEffectApplied(to.getGameObjectInfo(),
        from.getGameObjectInfo(), skillEffectResult);
}


void PlayerEffectController::effectHit(go::Entity& to, go::Entity& from, DataCode dataCode)
{
    evEffectHit(to.getGameObjectInfo(),
        from.getGameObjectInfo(), dataCode);
}



bool PlayerEffectController::shouldStopFieldDuel(go::Entity& target)
{    
    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.isFieldDueling() && target.isSame(target)) {
        return false;
    }
    if (! target.isPlayer()) {
        return true;
    }    

    if (owner.isMyDueler(target)) {
        return false;
    }

    return true;
}


void PlayerEffectController::stopFieldDuel() 
{
    FieldDualPtr ptr = getOwner().queryFieldDuelable()->getFieldDualPtr();
    if (ptr.get()) {
        ptr->stop(fdrtCancelEffect, invalidObjectId);
    }
}


void PlayerEffectController::standGraveStone()
{
    go::Entity& owner = getOwnerAs<go::Entity>();
    
    WorldMap* worldMap = owner.getCurrentWorldMap();
    assert(worldMap != nullptr);

    go::GraveStone* graveStone =
        GRAVE_STONE_SERVICE->registerGraveStone(owner, *worldMap);
    if (graveStone != nullptr) {
        auto event = std::make_shared<PlayerGraveStoneStoodEvent>(*graveStone);
        owner.queryKnowable()->broadcast(event);
    }

    ErrorCode errorCode = owner.queryLiveable()->revive(true);
    if (isFailed(errorCode)) {
        SNE_LOG_ERROR(__FUNCTION__ "(%W,%u)",
            owner.getNickname().c_str(), errorCode);
    }

    owner.getControllerAs<gc::PlayerController>().onRevive(errorCode, owner.getPosition());
}


void PlayerEffectController::effectAdded(const GameObjectInfo& creatureInfo,
    const DebuffBuffEffectInfo& info)
{
    evEffectAdded(creatureInfo, info);
}


void PlayerEffectController::effectRemoved(const GameObjectInfo& creatureInfo,
    DataCode dataCode, bool isCaster)
{
    evEffectRemoved(creatureInfo, dataCode, isCaster);
}


void PlayerEffectController::addCasterEffect(const SkillEffectResult& skillEffectResult)
{
    go::PassiveSkillCastable* castable = getOwner().queryPassiveSkillCastable();
    if (castable) {
        castable->getPassiveSkillManager().applySkill(skillEffectResult);
    }
}


void PlayerEffectController::removeCasterEffect(SkillCode skillCode)
{
    go::PassiveSkillCastable* castable = getOwner().queryPassiveSkillCastable();
    if (castable) {
        castable->getPassiveSkillManager().cancelSkill(skillCode);
    }
}


void PlayerEffectController::playerGraveStoneStood(go::GraveStone& graveStone)
{
    go::Entity& owner = getOwner();
    owner.queryKnowable()->know(graveStone);

    evPlayerGraveStoneStood(graveStone.getBaseGraveStoneInfo());
}


void PlayerEffectController::mesmerizationEffected(const GameObjectInfo& creatureInfo,
    MesmerizationType mezt, bool isActivate)
{
    evMesmerizationEffected(creatureInfo, mezt, isActivate);
}


void PlayerEffectController::mutated(const GameObjectInfo& creatureInfo,
    NpcCode npcCode, bool isActivate)
{
    evCreatureMutated(creatureInfo, npcCode, isActivate);
}


void PlayerEffectController::transformed(const GameObjectInfo& creatureInfo,
    NpcCode npcCode, bool isActivate)
{
    evCreatureTransformed(creatureInfo, npcCode, isActivate);
}


void PlayerEffectController::releaseHidden(const UnionEntityInfo& creatureInfo)
{
    evCreatureReleaseHidden(creatureInfo);
}


void PlayerEffectController::moveSpeedChanged(const GameObjectInfo& creatureInfo, float32_t currentSpeed)
{
    evCreatureMoveSpeedChanged(creatureInfo, currentSpeed);
}


void PlayerEffectController::frenzied(const GameObjectInfo& creatureInfo, float32_t currentScale, bool isActivate)
{
    evCreatureFrenzied(creatureInfo, currentScale, isActivate);
}


void PlayerEffectController::knockbacked(const GameObjectInfo& creatureInfo, const Position& position)
{
    evCreatureKnockbacked(creatureInfo, position);
}


void PlayerEffectController::knockbackReleased(const GameObjectInfo& creatureInfo)
{
    evCreatureKnockbackReleased(creatureInfo);
}


void PlayerEffectController::dashing(const GameObjectInfo& creatureInfo, const Position& position)
{
    evCreatureDashing(creatureInfo, position);
}


void PlayerEffectController::reviveEffected(const GameObjectInfo& creatureInfo, const ObjectPosition& position, HitPoint currentHp)
{
    evReviveEffected(creatureInfo, currentHp, position);
}

// = rpc::EffectRpc overriding
FORWARD_SRPC_METHOD_3(PlayerEffectController, evEffectApplied,
    GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
    SkillEffectResult, skillEffectResult);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evItemEffected,
    GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
    ItemEffectResult, itemEffectResult);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evMesmerizationEffected,
    GameObjectInfo, entityInfo, MesmerizationType, mezt, bool, isActivate);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evCreatureTransformed,
    GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evCreatureMutated,
    GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);


FORWARD_SRPC_METHOD_1(PlayerEffectController, evCreatureReleaseHidden,
    UnionEntityInfo, entityInfo);


FORWARD_SRPC_METHOD_2(PlayerEffectController, evCreatureMoveSpeedChanged,
    GameObjectInfo, entityInfo, float32_t, currentSpeed);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evCreatureFrenzied,
    GameObjectInfo, entityInfo, float32_t, currentScale, bool, isActivate);


FORWARD_SRPC_METHOD_2(PlayerEffectController, evCreatureKnockbacked,
    GameObjectInfo, entityInfo, Position, position);


FORWARD_SRPC_METHOD_1(PlayerEffectController, evCreatureKnockbackReleased,
    GameObjectInfo, entityInfo);


FORWARD_SRPC_METHOD_2(PlayerEffectController, evCreatureDashing,
    GameObjectInfo, entityInfo, Position, position);


FORWARD_SRPC_METHOD_1(PlayerEffectController, evPlayerGraveStoneStood,
    GraveStoneInfo, graveStroneInfo);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evReviveEffected,
    GameObjectInfo, entityInfo, HitPoint, currentPoint, ObjectPosition, position);


FORWARD_SRPC_METHOD_2(PlayerEffectController, evEffectAdded,
    GameObjectInfo, entityInfo, DebuffBuffEffectInfo, info);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evEffectRemoved,
    GameObjectInfo, entityInfo, DataCode, dataCode, bool, isCaster);


FORWARD_SRPC_METHOD_3(PlayerEffectController, evEffectHit,
    GameObjectInfo, casterInfo, GameObjectInfo, targetInfo, DataCode, dataCode);

// = sne::srpc::RpcForwarder overriding

void PlayerEffectController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerEffectController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerEffectController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerEffectController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
