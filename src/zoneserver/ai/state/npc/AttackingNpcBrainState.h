#pragma once

#include "../BrainState.h"
#include "../../NpcStateBrain.h"
#include "../../event/npc/NpcBrainEventDef.h"
#include "../../aggro/AggroList.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/Thinkable.h"
#include "../../../model/gameobject/ability/Moveable.h"
#include "../../../model/gameobject/ability/Castable.h"
#include "../../../model/gameobject/ability/Formable.h"
#include "../../../model/gameobject/ability/Summonable.h"
#include "../../../model/state/MoveState.h"
#include "../../../controller/NpcController.h"
#include "../../../controller/npc/NpcMoveController.h"
#include "../../../service/movement/MovementManager.h"
#include "../../../service/skill/NpcSkillList.h"
#include "../../../service/formation/NpcFormation.h"
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class AttackingNpcBrainState
 * 공격 상태
 */
class AttackingNpcBrainState : public BrainState
{
public:
    AttackingNpcBrainState() {}

private:
    virtual void entry(Brain& brain) {
        owner_ = static_cast<go::Npc*>(&brain.getOwner());
        owner_->setCombatState();

        go::Moveable* moveable = owner_->queryMoveable(); assert(moveable != nullptr);
        movementManager_ = &moveable->getMovementManager();
        maxMoveDistanceSq_ = square(moveable->getMaxMoveDistance());
        attackedPosition_ = owner_->getPosition();
        currentTarget_ = nullptr;
        skillList_ = &owner_->getSkillList();
        currentSkillCode_ = invalidSkillCode;
        canUpdate_ = true;

        const datatable::NpcTemplate& npcTemplate = owner_->getNpcTemplate();
        if (npcTemplate.getWalkSpeed() > 0.0f) {
            movementManager_->setChaseMovement();
        }
    }

    virtual void exit(Brain& /*brain*/) {
        // FYI: 어그로를 초기화하면 안된다
        //brain.getAggroList()->clear();
    }

    virtual void update(Brain& brain, msec_t diff) {
        if (! canUpdate_) {
            return;
        }

        if (isTired(brain)) {
            tiredToAttack(brain);
            return;
        }

        (void)static_cast<NpcStateBrain&>(brain).talkTo(diff);

        attackTarget();

        NpcStateBrain& npcBrain = static_cast<NpcStateBrain&>(brain);
        (void)npcBrain.watchPlayerWorldEventRegionJoinable(diff);
    }

private:
    bool selectTarget(Brain& brain) {
        ai::AggroList* aggroList = brain.getAggroList();
        aggroList->update(attackedPosition_, maxMoveDistanceSq_);
        if (! aggroList->isThreated()) {
            return false;
        }

        CreatureState* state = owner_->queryCreatureState();
        if (! state->canSelectTarget()) {
            return true;
        }

        go::Entity* target = aggroList->selectVictim(*owner_, currentTarget_);
        if (target != nullptr) {
            owner_->queryTargetSelectable()->selectTarget(target->getGameObjectInfo());
        }
        else {
            owner_->queryTargetSelectable()->unselectTarget();
        }
        currentTarget_ = target;
        return true;
    }

    void attackTarget() {
        go::Entity* target = owner_->queryTargetSelectable()->getSelectedTarget();
        if ((! target) || (! target->isValid())) {
            return;
        }

        go::Castable* castable = owner_->queryCastable();
        if (castable->isCasting()) { // 시전 중이고 대상이 범위를 벗어나면 시전 취소
            if (skillList_->shouldStopDuringCasting(currentSkillCode_)) {
                const float32_t toleranceFactor = 1.5f; // TODO: 얼마가 좋을라나?
                if (owner_->getSquaredLength(target->getPosition()) >
                    square(skillList_->getMaxDistance(currentSkillCode_) * toleranceFactor)) {
                    castable->cancelCasting();
                    currentSkillCode_ = invalidSkillCode;
                }
            }
        }
        else {
            TargetingType targetingType = ttTarget;
            const SkillCode skillCode = skillList_->getAttackableSkillCode(targetingType, *target);
            if (isValidSkillCode(skillCode)) {
                const ErrorCode errorCode = castSkill(skillCode, targetingType, *target);
                if (isSucceeded(errorCode)) {
                    currentSkillCode_ = skillCode;
                }
            }
        }
    }

    ErrorCode castSkill(SkillCode skillCode, TargetingType targetingType, go::Entity& target) {
        stopToCast(skillCode);

        go::SkillCastable* castable = owner_->querySkillCastable();
        assert(castable != nullptr);
        if (ttSelf == targetingType || ttSelfArea == targetingType) {
            return castable->castTo(owner_->getGameObjectInfo(), skillCode);
        }
        else if (ttTarget == targetingType || ttTargetArea == targetingType) {
            return castable->castTo(target.getGameObjectInfo(), skillCode);
        }
        else if (ttArea == targetingType ) {
            return castable->castAt(target.getPosition(), skillCode);
        }
        else {
            assert(false);
        }
        return ecServerInternalError;
    }

    void stopToCast(SkillCode skillCode) {
        if (skillList_->shouldStopDuringCasting(skillCode)) {
            owner_->getMoveController().stop();
        }
    }

    void tiredToAttack(Brain& brain) {
        if (owner_->queryMoveState()->isRooted()) {
            return;
        }

        brain.queryStateable()->handleEvent(eventTired);
        canUpdate_ = false;

        go::Formable* formable = owner_->queryFormable(); assert(formable != nullptr);
        NpcFormationRefPtr formation = formable->getFormation();
        if (formation.get() != nullptr) {
            formation->memberTiredToAttack(*owner_);
        }
        else {
            go::Summonable* summonable = owner_->querySummonable();
            if (summonable != nullptr) {
                go::Knowable* knowable = owner_->queryKnowable(); assert(knowable != nullptr);
                for (const GameObjectInfo& summonInfo : summonable->getSummons()) {
                    go::Entity* summon = knowable->getEntity(summonInfo);
                    if (summon != NULL) {
                        go::Thinkable* thinkable = summon->queryThinkable();
                        if (thinkable != nullptr) {
                            ai::Stateable* stateable = thinkable->getBrain().queryStateable();
                            if (stateable != nullptr) {
                                stateable->asyncHandleEvent(eventTired);
                            }
                        }
                    }
                }
            }
        }
    }

    bool isTired(Brain& brain) {
        if (! selectTarget(brain)) {
            return true;
        }
        return false;
    }

private:
    go::Npc* owner_;
    MovementManager* movementManager_;
    float32_t maxMoveDistanceSq_;
    Position attackedPosition_; //< 공격 받은 위치
    go::Entity* currentTarget_;
    const NpcSkillList* skillList_;
    SkillCode currentSkillCode_;
    bool canUpdate_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
