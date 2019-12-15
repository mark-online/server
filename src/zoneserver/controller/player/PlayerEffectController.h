#pragma once

#include "../CreatureEffectController.h"
#include "../callback/PassiveEffectCallback.h"
#include <gideon/cs/shared/rpc/player/EffectRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerEffectController
 * 효과 담당
 */
class ZoneServer_Export PlayerEffectController : public CreatureEffectController,
    public rpc::EffectRpc,
    public PassiveEffectCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerEffectController);
public:
    PlayerEffectController(go::Entity* owner) :
        CreatureEffectController(owner) {}

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    bool shouldStopFieldDuel(go::Entity& target);
    void stopFieldDuel();

private:
    // = EffectCallback overriding
    virtual void effectApplied(go::Entity& to, go::Entity& from,
        const SkillEffectResult& skillEffectResult);
    virtual void effectHit(go::Entity& to, go::Entity& from, DataCode dataCode);

    // = CreatureEffectCallback overriding
    virtual void standGraveStone();
    virtual void mesmerizationEffected(const GameObjectInfo& creatureInfo,
        MesmerizationType mezt, bool isActivate);
    virtual void playerGraveStoneStood(go::GraveStone& graveStone);

    virtual void mutated(const GameObjectInfo& creatureInfo, NpcCode npcCode,
        bool isActivate);
    virtual void transformed(const GameObjectInfo& creatureInfo, NpcCode npcCode,
        bool isActivate);
    virtual void releaseHidden(const UnionEntityInfo& creatureInfo);
    virtual void moveSpeedChanged(const GameObjectInfo& creatureInfo, float32_t currentSpeed);
    virtual void frenzied(const GameObjectInfo& creatureInfo, float32_t currentScale, bool isActivate);
    virtual void knockbacked(const GameObjectInfo& creatureInfo, const Position& position);
    virtual void knockbackReleased(const GameObjectInfo& creatureInfo);

    virtual void dashing(const GameObjectInfo& creatureInfo, const Position& position);
    virtual void reviveEffected(const GameObjectInfo& creatureInfo, const ObjectPosition& position, HitPoint currentHp);

    virtual void effectAdded(const GameObjectInfo& creatureInfo,
        const DebuffBuffEffectInfo& info);
    virtual void effectRemoved(const GameObjectInfo& creatureInfo,
        DataCode dataCode, bool isCaster);

private:
    // = PassiveEffectCallback overriding
    virtual void addCasterEffect(const SkillEffectResult& skillEffectResult);
    virtual void removeCasterEffect(SkillCode skillCode);

public:
    OVERRIDE_SRPC_METHOD_3(evEffectApplied,
        GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
        SkillEffectResult, skillEffectResult);
    OVERRIDE_SRPC_METHOD_3(evItemEffected,
        GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
        ItemEffectResult, itemEffectResult);
    OVERRIDE_SRPC_METHOD_3(evMesmerizationEffected,
        GameObjectInfo, entityInfo, MesmerizationType, mezt, bool, isActivate);
    OVERRIDE_SRPC_METHOD_3(evCreatureTransformed,
        GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);
    OVERRIDE_SRPC_METHOD_3(evCreatureMutated,
        GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);
    OVERRIDE_SRPC_METHOD_1(evCreatureReleaseHidden,
        UnionEntityInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_2(evCreatureMoveSpeedChanged,
        GameObjectInfo, entityInfo, float32_t, currentSpeed);
    OVERRIDE_SRPC_METHOD_3(evCreatureFrenzied,
        GameObjectInfo, entityInfo, float32_t, currnetScale, bool, isActivate);
    OVERRIDE_SRPC_METHOD_2(evCreatureKnockbacked,
        GameObjectInfo, entityInfo, Position, position);
    OVERRIDE_SRPC_METHOD_1(evCreatureKnockbackReleased,
        GameObjectInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_2(evCreatureDashing,
        GameObjectInfo, entityInfo, Position, position);
    OVERRIDE_SRPC_METHOD_1(evPlayerGraveStoneStood,
        GraveStoneInfo, graveStroneInfo);
    OVERRIDE_SRPC_METHOD_3(evReviveEffected,
        GameObjectInfo, entityInfo, HitPoint, currentPoint, ObjectPosition, position);

public:
    // 스킬 효과 추가
    OVERRIDE_SRPC_METHOD_2(evEffectAdded,
        GameObjectInfo, entityInfo, DebuffBuffEffectInfo, info);

    // 스킬 효과 삭제
    OVERRIDE_SRPC_METHOD_3(evEffectRemoved,
        GameObjectInfo, entityInfo, DataCode, dataCode, bool, isCaster);

    OVERRIDE_SRPC_METHOD_3(evEffectHit,
        GameObjectInfo, casterInfo, GameObjectInfo, targetInfo, DataCode, dataCode);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {