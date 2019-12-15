#include "ZoneServerPCH.h"
#include "CastableItemService.h"
#include "../../model/item/impl/CastableQuestItem.h"
#include "../../model/item/impl/CastableElementItem.h"
#include "../../model/gameobject/Entity.h"
#include <gideon/cs/datatable/QuestItemTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/ItemActiveSkillTable.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/SkillTemplate.h>

namespace gideon { namespace zoneserver {

SNE_DEFINE_SINGLETON(CastableItemService);

CastableItemService::CastableItemService()
{

}


bool CastableItemService::initialize()
{
    if (! CASTABLE_ITEM_SERVICE) {
        return false;
    }

    CastableItemService::instance(this);
    return true;
}


std::unique_ptr<CastableItem> CastableItemService::createCastableItemFor(go::Entity& caster,
    ObjectId itemId, DataCode itemCode)
{
    SkillCode skillCode = invalidSkillCode;
    CodeType codeType = getCodeType(itemCode);
    if (ctQuestItem == codeType) {
        const datatable::QuestItemTemplate* questItemTemplate = 
            QUEST_ITEM_TABLE->getQuestItemTemplate(itemCode);
        if (questItemTemplate) {
            skillCode = questItemTemplate->getSkillCode();
            
        }
    }
    else if (ctElement == codeType) {
        const gdt::element_t* element =  ELEMENT_TABLE->getElement(itemCode);
        if (element) {
            skillCode = element->skill_code();            
        }
    }

    if (isValidSkillCode(skillCode)) {
        const datatable::ItemActiveSkillTemplate* skillTemplate =
            ITEM_ACTIVE_SKILL_TABLE->getItemSkill(skillCode);
        if (! skillTemplate) {
            return nullptr;
        }
        const datatable::SkillEffectTemplate* effectTemplate =
            SKILL_EFFECT_TABLE->getSkillEffectTemplate(skillTemplate->effectIndex_);
        if (! effectTemplate) {
            return nullptr;
        }
        if (ctQuestItem == codeType) {
            return std::make_unique<CastableQuestItem>(caster, itemId, itemCode, *skillTemplate,
                *effectTemplate);
        }
        else if (ctElement == codeType) {
            return std::make_unique<CastableElementItem>(caster, itemId, itemCode, *skillTemplate,
                *effectTemplate);
        }
    }

    return nullptr;
}

}} // namespace gideon { namespace zoneserver {
