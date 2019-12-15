#pragma once

#include "../../helper/CastEventCallback.h"
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/datatable/EffectApplyInfo.h>
#include <sne/base/concurrent/Future.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon { namespace datatable {
struct ItemActiveSkillTemplate;
struct SkillEffectTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {

class Inventory;
class CastChecker;

/***
 * @class CastableItem
 * TODO: 1:1 필드 대전할때 체크 검사 넣어야함
 ***/
class CastableItem
{
public:
    CastableItem(go::Entity& caster, ObjectId itemId, DataCode itemCode,
        const datatable::ItemActiveSkillTemplate& itemSkillTemplate,
        const datatable::SkillEffectTemplate& effectTemplate);
    virtual ~CastableItem();

    ErrorCode castTo(const GameObjectInfo& targetInfo);
    ErrorCode castAt(const Position& position);
    ErrorCode castingCompleted(go::Entity* target,
        Position targetPosition, float32_t distanceToTargetSq);

    void cancel();

private:
    ErrorCode startCasting(go::Entity* target,
        const Position& targetPosition, float32_t distanceToTargetSq);
    void casted(go::Entity* target, const Position& targetPosition, EffectDefenceType effectDefenceType);
    void areaEffected(const Position& center);

    void setCooldown();
    void cancelCooldown();
    void releaseHide();
protected:
    go::Entity& getOwner() {
        return caster_;
    }

    const go::Entity& getOwner() const {
        return caster_;
    }

    ObjectId getObjectId() const {
        return itemId_;
    }

    DataCode getItemCode() const {
        return itemCode_;
    }
    
    const datatable::ItemActiveSkillTemplate& getItemActiveSkillTemplate() const {
        return itemSkillTemplate_;
    }

    const datatable::SkillEffectTemplate& getSkillEffectTemplate() const {
        return effectTemplate_;
    }

	ErrorCode checkCooltime() const;

private:
    virtual ErrorCode checkItemScriptUsable() { return ecOk; }
    virtual ErrorCode checkItem() = 0;
    virtual void useItem() = 0;

private:
    ErrorCode getTarget(go::Entity*& target, const GameObjectInfo& targetInfo) const;
    bool shouldCheckByArena() const;
    
private:
    go::Entity& caster_;
    ObjectId itemId_;
    DataCode itemCode_;
    const datatable::ItemActiveSkillTemplate& itemSkillTemplate_;
    const datatable::SkillEffectTemplate& effectTemplate_;
    sne::base::Future::WeakRef castingTask_;
    sne::base::Future::WeakRef activeTask_;

    float32_t maxDistance_;
    float32_t minDistance_;

};


}} // namespace gideon { namespace zoneserver {