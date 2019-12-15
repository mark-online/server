#pragma once

#include "../BrainState.h"
#include "../../NpcStateBrain.h"
#include "../../event/npc/NpcBrainEventDef.h"
#include "../../event/BrainEvent.h"
#include "../../aggro/AggroList.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/Thinkable.h"
#include "../../../model/gameobject/ability/Moveable.h"
#include "../../../model/gameobject/ability/Formable.h"
#include "../../../model/gameobject/ability/Invadable.h"
#include "../../../model/state/CreatureState.h"
#include "../../../controller/NpcController.h"
#include "../../../service/formation/NpcFormation.h"
#include "../../../service/movement/MovementManager.h"
#include "../../../service/time/GameTimer.h"
#include "../../../service/skill/NpcSkillList.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class ActiveNpcBrainState
 * 본격적으로 활동하는 상태
 */
class ActiveNpcBrainState : public BrainState
{
    enum {
        checkMovementInterval = 2 * 1000
    };

private:
    virtual void entry(Brain& brain) {
        owner_ = static_cast<go::Npc*>(&brain.getOwner());
        movementManager_ = &owner_->queryMoveable()->getMovementManager();

        owner_->setPeaceState();
        owner_->queryTargetSelectable()->unselectTarget();

        movementManager_->reset();
        checkMovementTracker_.reset(0);

        skillList_ = &owner_->getSkillList();

        canUpdate_ = true;
    }

    //virtual void exit(Brain& brain) {
    //}

    virtual void update(Brain& brain, msec_t diff) {
        if (! canUpdate_) {
            return;
        }

        checkMovementTracker_.update(diff);
        if (checkMovementTracker_.isPassed()) {
            if (movementManager_->isCompleted()) {
                if (canMove()) {
                    if (shouldMove()) {
                        setMovement();
                    }
                }
            }
            checkMovementTracker_.reset(checkMovementInterval);
        }

        if (brain.getAggroList()->isThreated()) {
            ai::Stateable* stateable = brain.queryStateable(); assert(stateable != nullptr);
            stateable->asyncHandleEvent(eventThreated);
            canUpdate_ = false;
            return;
        }

        castBuffSkill();

        NpcStateBrain& npcBrain = static_cast<NpcStateBrain&>(brain);
        (void)npcBrain.watchMovement(diff);
        (void)npcBrain.watchPlayerWorldEventRegionJoinable(diff);
        (void)npcBrain.talkTo(diff);
    }

private:
    void setMovement() {
        go::Formable* formable = owner_->queryFormable();
        if (formable != nullptr) {
            NpcFormationRefPtr formation = formable->getFormation();
            if ((formation.get() != nullptr) && formation->shouldFollowLeader(*owner_)) {
                movementManager_->setIdleMovement();
                return;
            }
        }

        // FYI: 침략군은 행군이 끝나면(마지막 노드에 도착하면) 배회를 한다
        if (owner_->hasPath()) {
            go::Marchable* marchable = owner_->queryMarchable();
            if ((marchable != nullptr) && marchable->shouldMarch()) {
                movementManager_->setMarchMovement();
                return;
            }
            else {
                movementManager_->setPathMovement();
                return;
            }
        }

        movementManager_->setWanderMovement();
    }

    void castBuffSkill() {
        const SkillCode skillCode = skillList_->getCastableBuffSkillCode();
        if (isValidSkillCode(skillCode)) {
            go::SkillCastable* castable = owner_->querySkillCastable();
            assert(castable != nullptr);
            (void)castable->castTo(owner_->getGameObjectInfo(), skillCode);
        }
    }

    bool canMove() const {
        CreatureState* state = owner_->queryCreatureState();
        if (state && state->isSpawnProtection()) {
            return false;
        }

        assert(owner_->queryMoveable() != nullptr);
        return true;
    }

    bool shouldMove() const {
        go::Thinkable* thinkable = owner_->queryThinkable();
        if (! thinkable->hasWalkRoutes()) {
            return false;
        }

        return true;
    }

private:
    // = BrainState overriding
    virtual bool handleEvent(Brain& brain, sne::core::fsm::Event& event) const {
        BrainEvent& brainEvent = static_cast<BrainEvent&>(event);
        const int eventId = brainEvent.getEventId();
        switch (eventId) {
            case eventAttacked:
            case eventAggressive:
            case eventThreated:
            case eventDialogRequested:
            case eventFeared:
            case eventSeekAssist:
            return brainEvent.handle(brain);
        }
        return true; // 다른 이벤트는 처리하지 않는다
    }

private:
    go::Npc* owner_;
    MovementManager* movementManager_;
    GameTimeTracker checkMovementTracker_;
    const NpcSkillList* skillList_;
    bool canUpdate_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
