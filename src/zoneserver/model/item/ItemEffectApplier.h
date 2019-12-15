#pragma once

#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/base/concurrent/Future.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace datatable {
class QuestItemTemplate;
struct QuestItemUseInfo;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {

class Inventory;
class CastingHelper;
class CastingTimer;
class CoolDownTimer;

/***
 * @class ItemEffectApplier
 ***/
class ItemEffectApplier
{
public:
    ItemEffectApplier(go::Player& owner, CastingTimer& castingTimer,
        CoolDownTimer& cooldDownTimer);

    ErrorCode castToTargetQuestItem(ObjectId itemId, const GameObjectInfo& info);
    ErrorCode castToAreaQuestItem(ObjectId itemId, const Position& position);
    ErrorCode castingCompleted(go::Entity* target, const Position& targetPosition,
        float32_t distanceToTargetSq);
private:
    ErrorCode startCasting(go::Entity* target, const datatable::QuestItemTemplate* questItemTemplate,
        const Position& targetPosition, float32_t distanceToTargetSq);
    void casted(go::Entity* target, const Position& targetPosition,
        const datatable::QuestItemTemplate* questItemTemplate);

    void cancelCastingUseItem();

private:
    ErrorCode startCastingElementItem(ObjectId elementItemId);

private:
    ErrorCode checkCastingQuestItem(const datatable::QuestItemTemplate*& questItemTemplate,
        ObjectId questItemId) const;

private:
    boost::scoped_ptr<CastingHelper> castingHelper_;
    go::Player& caster_;
    CastingTimer& castingTimer_;
    CoolDownTimer& cooldDownTimer_;
    sne::base::Future::WeakRef castingTask_;
    sne::base::Future::WeakRef activeTask_;

};


}} // namespace gideon { namespace zoneserver {