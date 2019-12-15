#pragma once

#include <gideon/Common.h>

namespace gideon { namespace zoneserver { namespace go {

class Castable;
class Positionable;
class Knowable;
class Moveable;
class Liveable;
class VehicleMountable;
class GliderMountable;
class SkillCastable;
class ItemCastable;
class Rewardable;
class Thinkable;
class CreatureStatusable;
class PassiveSkillCastable;
class CreatureStateable;
class Inventoryable;
class Questable;
class Partyable;
class Buildable;
class Skillable;
class Chaoable;
class GraveStoneable;
class PublicPartyable;
class Dialogable;
class CastGameTimeable;
class CastCheckable;
class ArenaMatchable;
class StaticObjectOwnerable;
class OutsideInventoryable;
class Factionable;
class Tickable;
class CastNotificationable;
class Formable;
class Moneyable;
class Guildable;
class Protectionable;
class Networkable;
class Cheatable;
class Gliderable;
class Vehicleable;
class Teleportable;
class AggroSendable;
class CombatStateable;
class Storeable;
class SkillLearnable;
class CharacterClassable;
class SOInventoryable;
class FieldDuelable;
class BuildingProductionInventoryable;
class Bankable;
class BuildingGuardable;
class Marchable;
class WorldEventable;
class Invadable;
class Summonable;
class Craftable;
class TargetSelectable;
class WeaponUseable;
class CharacterTitleable;
class Achievementable;

/**
 * @class EntityAbility
 * Entity의 능력
 * - http://en.wikibooks.org/w/index.php?title=More_C%2B%2B_Idioms/Capability_Query
 * - TODO: 깔끔하게 개선할 것(COM QueryInterface() 처럼)
 */
class EntityAbility
{
public:
    virtual ~EntityAbility() {}

public:
    virtual Positionable* queryPositionable() {
        return nullptr;
    }

    virtual Skillable* querySkillable() {
        return nullptr;
    }

    virtual Knowable* queryKnowable() {
        return nullptr;
    }

    virtual Moveable* queryMoveable() {
        return nullptr;
    }

    virtual Liveable* queryLiveable() {
        return nullptr;
    }

	virtual Networkable* queryNetworkable() {
		return nullptr;
	}

	virtual Cheatable* queryCheatable() {
		return nullptr;
	}

    virtual VehicleMountable* queryVehicleMountable() {
        return nullptr;
    }

    virtual GliderMountable* queryGliderMountable() {
        return nullptr;
    }

    virtual SkillCastable* querySkillCastable() {
        return nullptr;
    }

	virtual Guildable* queryGuildable() {
		return nullptr;
	}

    virtual ItemCastable* queryItemCastable() {
        return nullptr;
    }

	virtual Moneyable* queryMoneyable() {
		return nullptr;
	}

    virtual Rewardable* queryRewardable() {
        return nullptr;
    }

    virtual Thinkable* queryThinkable() {
        return nullptr;
    }

    virtual const CreatureStatusable* queryCreatureStatusable() const {
        return nullptr;
    }

    virtual PassiveSkillCastable* queryPassiveSkillCastable() { 
        return nullptr; 
    }

    virtual Inventoryable* queryInventoryable() {
        return nullptr;
    }

	virtual Questable* queryQuestable() {
		return nullptr;
	}

    virtual const Questable* queryQuestable() const {
        return nullptr;
    }

    virtual Partyable* queryPartyable() {
        return nullptr;
    }

    virtual Buildable* queryBuildable() {
        return nullptr;
    }

    virtual Chaoable* queryChaoable() {
        return nullptr;
    }

    virtual const Chaoable* queryChaoable() const {
        return nullptr;
    }

    virtual GraveStoneable* queryGraveStoneable() {
        return nullptr;
    }

    virtual PublicPartyable* queryPublicPartyable() {
        return nullptr;
    }

    virtual Dialogable* queryDialogable() {
        return nullptr;
    }

    virtual Castable* queryCastable() {
        return nullptr;
    }

    virtual CastGameTimeable* queryCastGameTimeable() {
        return nullptr;
    }

    virtual CastCheckable* queryCastCheckable() {
        return nullptr;
    }

	virtual Protectionable* queryProtectionable() {
		return nullptr;
	}

    virtual const Protectionable* queryProtectionable() const {
        return nullptr;
    }

    virtual ArenaMatchable* queryArenaMatchable() {
        return nullptr;
    }

    virtual StaticObjectOwnerable* queryStaticObjectOwnerable() {
        return nullptr;
    }

    virtual const StaticObjectOwnerable* queryStaticObjectOwnerable() const {
        return nullptr;
    }

    virtual OutsideInventoryable* queryOutsideInventoryable() {
        return nullptr;
    }

    virtual Factionable* queryFactionable() {
        return nullptr;
    }

    virtual const Factionable* queryFactionable() const {
        return nullptr;
    }

    virtual Tickable* queryTickable() {
        return nullptr;
    }

	virtual CastNotificationable* queryCastNotificationable() {
		return nullptr;
	}

    virtual Formable* queryFormable() {
        return nullptr;
    }

	virtual Gliderable* queryGliderable() {
		return nullptr;
	}

	virtual Vehicleable* queryVehicleable() {
		return nullptr;
	}

	virtual Teleportable* queryTeleportable() {
		return nullptr;
	}

	virtual AggroSendable* queryAggroSendable() {
		return nullptr;
	}

	virtual CombatStateable* queryCombatStateable() {
		return nullptr;
	}

	virtual Storeable* queryStoreable() {
		return nullptr;
	}

	virtual SkillLearnable* querySkillLearnable() {
		return nullptr;
	}

	virtual CharacterClassable* queryCharacterClassable() {
		return nullptr;
	}

	virtual SOInventoryable* querySOInventoryable() {
		return nullptr;
	}

    virtual FieldDuelable* queryFieldDuelable() {
        return nullptr;
    }

    virtual BuildingProductionInventoryable* queryBuildingProductionInventoryable() { 
        return nullptr; 
    }

    virtual Bankable* queryBankable() {
        return nullptr;
    }

    virtual BuildingGuardable* queryBuildingGuardable() {
        return nullptr;
    }

    virtual Marchable* queryMarchable() {
        return nullptr;
    }

    virtual WorldEventable* queryWorldEventable() { 
        return nullptr; 
    }

    virtual Invadable* queryInvadable() {
        return nullptr;
    }

    virtual Summonable* querySummonable() {
        return nullptr;
    }

    virtual Craftable* queryCraftable() {
        return nullptr;
    }

    virtual const Craftable* queryCraftable() const {
        return nullptr;
    }

    virtual TargetSelectable* queryTargetSelectable() {
        return nullptr;
    }

    virtual const WeaponUseable* queryWeaponUseable() const {
        return nullptr;
    }

    virtual CharacterTitleable* queryCharacterTitleable() {
        return nullptr;
    }
    virtual Achievementable* queryAchievementable() {
        return nullptr;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace go {
