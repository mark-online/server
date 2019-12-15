#pragma once

#include <gideon/Common.h>

namespace gideon { namespace zoneserver { namespace gc {

class InterestAreaCallback;
class MovementCallback;
class CreatureEffectCallback;
class StaticObjectEffectCallback;
class EffectCallback;
class VehicleCallback;
class GliderCallback;
class ChatCallback;
class EntityStatusCallback;
class DeathCallback;
class RewardCallback;
class GrowthCallback;
class CheatCallback;
class StatsCallback;
class LifeStatusCallback;
class TradeCallback;
class InventoryCallback;
class EquipInventoryCallback;
class AccessoryInventoryCallback;
class QuestCallback;
class HarvestCallback;
class PartyCallback;
class AnchorCallback;
class BuildCallback;
class QuestInventoryCallback;
class AnchorAbillityCallback;
class SkillCallback;
class ChaoCallback;
class BotCommandCallback;
class EmotionCallback;
class GuildCallback;
class PublicPartyCallback;
class DialogCallback;
class EntityActionCallback;
class CastCallback;
class PassiveSkillCallback;
class MailCallback;
class ArenaCallback;
class BuildingStateCallback;
class CooldownCallback;
class DeviceCallback;
class CombatStateCallback;
class TeleportCallback;
class BuildingTeleportCallback;
class GraveStoneCallback;
class NpcTalkingCallback;
class FieldDuelCallback;
class SelectProductionCallback;
class BuildingProductionTaskCallback;
class WorldEventCallback;
class OutsideInventoryCallback;
class PassiveEffectCallback;
class CraftCallback;
class AchievementCallback;
class CharacterTitleCallback;
class ItemCallback;

/**
 * @class EntityControllerAbility
 * - http://en.wikibooks.org/w/index.php?title=More_C%2B%2B_Idioms/Capability_Query
 */
class EntityControllerAbility
{
public:
    virtual ~EntityControllerAbility() {}

public:
    virtual InterestAreaCallback* queryAppearanceCallback() {
        return nullptr;
    }

    virtual MovementCallback* queryMovementCallback() {
        return nullptr;
    }

    virtual SkillCallback* querySkillCallback() {
        return nullptr;
    }

    virtual EffectCallback* queryEffectCallback() {
        return nullptr;
    }

    virtual PassiveEffectCallback* queryPassiveEffectCallback() {
        return nullptr;
    }

    virtual CreatureEffectCallback* queryCreatureEffectCallback() {
        return nullptr;
    }

    virtual StaticObjectEffectCallback* queryStaticObjectEffectCallback() {
        return nullptr;
    }

    virtual VehicleCallback* queryVehicleMountCallback() {
        return nullptr;
    }

    virtual GliderCallback* queryGliderCallback() {
        return nullptr;
    }

    virtual ChatCallback* queryChatCallback() {
        return nullptr;
    }

    virtual EntityStatusCallback* queryEntityStatusCallback() {
        return nullptr;
    }

    virtual DeathCallback* queryDeathCallback() {
        return nullptr;
    }

    virtual RewardCallback* queryRewardCallback() {
        return nullptr;
    }

    virtual GrowthCallback* queryGrowthCallback() {
        return nullptr;
    }
    
    virtual CheatCallback* queryCheatCallback() {
        return nullptr;
    }

    virtual StatsCallback* queryStatsCallback() {
        return nullptr;
    }

    virtual LifeStatusCallback* queryLifeStatusCallback() {
        return nullptr;
    }

	virtual TradeCallback* queryTradeCallback() {
		return nullptr;
	}

	virtual InventoryCallback* queryInventoryCallback() {
		return nullptr;
	}

    virtual EquipInventoryCallback* queryEquipInventoryCallback() {
        return nullptr;
    }

    virtual AccessoryInventoryCallback* queryAccessoryInventoryCallback() {
        return nullptr;
    }

    virtual QuestInventoryCallback* queryQuestInventoryCallback() {
        return nullptr;
    }

	virtual QuestCallback* queryQuestCallback() {
		return nullptr;
	}

    virtual HarvestCallback* queryHarvestCallback() {
        return nullptr;
    }

	
    virtual PartyCallback* queryPartyCallback() {
        return nullptr;
    }

    virtual AnchorCallback* queryAnchorCallback() {
        return nullptr;
    }

    virtual BuildCallback* queryBuildCallback() {
        return nullptr;
    }

    virtual AnchorAbillityCallback* queryAnchorAbillityCallback() {
        return nullptr;
    }

    virtual SkillCallback* queryPlayerSkillCallback() {
        return nullptr;
    }

    virtual ChaoCallback* queryChaoCallback() {
        return nullptr;
    }

    virtual BotCommandCallback* queryBotCommandCallback() {
        return nullptr;
    }

    virtual EmotionCallback* queryEmotionCallback() {
        return nullptr;
    }

    virtual GuildCallback* queryGuildCallback() {
        return nullptr;
    }

    virtual PublicPartyCallback* queryPublicPartyCallback() {
        return nullptr;
    }

    virtual DialogCallback* queryDialogCallback() {
        return nullptr;
    }

    virtual EntityActionCallback* queryEntityActionCallback() {
        return nullptr;
    }

    virtual CastCallback* queryCastCallback() {
        return nullptr;
    }

    virtual PassiveSkillCallback* queryPassiveSkillCallback() {
        return nullptr;
    }

    virtual MailCallback* queryMailCallback() {
        return nullptr;
    }

	virtual ArenaCallback* queryArenaCallback() {
		return nullptr;
	}

    virtual BuildingStateCallback* queryBuildingStateCallback() {
        return nullptr;
    }

    virtual CooldownCallback* queryCooldownCallback() {
        return nullptr;
    }

    virtual DeviceCallback* queryDeviceCallback() {
        return nullptr;
    }

	virtual CombatStateCallback* queryCombatStateCallback() {
		return nullptr;
	}

	virtual TeleportCallback* queryTeleportCallback() {
		return nullptr;
	}

	virtual BuildingTeleportCallback* queryBuildingTeleportCallback() {
		return nullptr;
	}

	virtual GraveStoneCallback* queryGraveStoneCallback() {
		return nullptr;
	}

    virtual NpcTalkingCallback* queryNpcTalkingCallback() {
        return nullptr;
    }

    virtual FieldDuelCallback* queryFieldDuelCallback() {
        return nullptr;
    }

    virtual SelectProductionCallback* querySelectProductionCallback() {
        return nullptr;
    }

    virtual BuildingProductionTaskCallback* queryBuildingProductionTaskCallback() {
        return nullptr;
    }

    virtual WorldEventCallback* queryWorldEventCallback() {
        return nullptr;
    }

    virtual OutsideInventoryCallback* queryOutsideInventoryCallback() {
        return nullptr;
    }

    virtual CraftCallback* queryCraftCallback() {
        return nullptr;
    }

    virtual AchievementCallback* queryAchievementCallback() {
        return nullptr;
    }

    virtual CharacterTitleCallback* queryCharacterTitleCallback() {
        return nullptr;
    }

    virtual ItemCallback* queryItemCallback() {
        return nullptr;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
