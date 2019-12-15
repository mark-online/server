#include "ZoneServerPCH.h"
#include "CastableElementItem.h"
#include "../../gameobject/Entity.h"
#include "../../gameobject/ability/ItemCastable.h"
#include "../../../ai/evt/EventTriggerManager.h"

namespace gideon { namespace zoneserver {

CastableElementItem::CastableElementItem(go::Entity& caster, ObjectId itemId, DataCode itemCode,
    const datatable::ItemActiveSkillTemplate& itemSkillTemplate,
    const datatable::SkillEffectTemplate& effectTemplate) :
    CastableItem(caster, itemId, itemCode, itemSkillTemplate, effectTemplate)
{

}


CastableElementItem::~CastableElementItem()
{
}


ErrorCode CastableElementItem::checkItem()
{
    go::ItemCastable* itemCastable = getOwner().queryItemCastable();
    if (itemCastable) {
        if (! itemCastable->checkElementItem(getObjectId())) {
            return ecItemNotEnoughUseable;
        }
    }
    return ecOk;
}


ErrorCode CastableElementItem::checkItemScriptUsable()
{
    //for (const datatable::BaseEffectInfo& info : elementItemTemplate_->getBaseEffectInfos()) {
    //    if (info.scriptType_ == estInvenExtendFreeSlot || info.scriptType_ == estInvenExtendCashSlot) {
    //        go::Inventoryable* inventoryable = getOwner().queryInventoryable();
    //        if (inventoryable) {
    //            if (! inventoryable->canExtendSlot(info.scriptType_ == estInvenExtendCashSlot ? true : false,
    //                uint8_t(info.effectValue_))) {
    //                return ecItemNotExtendIventorySlot;
    //            }
    //        }
    //    }        
    //}  
    return ecOk;
}


void CastableElementItem::useItem()
{
    go::ItemCastable* itemCastable = getOwner().queryItemCastable();
    if (itemCastable) {
        itemCastable->useElementItem(getObjectId());

        EVT_MANAGER.trigger(ai::evt::evtItemUsed, getOwner(), getItemCode());
    }
}

}} // namespace gideon { namespace zoneserver {
