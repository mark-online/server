#include "ZoneServerPCH.h"
#include "PlayerSkillChecker.h"
#include "../../party/Party.h"
#include "../../arena/mode/Arena.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/Player.h"
#include "../../../model/gameobject/ability/CastCheckable.h"
#include "../../../model/gameobject/ability/ArenaMatchable.h"
#include "../../../model/gameobject/ability/Protectionable.h"
#include "../../../model/gameobject/ability/Partyable.h"
#include "../../../model/gameobject/ability/Guildable.h"
#include "../../../model/state/SkillCasterState.h"
#include "../../../model/state/FieldDuelState.h"
#include "../../../model/state/CreatureState.h"
#include "../../../world/WorldMap.h"
#include "../../../world/World.h"
#include "../../../helper/CastChecker.h"
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/WorldMapTable.h>

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

inline bool isBeginnerGlobalMap() 
{
    const gdt::map_t* mapTemplate = WORLDMAP_TABLE->getMap(WORLD->getWorldMapCode()); assert(mapTemplate != nullptr);
    return wtBeginner == WorldType(mapTemplate->world_type());
}


} // namespace

PlayerSkillChecker::PlayerSkillChecker(go::Entity& caster, bool shouldCheckSafeRegion,
    const datatable::SkillEffectTemplate& effectTemplate,
    const datatable::PlayerActiveSkillTemplate& skillTemplate) :
    SkillChecker(caster, shouldCheckSafeRegion, effectTemplate),
    skillTemplate_(skillTemplate)
{
}


ErrorCode PlayerSkillChecker::canCastable() const
{
    ErrorCode errorCode = SkillChecker::canCastable();
    if (isFailed(errorCode)) {
        return errorCode;
    }

    errorCode = checkCastableCurrentState();
    if (isFailed(errorCode)) {
        return errorCode;
    }

    if (! hasConsumeMaterialItem()) {
        return ecSkillNotEnoughMetarialItem;
    }

    if (shouldCheckByArena()) {
        if (skillEffectTemplate_.hasExecutionEffect()) {
            return ecSkillCannotCastableInArena;
        }
    }
    go::Player& caster = static_cast<go::Player&>(caster_);
    if (caster.isFieldDueling()) {
        if (skillEffectTemplate_.hasExecutionEffect()) {
            return ecDuelNotCastingSkill;
        }
    }

    return ecOk;
}


bool PlayerSkillChecker::checkConcentrationSkill(go::Entity* target, const Position& targetPosition) const
{
    
    TargetingType targetingType = skillEffectTemplate_.checkInfo_.targetingType_;
    if (targetingType == ttSelf || targetingType == ttSelfArea) {
        return true;
    }
    const datatable::DistanceRangeInfo& distanceRange = skillTemplate_.skillCastCheckDistanceInfo_;
    float32_t distanceToTargetSq = 0.0f;
    if (targetingType == ttTarget || targetingType == ttTargetArea) {
        const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
            distanceRange.maxDistance_, distanceRange.minDistance_, *target);
        if (isFailed(ecDistance)) {
            return false;
        }
    }
    else if (targetingType == ttArea) {
        const ErrorCode ecDistance = caster_.getCastChecker().checkDistance(distanceToTargetSq,
            distanceRange.maxDistance_, distanceRange.minDistance_, targetPosition);
        if (isFailed(ecDistance)) {
            return false;
        }
    }

    if (targetingType == ttTarget) {
        const ErrorCode ecDirection = caster_.getCastChecker().checkDirection(*target, targetingType,
            skillEffectTemplate_.checkInfo_.targetDirection_, skillEffectTemplate_.checkInfo_.targetOrientation_);
        if (isFailed(ecDirection)) {
            return false;
        }
    }

    return true;
}

const datatable::ActiveSkillTemplate& PlayerSkillChecker::getActiveSkillTemplate() const
{
    return skillTemplate_;
}


bool PlayerSkillChecker::canCastableState() const
{
    SkillCasterState* state = caster_.querySkillCasterState();
    if (! state) {
        return false;
    }

    return state->canCastSkill(skillTemplate_.checkCategory_ != escNone);    
}


