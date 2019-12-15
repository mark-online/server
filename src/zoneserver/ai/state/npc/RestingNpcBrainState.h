#pragma once

#include "../BrainState.h"
#include "../../Brain.h"
#include "../../event/npc/NpcBrainEventDef.h"
#include "../../aggro/AggroList.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../controller/EntityController.h"
#include "../../../controller/callback/EntityStatusCallback.h"
#include "../../../model/gameobject/status/CreatureStatus.h"
#include "../../../service/time/GameTimer.h"
#include <esut/Random.h>
#include "sne/core/memory/MemoryPoolMixin.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class RestingNpcBrainState
 * 휴식 상태
 */
class RestingNpcBrainState : public BrainState
{
private:
    virtual void entry(Brain& brain) {
        brain.getAggroList()->clear();

        npc_ = &static_cast<go::Npc&>(brain.getOwner());
        npc_->setPeaceState();
        npc_->queryTargetSelectable()->unselectTarget();

        go::Liveable* liveable = npc_->queryLiveable();
        if (liveable != nullptr) {
            liveable->getCreatureStatus().restorePoints();
        }

        // TODO: 차후에 C/S간 이동 동기화가 어느 정도 맞을 경우 방향 전환 시간을 줄여야 한다
        const msec_t delayTime = shouldChangeHead() ? 4000 : esut::random(1000, 5000);
        restingTimer_.reset(delayTime);
    }

    virtual void update(Brain& brain, msec_t diff) {
        restingTimer_.update(diff);
        if (! restingTimer_.isPassed()) {
            return;
        }

        if (shouldChangeHead()) {
            npc_->getMoveController().turn(npc_->getHomePosition().heading_);
        }

        brain.queryStateable()->handleEvent(eventActive);
    }

private:
    bool shouldChangeHead() const {
        return npc_->isNpc() && (! npc_->queryMoveable()->isMoving());
    }

private:
    go::Npc* npc_;
    GameTimeTracker restingTimer_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
