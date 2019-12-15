#include "ZoneServerPCH.h"
#include "SkillController.h"
#include "EntityController.h"
#include "PlayerController.h"
#include "../model/state/CreatureState.h"
#include "../model/state/SkillCasterState.h"
#include "../model/gameobject/skilleffect/CreatureEffectScriptApplier.h"
#include "../model/gameobject/Creature.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/ability/SkillCastable.h"
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {


// = SkillController

SkillController::SkillController(go::Entity* owner) :
	Controller(owner),
	skillCasterState_(*owner->querySkillCasterState()),
	creatureState_(*owner->queryCreatureState())
{
}


}}} // namespace gideon { namespace zoneserver { namespace gc {