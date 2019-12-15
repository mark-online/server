#pragma once

#include "../BrainState.h"
#include "../../NpcStateBrain.h"
#include "../../aggro/AggroList.h"
#include "../../../model/state/CombatState.h"
#include "../../../model/gameobject/ability/Factionable.h"
#include "../../../service/movement/MovementManager.h"
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver { namespace ai {

namespace {

/**
 * @class EnumerateCommandForAssist
 * 전투 상태가 아니고 가장 가까운 친구 검색
 */
class EnumerateCommandForAssist : public go::Knowable::EnumerateCommand
{
public:
    EnumerateCommandForAssist(const go::Npc& caller,
        float32_t assistableDistance) :
        target_(nullptr),
        caller_(caller),
        callerPosition_(caller.getPosition()),
        assistableDistanceSq_(assistableDistance * assistableDistance) {}

public:
    go::Entity* getTarget() {
        return target_;
    }

private:
    virtual bool execute(go::Entity& entity) {
        if (entity.isSame(caller_)) {
            return false;
        }

        CombatState* combatState = entity.queryCombatState();
        if ((! combatState) || combatState->isCombating()) {
            return false;
        }

        go::Factionable* callerFactionable = const_cast<go::Npc&>(caller_).queryFactionable();
        if (! callerFactionable->isFriendlyTo(entity)) {
            return false;
        }

        const float32_t distanceSq = entity.getSquaredLength(callerPosition_);
        if (distanceSq > assistableDistanceSq_) {
            return false;
        }

        assert(entity.queryThinkable() != nullptr);

        assistableDistanceSq_ = distanceSq;
        target_ = &entity;
        return true;
    }

private:
    go::Entity* target_;
    const go::Npc& caller_;
    const Position callerPosition_;
    float32_t assistableDistanceSq_;
};

} // namespace {


/**
 * @class SeekingAssistNpcBrainState
 * 도와줄 친구 찾아 강남 가는 상태
 */
class SeekingAssistNpcBrainState : public BrainState
{
private:
    virtual void entry(Brain& brain) {
        movementManager_ = nullptr;

        assist_ = searchAssist(brain);
        if (assist_ != nullptr) {
            //brain.getAggroList()->clear(); // FYI: aggro 초기화하면 안됨!
            go::Npc& owner = static_cast<go::Npc&>(brain.getOwner());
            owner.queryTargetSelectable()->unselectTarget(); // FYI: 클라이언트에게 알리기 위함!
            movementManager_ = &owner.queryMoveable()->getMovementManager();
            movementManager_->setSeekAssistMovement(*assist_);

            contactableDistanceSq_ =
                square(owner.getNpcTemplate().getAiLinkReactiveDistance());

            canUpdate_ = true;
        }
        else {
            brain.queryStateable()->asyncHandleEvent(eventFeared);
            canUpdate_ = false;
        }
    }

    virtual void update(Brain& brain, msec_t /*diff*/) {
        if (! canUpdate_) {
            return;
        }

        if (assist_ != nullptr) {
            if (brain.getOwner().getSquaredLength(assist_->getPosition()) <=
                contactableDistanceSq_) {
                static_cast<NpcStateBrain&>(brain).callAssist(*assist_);
                brain.queryStateable()->asyncHandleEvent(eventThreated);
                canUpdate_ = false;
            }
        }
    }

private:
    go::Entity* searchAssist(Brain& brain) {
        go::Npc& owner = static_cast<go::Npc&>(brain.getOwner());

        const datatable::NpcTemplate& npcTemplate = owner.getNpcTemplate();
        if (! npcTemplate.canCallLink()) {
            return nullptr;
        }

        if (! canStrikeBack(npcTemplate.getReactiveType())) {
            return nullptr;
        }

        if (! brain.getAggroList()->isThreated()) {
            return nullptr;
        }

        const float32_t assistableDistance = 30.0f; // TODO: table에서 읽어올 것
        EnumerateCommandForAssist enumerateCommand(owner, assistableDistance);
        (void)owner.queryKnowable()->enumerate(enumerateCommand);
        return enumerateCommand.getTarget();
    }

private:
    // = BrainState overriding
    virtual bool handleEvent(Brain& brain, sne::core::fsm::Event& event) const {
        BrainEvent& brainEvent = static_cast<BrainEvent&>(event);
        const int eventId = brainEvent.getEventId();
        switch (eventId) {
        case eventThreated:
        case eventFeared:
            return brainEvent.handle(brain);
        }
        return true; // 다른 이벤트는 처리하지 않는다
    }

private:
    MovementManager* movementManager_;
    go::Entity* assist_;
    float32_t contactableDistanceSq_;
    bool canUpdate_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