ErrorCode PlayerSkillChecker::checkCastableCurrentState() const
{
    CreatureState* creatureState = caster_.queryCreatureState();
    if (! creatureState) {
        return ecServerInternalError;
    }
    if (creatureState->isDisableMagic()) {
        if (isMagicSkillType(skillTemplate_.skillType_)) {
            return ecSkillDisableMagic;
        }
    }
    go::CastCheckable* skillCastable = caster_.queryCastCheckable();
    if (! skillCastable) {
        return ecServerInternalError;
    }

    //if (isValidSkillCode(playerSkillInfo_.needSkillCode_)) {
    //    if (! skillCastable->checkCastableNeedSkill(playerSkillInfo_.needSkillCode_)) {
    //        return ecSkillHasNotNeedSkill;
    //    }
    //}
    //
    if (susAlaways != skillTemplate_.skillUseableState_) {
        if (! skillCastable->checkCastableUsableState(skillTemplate_.skillUseableState_)) {
            return ecSkillNotUseableState;
        }
    }

    if (skillTemplate_.castableCheckEquip_.castableEquipType_ != scetAny ||
        skillTemplate_.castableCheckEquip_.equipPart_ != epInvalid) {
        if (! skillCastable->checkCastableEquip(
            skillTemplate_.castableCheckEquip_.equipPart_, skillTemplate_.castableCheckEquip_.castableEquipType_)) {
                return ecSkillNotAllowedEquipment;
        }
    }

    if (skillTemplate_.castCheckPoint_.checkStatTarget_ == csttCaster) {
        const ErrorCode ec = checkCastableStatState(caster_);
        if (isFailed(ec)) {
            return ec;
        }
    }

    if (skillTemplate_.checkCategory_ != escNone && skillTemplate_.checkCategory_ != escSkipMezCheck) {
        if (! skillCastable->checkCastableEffectCategory(skillTemplate_.checkCategory_)) {
            return ecSkillNotApplySkillEffectScript;
        }
    }
    
    return checkCasterPointStats();
}


ErrorCode PlayerSkillChecker::checkCasterPointStats() const
{
    const UnionEntityInfo unionEntityInfo = caster_.getUnionEntityInfo();
    if (unionEntityInfo.asEntityInfo().isNpcOrMonster()) {
        return ecOk;
    }

    if (isFailed(caster_.queryCastCheckable()->checkCastablePoints(ptHp, false, true, skillTemplate_.consumePoints_.hp_))) {
        return ecSkillNotEnoughStats;
    }
    if (isFailed(caster_.queryCastCheckable()->checkCastablePoints(ptMp, false, true, skillTemplate_.consumePoints_.mp_))) {
        return ecSkillNotEnoughStats;
    }


    return ecOk;    
}


ErrorCode PlayerSkillChecker::checkCastableStatState(go::Entity& target) const
{    
    go::CastCheckable* castCheckable = target.queryCastCheckable();
    if (! castCheckable) {
        return ecSkillInvalidAvailableTarget;
    }

    return castCheckable->checkCastablePoints(skillTemplate_.castCheckPoint_.pointType_,
        skillTemplate_.castCheckPoint_.isUsePercent_, skillTemplate_.castCheckPoint_.isUpCheckValue_,
        skillTemplate_.castCheckPoint_.checkValue_);
}



bool PlayerSkillChecker::checkProtectionCharacter(go::Entity& target) const
{
    if (target.queryProtectionable()->isBeginnerProtection()) {
        return true;
    }

    if (caster_.queryProtectionable()->isBeginnerProtection()) {
        return true;
    }

    return false;
}


bool PlayerSkillChecker::hasConsumeMaterialItem() const
{
    if (! skillTemplate_.consumeItemInfo_.isValid()) {
        return true;
    }

    go::CastCheckable* skillCastable = caster_.queryCastCheckable();
    if (! skillCastable) {
        return false;
    }

    return skillCastable->checkCastableNeedItem(skillTemplate_.consumeItemInfo_);
}



bool PlayerSkillChecker::isAreaAttackOrDebuffSkill() const 
{
   return (isAreaEffectSkill(skillEffectTemplate_.checkInfo_.targetingType_) && 
        (skillTemplate_.isAttackSkill() || skillTemplate_.isDebuffSkill()));
}


bool PlayerSkillChecker::isAttackSkill() const
{
    return skillTemplate_.isAttackSkill();
}


bool PlayerSkillChecker::isDebuffSkill() const
{    
    return skillTemplate_.isDebuffSkill();
}


}} // namespace gideon { namespace zoneserver {