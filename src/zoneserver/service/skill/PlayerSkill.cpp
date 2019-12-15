#include "ZoneServerPCH.h"
#include "PlayerSkill.h"
#include "../../world/WorldMap.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/ability/ArenaMatchable.h"
#include "../../model/gameobject/ability/CombatStateable.h"
#include "../../model/gameobject/ability/Protectionable.h"
#include "../../model/gameobject/ability/Chaoable.h"
#include "../../model/gameobject/ability/SkillCastable.h"
#include "../../model/gameobject/ability/CastCheckable.h"
#include "../../model/gameobject/ability/PassiveSkillCastable.h"
#include "../../model/gameobject/ability/Partyable.h"
#include "../../model/state/FieldDuelState.h"
#include "../../model/state/CreatureState.h"
#include "../../model/state/SkillCasterState.h"
#include "../../service/arena/mode/Arena.h"
#include "../../service/party/Party.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/SkillCallback.h"
#include "../../controller/callback/EntityStatusCallback.h"
#include "../../helper/CastChecker.h"
#include "../time/GameTimer.h"
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/3d/3d.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

namespace {

template <typename T>
inline bool isEnoughPointStat(T src, T dec)
{
    if ((src > 0) && (dec < src)) {
        return false;
    }
    return true;
}

} // namespace

// = Skill

PlayerSkill::PlayerSkill(go::Entity& caster, bool shouldCheckSafeRegion,
    const datatable::SkillEffectTemplate& skillEffectTemplate,
	const datatable::PlayerActiveSkillTemplate& skillTemplate) :
    CreatureSkill(caster, shouldCheckSafeRegion, skillEffectTemplate),
	skillTemplate_(skillTemplate),
    playerSkillChecker_(caster, shouldCheckSafeRegion, skillEffectTemplate, skillTemplate)
{
}


PlayerSkill::~PlayerSkill()
{
}


ErrorCode PlayerSkill::startCasting(go::Entity* target, const Position& targetPosition,
	float32_t distanceToTargetSq)
{
	const ErrorCode errorCode = Skill::startCasting(target, targetPosition, distanceToTargetSq);
	if (isSucceeded(errorCode)) {
		changeCharacterState();
	}
	return errorCode;
}


void PlayerSkill::active(go::Entity* target, const Position& targetPosition)
{
    CreatureSkill::active(target, targetPosition);

    consumePoints();
	consumeMaterialItem();
}


void PlayerSkill::changeCharacterState()
{
    if (stsNone == skillTemplate_.transitionState_) {
        return;
    }

    bool isChange = false;
    bool isCombatState = false;
    if (stsPeace == skillTemplate_.transitionState_) {
        isChange = true;
		caster_.queryCombatStateable()->updateLockPeaceTime(10000);
    }
    else if (stsCombat == skillTemplate_.transitionState_) {
        isCombatState = true;
        isChange = true;
    }

    // TODO: 인터페이스 다른곳에 두어야 할지 상의
    if (isChange) {
        gc::EntityStatusCallback* callback = caster_.getController().queryEntityStatusCallback();
        if (callback) {
            callback->playerCombatStateChanged(isCombatState);
        }
    }
	if (skillTemplate_.isAttackSkill() || skillTemplate_.isDebuffSkill()) {
		caster_.queryCombatStateable()->updateAttackTime(skillTemplate_.castingInfo_.castTime_);
	}
}


void PlayerSkill::consumePoints()
{
    // FYI: NPC는 HP/MP/CP 소모를 하지 않는다
    
    go::SkillCastable* skillCastable = caster_.querySkillCastable();
    assert(skillCastable != nullptr);
    skillCastable->consumePoints(skillTemplate_.consumePoints_);
}


void PlayerSkill::consumeMaterialItem()
{
    go::SkillCastable* skillCastable = caster_.querySkillCastable();
    assert(skillCastable != nullptr);
    skillCastable->consumeMaterialItem(skillTemplate_.consumeItemInfo_);
}


const datatable::ActiveSkillTemplate& PlayerSkill::getActiveSkillTemplate() const
{
    return skillTemplate_;
}


const datatable::SkillCastCheckPointInfo& PlayerSkill::getSkillCastCheckPointInfo() const
{
    return skillTemplate_.castCheckPoint_;
}


bool PlayerSkill::isUsing() const
{
    if (! Skill::isUsing()) {
        return false;
    }
    sne::base::Future::Ref activeConcentrationTask = activeConcentrationTask_.lock();
    if (! activeConcentrationTask.get()) {
        return false;
    }

    return (! activeConcentrationTask->isCancelled()) && (! activeConcentrationTask->isDone());
}

}} // namespace gideon { namespace zoneserver {
