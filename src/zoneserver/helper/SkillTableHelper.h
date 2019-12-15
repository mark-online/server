#pragma once

#include <gideon/cs/datatable/PlayerActiveSkillTable.h>
#include <gideon/cs/datatable/NpcActiveSkillTable.h>
#include <gideon/cs/datatable/ItemActiveSkillTable.h>
#include <gideon/cs/datatable/SoActiveSkillTable.h>
#include <gideon/cs/datatable/PlayerPassiveSkillTable.h>

namespace gideon { namespace zoneserver {


inline const datatable::ActiveSkillTemplate* getActiveSkillTemplate(DataCode dataCode)
{
    CodeType ct = getCodeType(dataCode);
    if (ctSkill == ct) {
        SkillTableType skillTableType = getSkillTableType(dataCode);
        if (skillTableType == sttActivePlayer) {
            return PLAYER_ACTIVE_SKILL_TABLE->getPlayerSkill(dataCode);
        }
        else if (skillTableType == sttActiveNpc) {
            return NPC_ACTIVE_SKILL_TABLE->getNpcSkill(dataCode);
        }
        else if (skillTableType == sttActiveItem) {
            return ITEM_ACTIVE_SKILL_TABLE->getItemSkill(dataCode);
        }
        else if (skillTableType == sttActiveSo) {
            return SO_ACTIVE_SKILL_TABLE->getSOSkill(dataCode);
        }
    }

    return nullptr;
}


inline const SkillType getSkillType(DataCode dataCode)
{
    CodeType ct = getCodeType(dataCode);
    if (! isSkillType(ct)) {
        assert(false);
        return sktGeneral;
    }


    if (isActiveSkillTable(getSkillTableType(dataCode))) {
        const datatable::ActiveSkillTemplate* skillTemplate = getActiveSkillTemplate(dataCode);
        if (skillTemplate) {
            return skillTemplate->skillType_;
        }
    }

    const datatable::PassiveSkillTemplate* skillTemplate = PLAYER_PASSIVE_SKILL_TABLE->getPlayerSkill(dataCode);
    if (skillTemplate) {
        return skillTemplate->skillType_;
    }

    return sktGeneral; 
}

inline permil_t getTopThreatPerValue(DataCode dataCode)
{
    CodeType ct = getCodeType(dataCode);
    if (! isSkillType(ct)) {
        assert(false);
        return 0;
    }


    if (isActiveSkillTable(getSkillTableType(dataCode))) {
        const datatable::ActiveSkillTemplate* skillTemplate = getActiveSkillTemplate(dataCode);
        if (skillTemplate) {
            return skillTemplate->topThreadApplyValue_;
        }
        assert(false);
        return 0;
    }

    const datatable::PassiveSkillTemplate* skillTemplate = PLAYER_PASSIVE_SKILL_TABLE->getPlayerSkill(dataCode);
    if (skillTemplate) {
        return skillTemplate->topThreadApplyValue_;
    }

    return 0; 
}


inline bool isDotEffect(bool isCaster, SkillCode dataCode, EffectApplyNum num)
{
    if (isActiveSkillTable(getSkillTableType(dataCode))) {
        const datatable::ActiveSkillTemplate* skillTemplate = getActiveSkillTemplate(dataCode);
        if (skillTemplate) {
            if (isCaster) {
                return skillTemplate->casterSkillActivateTimeInfos_[num].isDot();
            }
            else {
                return skillTemplate->casterSkillActivateTimeInfos_[num].isDot();
            }            
        }
        assert(false);
        return false;
    }

    const datatable::PassiveSkillTemplate* skillTemplate = PLAYER_PASSIVE_SKILL_TABLE->getPlayerSkill(dataCode);
    if (skillTemplate) {
        if (isCaster) {
            return skillTemplate->casterSkillActivateTimeInfos_[num].isDot();
        }
        else {
            return skillTemplate->casterSkillActivateTimeInfos_[num].isDot();
        } 
    }

    return false;
}
}} // namespace gideon { namespace zoneserver {
