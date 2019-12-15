#pragma once

#include "../CastableItem.h"
#include <gideon/cs/shared/data/ItemEffectInfo.h>


namespace gideon { namespace zoneserver {

/***
 * @class CastableQuestItem
 ***/
class CastableQuestItem : public CastableItem
{
public:
    CastableQuestItem(go::Entity& caster, ObjectId itemId, DataCode itemCode,
        const datatable::ItemActiveSkillTemplate& itemSkillTemplate,
        const datatable::SkillEffectTemplate& effectTemplate);
    virtual ~CastableQuestItem();
private:

    virtual ErrorCode checkItem();
    virtual void useItem();
};


}} // namespace gideon { namespace zoneserver {