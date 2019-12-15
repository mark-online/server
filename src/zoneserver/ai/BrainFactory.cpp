#include "ZoneServerPCH.h"
#include "BrainFactory.h"
#include "NpcStateBrain.h"
#include "BuildingStateBrain.h"
#include "scripting/GameScriptManager.h"
#include "evt/EventTrigger.h"
#include "../model/gameobject/Npc.h"

namespace gideon { namespace zoneserver { namespace ai {

std::unique_ptr<Brain> BrainFactory::createBrain(go::Entity& owner)
{
    if (owner.isNpcOrMonster()) {
        auto brain = GAME_SCRIPT_MANAGER.createBrain(static_cast<go::Npc&>(owner));
        if (brain) {
            return brain;
        }
        auto eventTrigger = std::make_unique<evt::EventTrigger>(owner);
        return std::make_unique<ai::NpcStateBrain>(owner, std::move(eventTrigger));
    }
    else if (owner.isBuilding()) {
        return std::make_unique<ai::BuildingStateBrain>(owner);
    }

    assert(false);
    return nullptr;
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
