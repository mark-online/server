#include "ZoneServerPCH.h"
#include "CastableQuestItem.h"
#include "../../gameobject/Entity.h"
#include "../../gameobject/ability/Chaoable.h"
#include "../../gameobject/ability/Questable.h"
#include "../../gameobject/ability/CreatureStatusable.h"
#include "../../gameobject/ability/Chaoable.h"
#include "../../gameobject/ability/ItemCastable.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/EffectCallback.h"
#include "../../../ai/evt/EventTriggerManager.h"
#include "../../quest/QuestRepository.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/shared/data/ItemEffectInfo.h>


namespace gideon { namespace zoneserver {

CastableQuestItem::CastableQuestItem(go::Entity& caster, ObjectId itemId, DataCode itemCode,
    const datatable::ItemActiveSkillTemplate& itemSkillTemplate,
    const datatable::SkillEffectTemplate& effectTemplate) :
    CastableItem(caster, itemId, itemCode, itemSkillTemplate, effectTemplate)
{

}


CastableQuestItem::~CastableQuestItem()
{
}


ErrorCode CastableQuestItem::checkItem()
{
    assert(isQuestItemType(getCodeType(getItemCode())));

    go::ItemCastable* itemCastable = getOwner().queryItemCastable();
    if (itemCastable) {
        if (! itemCastable->checkQuestItem(getObjectId())) {
            return ecItemNotEnoughUseable;
        }
    }

    assert(getOwner().isPlayer());
    go::Entity& owner = static_cast<go::Entity&>(getOwner());
    return owner.queryQuestable()->getQuestRepository().checkActivationMission(getItemCode());
}


void CastableQuestItem::useItem()
{
    go::ItemCastable* itemCastable = getOwner().queryItemCastable();
    if (itemCastable) {
        itemCastable->useQuestItem(getObjectId());
    }

    assert(getOwner().isPlayer());
    go::Entity& owner = static_cast<go::Entity&>(getOwner());
    const ErrorCode errorCode =
        owner.queryQuestable()->getQuestRepository().completeActivationMission(getItemCode());
    if (isSucceeded(errorCode)) {
        EVT_MANAGER.trigger(ai::evt::evtItemUsed, owner, getItemCode());
    }
}

}} // namespace gideon { namespace zoneserver {
