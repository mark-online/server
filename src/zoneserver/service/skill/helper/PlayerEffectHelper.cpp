#include "ZoneServerPCH.h"
#include "PlayerEffectHelper.h"
#include "../../../model/gameobject/Player.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/Chaoable.h"
#include "../../../model/state/FieldDuelState.h"
#include "../../../model/state/CreatureState.h"
#include "../../../world/World.h"
#include "../../../world/WorldMap.h"
#include "../../../helper/CastChecker.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/WorldMapTable.h>

namespace gideon { namespace zoneserver {

namespace {

inline bool isBeginnerGlobalMap() 
{
    const gdt::map_t* mapTemplate = WORLDMAP_TABLE->getMap(WORLD->getWorldMapCode()); 
    assert(mapTemplate != nullptr);
    return wtBeginner == WorldType(mapTemplate->world_type());
}

} // namespace


PlayerEffectHelper::PlayerEffectHelper(go::Entity& caster) :
    CreatureEffectHelper(caster)
{

}


PlayerEffectHelper::~PlayerEffectHelper()
{

}


void PlayerEffectHelper::applyTargetEffect_i(SkillEffectResult& skillEffectResult, SkillType skillType, 
    const EffectValues& effectValues, go::Entity& target, 
    const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType defenceType)
{
    CreatureEffectHelper::applyTargetEffect_i(skillEffectResult, skillType, effectValues, target, effectTemplate, defenceType);

    go::Player& caster = getCasterAs<go::Player>();
    if (! caster.isInArenaMap()) {
        updateCaotic(skillType, effectTemplate, target);
    }
}


ErrorCode PlayerEffectHelper::canTargetEffected(SkillType skillType, bool isCheckSafeRegion, EffectLevel effectLevel,
    const datatable::SkillCastCheckPointInfo& checkPointInfo, const datatable::SkillEffectTemplate& effectTemplate, go::Entity& target) const
{
    ErrorCode errorCode = EffectHelper::canTargetEffected(skillType, isCheckSafeRegion, effectLevel, checkPointInfo, effectTemplate, target);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    if (target.isPlayer() && effectTemplate.hasReviveEffect()) {
        if (! target.queryCreatureState()->isDied()) {
            return ecSkillInvalidAvailableTarget;
        }
    }

    if (effectTemplate.hasDashEffect() && target.isBuilding()) {
        return ecSkillInvalidAvailableTarget;
    }

    if (checkPointInfo.checkStatTarget_ == csttCastee) {
        errorCode = checkCastableTargetStatus(checkPointInfo, target);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }

    const go::Player& caster = getCasterAs<go::Player>();

    if (! caster.isInArenaMap()) {
        errorCode = checkTargetEffectable(skillType, effectTemplate, isCheckSafeRegion, target);        
    }

    return errorCode;
}


ErrorCode PlayerEffectHelper::checkTargetEffectable(SkillType skillType, const datatable::SkillEffectTemplate& effectTemplate,
    bool isCheckSafeRegion, go::Entity& target) const
{   
    const go::Player& caster = getCasterAs<go::Player>();
    ErrorCode errorCode = caster.getCastChecker().checkSafeRegion(isCheckSafeRegion, target.getPosition());
    if (isFailed(errorCode)) {
        return errorCode;
    }
    
    if (caster.isFieldDueling()) {
        if (! caster.isMyDueler(target)) {
            return ecSkillInvalidAvailableTarget;            
        }
    }
    else {
        if ((isAttackOrDebuffSkillType(skillType) || effectTemplate.hasExecutionEffect()) && target.isPlayer()) {
            /*if (isBeginnerGlobalMap()) {
                return ecSkillNotCastToPlayerInBeginnerMap;
            }*/
            if (target.queryProtectionable()->isBeginnerProtection() ||
                caster.queryProtectionable()->isBeginnerProtection()) {
                return ecSkillProtectionCharacter;
            }
        }
    }

    if (isBeginnerGlobalMap() && target.isNpc() && ! isCritter(getNpcType(target.getEntityCode()))) {
        if (! caster.queryChaoable()->isChao()) {
            return ecSkillNotAttackTatget;
        }
    }

    errorCode = checkAttackOrDebufEffectable(skillType, effectTemplate.checkInfo_.targetingType_, target);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    if (! caster.isFieldDueling()) {
        if (isAttackOrDebuffSkillType(skillType) && checkProtectionCharacter(target)) {
            return ecSkillProtectionCharacter;
        }
    }

    return ecOk;
}


ErrorCode PlayerEffectHelper::checkAttackOrDebufEffectable(SkillType skillType, 
    TargetingType targetingType, go::Entity& target) const
{
    if (! target.isPlayer()) {
        return ecOk;
    }
    const go::Player& caster = getCasterAs<go::Player>();
    if (isAreaEffectSkill(targetingType) && isAttackOrDebuffSkillType(skillType)) {
        if (caster.isMyPartyMember(target)) {
            return ecSkillNotAttackTatget;
        }        
    }

    if (! caster.isInArenaMap()) {
        return ecOk;
    }
    
    if (isAttackOrDebuffSkillType(skillType) && caster.isSameTeamInArena(target)) {
        return ecSkillNotAttackTatget;
    }

    return ecOk;
}


bool PlayerEffectHelper::checkProtectionCharacter(go::Entity& target) const
{
    if (target.isPlayer() && target.queryProtectionable()->isBeginnerProtection()) {
        return true;        
    }

    if (target.isPlayer() && getCaster().queryProtectionable()->isBeginnerProtection()) {
        return true;
    }

    return false;
}


bool PlayerEffectHelper::isAvailableTarget(go::Entity& target, AvailableTarget availableTarget) const
{
    if (target.isNpcOrMonster()) {
        if (static_cast<go::Npc&>(target).isStaticNpc()) {
            return false;
        }
    }

    if (availableTarget == atAll) {
        return true;
    }

    const go::Player& caster = getCasterAs<go::Player>();
    if (availableTarget == atGuild || availableTarget  == atParty || availableTarget == atGuildOrParty) {
        if (caster.isSame(target)) {
            return true;
        }
    }
    if (availableTarget == atParty) {
        return caster.isMyPartyMember(target);
    }
    else if (availableTarget == atGuild) {
        return caster.isMyGuildMember(target);
    }
    else if (availableTarget == atGuildOrParty) {
        return caster.isMyPartyMember(target) || caster.isMyGuildMember(target);
    }
    else if (availableTarget == atMoster) {
        if (! target.isMonster()) {
            return false;
        }
    }
    return true;
}


bool PlayerEffectHelper::isExceptTarget(go::Entity& target, ExceptTarget exceptTarget) const
{
    if (etNoneTaget ==  exceptTarget) {
        return false;   
    }

    if (etSelf == exceptTarget) {
        return getCaster().isSame(target);
    }
    const go::Player& caster = getCasterAs<go::Player>();
    if (exceptTarget == etGuild || exceptTarget  == etParty || exceptTarget == etPartyOrGuild) {
        if (caster.isSame(target)) {
            return true;
        }
    }
    if (exceptTarget == etParty) {
        return caster.isMyPartyMember(target);
    }
    else if (exceptTarget == etGuild) {
        return caster.isMyGuildMember(target);
    }
    else if (exceptTarget == etPartyOrGuild) {
        return caster.isMyPartyMember(target) || caster.isMyGuildMember(target);
    }
    else if (etNpcOrMonster == exceptTarget) {
        return target.isNpcOrMonster();
    }
    else if (etNpcOrMosterOrSelf == exceptTarget) {
        if (getCaster().isSame(target) || target.isMonster()) {
            return true;
        }
    }

    return false;
}



void PlayerEffectHelper::updateCaotic(SkillType skillType, const datatable::SkillEffectTemplate& effectTemplate, go::Entity& target)
{
    if (getCaster().isSame(target)) {
        return;
    }

    go::Chaoable* casterChaoable = getCaster().queryChaoable();
    if (! casterChaoable) {
        return;
    }

    const go::Chaoable* targetChaoable = target.queryChaoable();
    if (! targetChaoable) {
        if (target.isNpc()) {
            NpcType npcType = getNpcType(target.getEntityCode());
            if (! (isCritter(npcType) || isHireableNpc(npcType))) {
                casterChaoable->hitToPlayer();
            }
        }
        return;
    }

    if (getCaster().queryFieldDuelState()->isFieldDueling()) {
        return;
    }

    // 임시 카오
    if (isAttackSkillType(skillType)) {
        if (! casterChaoable->isRealChao() && ! targetChaoable->isChao()) {
            casterChaoable->hitToPlayer();
        }
    }
    //  비석 카오
    if (effectTemplate.hasExecutionEffect()) {        
        if (! targetChaoable->isChao()) {
            casterChaoable->standGraveStone();
        }        
    }
}


}} // namespace gideon { namespace zoneserver {

