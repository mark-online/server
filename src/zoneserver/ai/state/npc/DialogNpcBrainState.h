#pragma once

#include "NpcBrainStateDef.h"
#include "../BrainState.h"
#include "../../Brain.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../controller/MoveController.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class DialogNpcBrainState
 * 대화 상태
 */
class DialogNpcBrainState : public BrainState
{
private:
    virtual void entry(Brain& brain) {
        assert(brain.getOwner().queryThinkable() != nullptr);

        go::Npc& npc = static_cast<go::Npc&>(brain.getOwner());
        npc.getMoveController().stop();
    }
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
