#pragma once

#include "../CreatureEffectController.h"
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class NpcEffectController
 * 스킬 담당
 */
class NpcEffectController : public CreatureEffectController
{
public:
    NpcEffectController(go::Entity* owner) :
        CreatureEffectController(owner) {}

private:
	// = EffectCallback overriding
    virtual void effectApplied(go::Entity& to, go::Entity& from,
        const SkillEffectResult& skillEffectResul);
    virtual void effectHit(go::Entity& to, go::Entity& from, DataCode dataCode);

    // = CreatureEffectCallback overriding
    virtual void standGraveStone() {}
    virtual void playerGraveStoneStood(go::GraveStone& /*graveStone*/) {}
    virtual void mesmerizationEffected(const GameObjectInfo& /*creatureInfo*/, MesmerizationType /*mezt*/, bool /*isActivate*/) {}
    virtual void mutated(const GameObjectInfo& /*creatureInfo*/, NpcCode /*npcCode*/,
        bool /*isActivate*/)  {}
    virtual void transformed(const GameObjectInfo& /*creatureInfo*/, NpcCode /*npcCode*/,
        bool /*isActivate*/) {}
    virtual void releaseHidden(const UnionEntityInfo& /*creatureInfo*/) {}
    virtual void frenzied(const GameObjectInfo& /*creatureInfo*/, float32_t /*currentScale*/, bool /*isActivate*/) {}
    virtual void moveSpeedChanged(const GameObjectInfo& /*creatureInfo*/, float32_t /*curretSpeed*/) {}
    virtual void knockbacked(const GameObjectInfo& /*creatureInfo*/, const Position& /*position*/) {}
    virtual void knockbackReleased(const GameObjectInfo& /*creatureInfo*/) {}
    virtual void dashing(const GameObjectInfo& /*creatureInfo*/, const Position& /*position*/) {}
    virtual void reviveEffected(const GameObjectInfo& /*creatureInfo*/, const ObjectPosition& /*position*/, HitPoint /*currentHp*/) {}

	virtual void effectAdded(const GameObjectInfo& /*creatureInfo*/,
        const DebuffBuffEffectInfo& /*info*/) {}
	virtual void effectRemoved(const GameObjectInfo& /*creatureInfo*/,
		DataCode /*dataCode*/, bool /*isCaster*/) {}

};


}}} // namespace gideon { namespace zoneserver { namespace gc {