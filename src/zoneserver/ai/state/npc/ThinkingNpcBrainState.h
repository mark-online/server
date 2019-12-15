#pragma once

#include "../BrainState.h"
#include "NpcBrainStateDef.h"
#include "../../NpcStateBrain.h"
#include "../../event/npc/NpcBrainEventDef.h"
#include "../../aggro/AggroList.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/status/CreatureStatus.h"
#include "../../../model/state/MoveState.h"
#include <gideon/cs/datatable/NpcTable.h>


namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class ThinkingNpcBrainState
 * 상황 판단이 필요한 상태
 */
class ThinkingNpcBrainState : public BrainState
{
private:
    virtual void entry(Brain& brain) {
        assert(brain.getOwner().queryThinkable() != nullptr);

        go::Npc* owner = static_cast<go::Npc*>(&brain.getOwner());
        if (owner->queryThinkable()->hasWalkRoutes()) {
            const datatable::NpcTemplate& npcTemplate = owner->getNpcTemplate();
            maxEvadingDistance_ = npcTemplate.getMaxMoveDistance() * 0.4f;
        }
        else {
            maxEvadingDistance_ = 0.0f;
        }
    }

    virtual void update(Brain& brain, msec_t /*diff*/) {
        const msec_t watchMovementDiff = 100 * 1000; // 무조건 실행 가능하게 그냥 충분히 큰 값
        NpcStateBrain& npcBrain = static_cast<NpcStateBrain&>(brain);
        (void)npcBrain.watchMovement(watchMovementDiff);

        go::Npc& npc = static_cast<go::Npc&>(brain.getOwner());
        ai::Stateable* stateable = brain.queryStateable(); assert(stateable != nullptr);
        if (brain.getAggroList()->isThreated()) {
            stateable->setNextState(stateAttacking);
            return;
        }

        if (npc.getNpcTemplate().getWalkSpeed() > 0.0f) {
            if (! npc.isAtSpawnLocation(maxEvadingDistance_)) {
			    stateable->setNextState(stateEvading);
			    return;
            }
        }

        if (! npc.getCreatureStatus().isHpFullyRestored()) {
            stateable->setNextState(stateResting);
            return;
        }

        stateable->setNextState(stateActive);
    }

private:
    float32_t maxEvadingDistance_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
