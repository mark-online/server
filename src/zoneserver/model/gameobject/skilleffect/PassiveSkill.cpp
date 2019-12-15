#include "ZoneServerPCH.h"
#include "PassiveSkill.h"
#include "CreatureEffectScriptCommand.h"
#include "../Creature.h"
#include "../status/CreatureStatus.h"
#include "../ability/SkillCastable.h"
#include "../ability/CastCheckable.h"
#include "../../../controller/callback/LifeStatusCallback.h"
#include "../../../controller/callback/PassiveSkillCallback.h"
#include "../../../controller/EntityController.h"
#include "../../../service/skill/helper/EffectHelper.h"
#include <gideon/cs/datatable/SkillEffectTable.h>


namespace gideon { namespace zoneserver { namespace go {


PassiveSkill::PassiveSkill(Creature& caster, const datatable::SkillEffectTemplate& effectTemplate,
    const datatable::PassiveSkillTemplate& passiveSkillTemplate) :
    caster_(caster),
    effectTemplate_(effectTemplate),
    skillTemplate_(passiveSkillTemplate)
{
    conditions_.fill(true);

    CastCheckable* skillCastable = caster.queryCastCheckable();
    if (skillTemplate_.shouldCheckSelfPoints()) {
        conditions_[pccPoints] = isSucceeded(skillCastable->checkCastablePoints(
            skillTemplate_.checkPoint_.pointType_, skillTemplate_.checkPoint_.isUsePercent_, 
            skillTemplate_.checkPoint_.isUpCheckValue_, skillTemplate_.checkPoint_.checkValue_));
    }

    if (skillTemplate_.shouldCheckUsableState()) {
		conditions_[pccUseableState] = 
            skillCastable->checkCastableUsableState(skillTemplate_.skillUseableState_);
    }
    if (skillTemplate_.shouldCheckEquip()) {
		conditions_[pccEquip] = skillCastable->checkCastableEquip(skillTemplate_.checkEquipItem_.equipPart_,
			skillTemplate_.checkEquipItem_.castableEquipType_);
    }
    if (skillTemplate_.shouldCheckLeanedSkill()) {
        const bool isSuccess1 = skillCastable->checkCastableNeedSkill(skillTemplate_.learnInfo_.needLearnSkillCode1_) ;
        const bool isSuccess2 = skillCastable->checkCastableNeedSkill(skillTemplate_.learnInfo_.needLearnSkillCode2_) ;
        if (isSuccess1 && isSuccess2) {
            conditions_[pccLearnSkill] = true;
        }
        else {
            conditions_[pccLearnSkill] = false;
        }
    }

    setCondition();
}


PassiveSkill::~PassiveSkill()
{
}


SkillCode PassiveSkill::getSkillCode() const
{
    return skillTemplate_.skillCode_;
}


bool PassiveSkill::isSuccessCondition() const
{    
    return isConditionSuccess_;
}


bool PassiveSkill::hasTimeSkill() const
{
    for (int i = 0; i < eanCount; ++i) {
        if (skillTemplate_.casterSkillActivateTimeInfos_[i].activateMilSec_ > 0) {
            return true;
        }
    }
    return false;
}


bool PassiveSkill::isCooldown() const
{
    if (skillTemplate_.hasCooldown()) {
        if (caster_.queryCastGameTimeable()->isLocalCooldown(getSkillCode())) {
            return true;
        }        
    }
    if (skillTemplate_.hasGlobalCooldown()) {
        if (caster_.queryCastGameTimeable()->isGlobalCooldown(skillTemplate_.cooltimeInfo_.globalCooldownTimeIndex_)) {
            return true;
        }        
    }
    return false;
}


void PassiveSkill::activate()
{
    if (caster_.queryCastGameTimeable()->isLocalCooldown(getSkillCode())) {
        isConditionSuccess_ = false;
        return;
    }

    caster_.getEffectHelper().applyCasterPassiveEffect(getSkillCode(), skillTemplate_.skillType_, 
        skillTemplate_.casterEffectValues_, effectTemplate_);

    if (effectTemplate_.checkInfo_.targetingType_ == ttSelfArea) {
        caster_.getEffectHelper().applyAreaTargetEffect(getSkillCode(), skillTemplate_.skillType_,
            skillTemplate_.effectLevel_, true, skillTemplate_.casteeEffectValues_, skillTemplate_.checkPoint_, 
            caster_.getPosition(), effectTemplate_);
    }

    if (skillTemplate_.hasCooldown()) {
        caster_.queryCastGameTimeable()->setCooldown(getSkillCode(), skillTemplate_.cooltimeInfo_.cooldownMilSec_,
            skillTemplate_.cooltimeInfo_.globalCooldownTimeIndex_, skillTemplate_.cooltimeInfo_.globalCooldownMilSec_);
    }

    caster_.getController().queryPassiveSkillCallback()->activatePassive(getSkillCode());
}


void PassiveSkill::deactivate()
{
    caster_.getEffectHelper().releaseCasterPassiveEffect(getSkillCode());
}


void PassiveSkill::changeCondition(PassiveCheckCondition condition)
{
    CastCheckable* skillCastable = caster_.queryCastCheckable();
    
    switch (condition) {
    case pccPoints:
        if (skillTemplate_.shouldCheckSelfPoints()) {
            conditions_[pccPoints] = isSucceeded(skillCastable->checkCastablePoints(
                skillTemplate_.checkPoint_.pointType_, skillTemplate_.checkPoint_.isUsePercent_, 
                skillTemplate_.checkPoint_.isUpCheckValue_, skillTemplate_.checkPoint_.checkValue_));
        }

        break;
    case pccUseableState:
        if (skillTemplate_.shouldCheckUsableState()) {
            conditions_[pccUseableState] = 
                skillCastable->checkCastableUsableState(skillTemplate_.skillUseableState_);
        }
        break;
    case pccEquip:
        if (skillTemplate_.shouldCheckEquip()) {
            conditions_[pccEquip] = skillCastable->checkCastableEquip(skillTemplate_.checkEquipItem_.equipPart_,
                skillTemplate_.checkEquipItem_.castableEquipType_);		
        }
        break;
    case pccLearnSkill:
		if (skillTemplate_.shouldCheckLeanedSkill()) {
            const bool isSuccess1 = skillCastable->checkCastableNeedSkill(skillTemplate_.learnInfo_.needLearnSkillCode1_) ;
            const bool isSuccess2 = skillCastable->checkCastableNeedSkill(skillTemplate_.learnInfo_.needLearnSkillCode2_) ;
            if (isSuccess1 && isSuccess2) {
                conditions_[pccLearnSkill] = true;
            }
            else {
                conditions_[pccLearnSkill] = false;
            }
		}
        break;
    }

    setCondition();
}


void PassiveSkill::setCondition()
{   
    isConditionSuccess_ = conditions_[pccPoints] && conditions_[pccUseableState] && 
        conditions_[pccEquip] && conditions_[pccLearnSkill];
}

}}} // namespace gideon { namespace zoneserver { namespace go {
