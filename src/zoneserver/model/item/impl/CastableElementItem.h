#pragma once

#include "../CastableItem.h"


namespace gideon { namespace zoneserver {

/***
 * @class CastableElementItem
 ***/
class CastableElementItem : public CastableItem
{
public:
    CastableElementItem(go::Entity& caster, ObjectId itemId, DataCode itemCode,
        const datatable::ItemActiveSkillTemplate& itemSkillTemplate,
        const datatable::SkillEffectTemplate& effectTemplate);
    virtual ~CastableElementItem();
private:

    virtual ErrorCode checkItem();
    virtual ErrorCode checkItemScriptUsable();
    virtual void useItem();

};


}} // namespace gideon { namespace zoneserver {