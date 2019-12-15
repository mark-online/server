#include "ZoneServerPCH.h"
#include "SkillService.h"
#include "PlayerSkill.h"
#include "NpcSkill.h"
#include "StaticObjectSkill.h"
#include "../../model/gameobject/Entity.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/PlayerActiveSkillTable.h>
#include <gideon/cs/datatable/PlayerPassiveSkillTable.h>
#include <gideon/cs/datatable/NpcActiveSkillTable.h>
#include <gideon/cs/datatable/SoActiveSkillTable.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <sne/server/common/Property.h>

namespace gideon { namespace zoneserver {

SNE_DEFINE_SINGLETON(SkillService);

SkillService::SkillService() :
    shouldCheckSafeRegion_(true)
{
}


bool SkillService::initialize()
{
    if (!SKILL_EFFECT_TABLE) {
        return false;
    }
    if (!PLAYER_ACTIVE_SKILL_TABLE) {
        return false;
    }
    if (!NPC_ACTIVE_SKILL_TABLE) {
        return false;
    }
    if (!SO_ACTIVE_SKILL_TABLE) {
        return false;
    }

    try {
        shouldCheckSafeRegion_ =
            ! SNE_PROPERTIES::getProperty<bool>("zone_server.dont_check_safe_region");
    }
    catch (boost::bad_lexical_cast&) {}

    SkillService::instance(this);
    return true;
}


std::unique_ptr<Skill> SkillService::createSkillFor(go::Entity& caster,
    SkillCode skillCode)
{
	if (caster.isPlayer()) {
		const datatable::PlayerActiveSkillTemplate* skillTemplate =
            PLAYER_ACTIVE_SKILL_TABLE->getPlayerSkill(skillCode);
		if (skillTemplate != nullptr) {
            const datatable::SkillEffectTemplate* effectTemplate = 
                SKILL_EFFECT_TABLE->getSkillEffectTemplate(skillTemplate->effectIndex_);
            if (effectTemplate != nullptr) {
                return std::make_unique<PlayerSkill>(caster, shouldCheckSafeRegion_,
                    *effectTemplate, *skillTemplate);
            }
		}
	}
	else if (caster.isNpcOrMonster()){
		const datatable::NpcActiveSkillTemplate* skillTemplate =
            NPC_ACTIVE_SKILL_TABLE->getNpcSkill(skillCode);
		if (skillTemplate != nullptr) {
            const datatable::SkillEffectTemplate* effectTemplate = 
                SKILL_EFFECT_TABLE->getSkillEffectTemplate(skillTemplate->effectIndex_);
            if (effectTemplate != nullptr) {
                return std::make_unique<NpcSkill>(caster, shouldCheckSafeRegion_,
                    *effectTemplate, *skillTemplate);
            }

		}
	}
    else if (caster.isAnchorOrBuilding()) {
        const datatable::SOActiveSkillTemplate* skillTemplate =
            SO_ACTIVE_SKILL_TABLE->getSOSkill(skillCode);
        if (skillTemplate != nullptr) {
            const datatable::SkillEffectTemplate* effectTemplate = 
                SKILL_EFFECT_TABLE->getSkillEffectTemplate(skillTemplate->effectIndex_);
            if (effectTemplate != nullptr) {
                return std::make_unique<StaticObjectSkill>(caster, shouldCheckSafeRegion_,
                    *effectTemplate, *skillTemplate);
            }
        }
    }
    
    return nullptr;
}

}} // namespace gideon { namespace zoneserver {
