#include "ZoneServerPCH.h"
#include "StaticObjectSkill.h"
#include "../../model/gameobject/status/StaticObjectStatus.h"
#include "../../model/gameobject/StaticObject.h"
#include "../../model/gameobject/ability/CastCheckable.h"
#include "../../model/gameobject/ability/Castable.h"
#include "../../model/gameobject/ability/Castnotificationable.h"
#include "../../controller/PlayerController.h"
#include "../../helper/CastChecker.h"
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/shared/data/CastInfo.h>
#include <gideon/3d/3d.h>
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver {

namespace {


} // namespace

// = Skill

StaticObjectSkill::StaticObjectSkill(go::Entity& caster, bool shouldCheckSafeRegion,
    const datatable::SkillEffectTemplate& skillEffectTempate, 
    const datatable::SOActiveSkillTemplate& skillTemplate) :
    Skill(caster, shouldCheckSafeRegion, skillEffectTempate),
	skillTemplate_(skillTemplate),
    skillChecker_(caster, shouldCheckSafeRegion, effectTemplate_, skillTemplate)
{
}


StaticObjectSkill::~StaticObjectSkill()
{
}


void StaticObjectSkill::active(go::Entity* target, const Position& targetPosition)
{
    consumePoints();
    consumeMaterialItem();

    TargetingType targetingType = effectTemplate_.checkInfo_.targetingType_;
    EffectDefenceType effectDefenceType = edtNone;
    if (ttTarget == effectTemplate_.checkInfo_.targetingType_ && ! isBuffSkillType(skillTemplate_.skillType_) && target) {
        effectDefenceType = caster_.getCastChecker().checkTargetDefence(*target, skillTemplate_.skillType_, false, abtNone);
    }

    CompleteCastResultInfo completeCastInfo;
    if (isAreaEffectSkill(targetingType)) {
        completeCastInfo.set(uctSkillAt, caster_.getGameObjectInfo(), targetPosition, getSkillCode());
    }
    else if (isToIndividualEffectSkill(targetingType)) {
        completeCastInfo.set(uctSkillTo, caster_.getGameObjectInfo(),
            target->getGameObjectInfo(), getSkillCode(), effectDefenceType);
    }
    caster_.queryCastNotificationable()->notifyCompleteCasting(completeCastInfo);

    const CastType castType = skillTemplate_.castingInfo_.castType_;
    if (isDirectCastSkill(castType)) {
        Skill::casted(target, targetPosition, effectDefenceType);
    }
    else if (isProjectileCastSkill(castType)) {
        fireProjectile(target, targetPosition, skillTemplate_.projectileSpeed_, effectDefenceType);
    }
}

//
//void StaticObjectSkill::individualEffected(go::Entity& target, const Position& /*centerPosition*/,
//    bool /*isTargetDefence*/, bool isCasterEffect)
//{
//    // TODO: target 유효성 문제
//    gc::EffectCallback* effectCallback =
//        target.getController().queryEffectCallback();
//    if (! effectCallback) {
//        SNE_LOG_ERROR("skill not find skill effect callback!! ObjectInfo(%" PRIu64 ", %u)", 
//            target.getObjectId(), target.getObjectType());
//        return;
//    }
//  
//
//    SkillEffectResult skillEffectResult(getSkillCode());
//    if (target.isCreature()) {
//        applyCreatureEffectInfo(skillEffectResult, target); 
//    }
//    else {
//        applyStaticObjectEffectInfo(skillEffectResult, target, isCasterEffect);  
//    }
//    
//    effectCallback->applyEffect(caster_, skillEffectResult); 
//}
//
//
//void StaticObjectSkill::applyStaticObjectEffectInfo(SkillEffectResult& skillEffectResult,
//    go::Entity& target, bool isCasterEffect)
//{
//    skillEffectResult, isCasterEffect;
//    if (! target.isStaticObject()) {
//        return ;
//    }
//    go::StaticObject& effectee = static_cast<go::StaticObject&>(target);
//    const HitPoints hitPoints = effectee.getCurrentPoints();
//    //const AttributeRate registRate = effectee.getResistRate();
//    //skillTemplate_.applyStaticObjectSkillEffect(skillEffectResult, hitPoints, 0, isCasterEffect);
//}
//
//
//void StaticObjectSkill::applyCreatureEffectInfo(SkillEffectResult& skillEffectResult,
//    go::Entity& target)
//{        
//    skillEffectResult;
//    CreatureStatusInfo targetGameState;
//    CreatureStatusInfo targetOrgGameState;
//    const go::CreatureStatusable* targetCreatureStatusable = target.queryCreatureStatusable();
//    if (targetCreatureStatusable) {
//        targetGameState = targetCreatureStatusable->getCurrentCreatureStatusInfo();
//        targetOrgGameState = targetCreatureStatusable->getCreatureStatusInfo();
//    }
//
//    const UnionEntityInfo& unionEntityInfo = target.getUnionEntityInfo();
//    const Points& targetPoints = unionEntityInfo.asCreatureInfo().currentPoints_;
//    targetPoints ;
//    //skillTemplate_.applyCreatureSkillEffect(skillEffectResult, targetPoints, targetOrgGameState,
//    //    targetGameState);
//}


const datatable::ActiveSkillTemplate& StaticObjectSkill::getActiveSkillTemplate() const
{
    return skillTemplate_;
}


const datatable::SkillCastCheckPointInfo& StaticObjectSkill::getSkillCastCheckPointInfo() const
{
    return skillTemplate_.castCheckPoint_;
}


void StaticObjectSkill::consumePoints()
{
/*    if (soSkillCastInfo_.consumedHp_ > 0) {
        go::StaticObject& caster = static_cast<go::StaticObject&>(caster_);
        go::StaticObjectStatus* lifeStats = caster.getCreatureStatus();
        if (lifeStats) {
            lifeStats->reduceHp(soSkillCastInfo_.consumedHp_);
        }
    }*/    
}


void StaticObjectSkill::consumeMaterialItem()
{
    //go::SkillCastable* skillCastable = caster_.querySkillCastable();
    //if (skillCastable) {
    //    skillCastable->consumeMaterialItem(skillTemplate_.get);
    //}
}

}} // namespace gideon { namespace zoneserver {
