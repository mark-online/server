#pragma once

#include <gideon/server/data/ServerSkillEffect.h>
#include <gideon/cs/shared/data/UnionEntityInfo.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/MesmerizationInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/NpcInfo.h>

namespace gideon { namespace zoneserver {
class Skill;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
class GraveStone;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CreatureEffectCallback
 */
class CreatureEffectCallback
{
public:
    virtual ~CreatureEffectCallback() {}

    /// 메즈류
    virtual void effectiveMesmerization(MesmerizationType mezt, bool isActivate) = 0;

    /// 변이
    virtual void mutate(NpcCode npcCode, sec_t duration, bool releaseMutate) = 0;


    virtual void panic(sec_t duration, bool isActivate) = 0;

    /// 변신
    virtual void transform(NpcCode npcCode, bool releaseTransform) = 0;

    /// 비석화 스킬
    virtual void standGraveStone() = 0;

    /// 속도 변경
    virtual void changeMoveSpeed(float32_t oldSpeed) = 0;

    /// 넉백
    virtual void knockback(const Position& position) = 0;

    /// 넉백 해제
    virtual void knockbackRelease() = 0;

    virtual void dash(const Position& position) = 0;
    virtual void reviveAtOnce(permil_t perRefillHp) = 0;

    virtual void addEffect(const DebuffBuffEffectInfo& info) = 0;
    virtual void removeEffect(DataCode dataCode, bool isCaster) = 0;

public:

    virtual void mesmerizationEffected(const GameObjectInfo& creatureInfo, MesmerizationType mezt, bool isActivate) = 0;

    virtual void mutated(const GameObjectInfo& creatureInfo, NpcCode npcCode,
        bool isActivate) = 0;

    /// 변신
    virtual void transformed(const GameObjectInfo& creatureInfo, NpcCode npcCode,
        bool isActivate) = 0;

    /// 은신해재
    virtual void releaseHidden(const UnionEntityInfo& creatureInfo) = 0;

    /// 사용자 비석이 세워졌다.
    virtual void playerGraveStoneStood(go::GraveStone& graveStone) = 0;

    /// 속도 변경
    virtual void moveSpeedChanged(const GameObjectInfo& creatureInfo, float32_t currentSpeed) = 0;

    /// 광분
    virtual void frenzied(const GameObjectInfo& creatureInfo, float32_t currentScale, bool isActivate) = 0;

    virtual void knockbacked(const GameObjectInfo& creatureInfo, const Position& position) = 0;
    virtual void knockbackReleased(const GameObjectInfo& creatureInfo) = 0;

    virtual void dashing(const GameObjectInfo& creatureInfo, const Position& position) = 0;

    virtual void reviveEffected(const GameObjectInfo& creatureInfo, const ObjectPosition& position, HitPoint currentHp) = 0;

    virtual void effectAdded(const GameObjectInfo& creatureInfo,
        const DebuffBuffEffectInfo& info) = 0;
    virtual void effectRemoved(const GameObjectInfo& creatureInfo,
        DataCode dataCode, bool isCaster) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace gc {