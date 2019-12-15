#pragma once

#include "../BrainState.h"
#include "../../event/npc/NpcBrainEventDef.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../controller/MoveController.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class IdleNpcBrainState
 * 아무 것도 하지 않는 상태 (초기 상태)
 */
class IdleNpcBrainState : public BrainState
{
private:
    virtual void entry(Brain& brain) {
        go::Npc& npc = static_cast<go::Npc&>(brain.getOwner());

        npc.setPeaceState();
        npc.queryTargetSelectable()->unselectTarget();
        npc.getMoveController().stop();

        // FYI: aggro를 초기화하면 안된다!
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
