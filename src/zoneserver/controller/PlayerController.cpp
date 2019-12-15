#include "ZoneServerPCH.h"
#include "PlayerController.h"
#include "player/PlayerItemController.h"
#include "player/PlayerMailController.h"
#include "player/PlayerTradeController.h"
#include "player/PlayerBuildingController.h"
#include "player/PlayerInventoryController.h"
#include "player/PlayerQuestController.h"
#include "player/PlayerGraveStoneController.h"
#include "player/PlayerMoveController.h"
#include "player/PlayerSkillController.h"
#include "player/PlayerHarvestController.h"
#include "player/PlayerTreasureController.h"
#include "player/PlayerPartyController.h"
#include "player/PlayerAnchorController.h"
#include "player/PlayerGuildController.h"
#include "player/PlayerNpcController.h"
#include "player/PlayerCastController.h"
#include "player/PlayerArenaController.h"
#include "player/PlayerQuestController.h"
#include "player/PlayerDeviceController.h"
#include "player/PlayerTeleportController.h"
#include "player/PlayerDuelController.h"
#include "player/PlayerWorldEventController.h"
#include "player/PlayerOutsideInventoryController.h"
#include "player/PlayerEffectController.h"
#include "player/PlayerAchievementController.h"
#include "player/PlayerCharacterTitleController.h"
#include "../model/state/CreatureState.h"
#include "../model/gameobject/Player.h"
#include "../model/gameobject/GraveStone.h"
#include "../model/gameobject/Npc.h"
#include "../model/gameobject/Harvest.h"
#include "../model/gameobject/Dungeon.h"
#include "../model/gameobject/RandomDungeon.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/ability/CastGameTimeable.h"
#include "../model/gameobject/ability/FieldDuelable.h"
#include "../model/gameobject/ability/Guildable.h"
#include "../model/state/VehicleState.h"
#include "../model/state/GliderState.h"
#include "../model/item/Inventory.h"
#include "../model/item/QuestInventory.h"
#include "../model/item/EquipInventory.h"
#include "../model/state/ChaoState.h"
#include "../model/gameobject/skilleffect/EffectScriptApplier.h"
#include "../model/gameobject/status/PlayerStatus.h"
#include "../world/World.h"
#include "../world/WorldMap.h"
#include "../service/party/Party.h"
#include "../service/cheat/CheatCommander.h"
#include "../service/item/ItemOptionService.h"
#include "../service/arena/mode/Arena.h"
#include "../service/world_event/WorldEventService.h"
#include "../helper/InventoryHelper.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <sne/server/utility/Profiler.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <boost/algorithm/string.hpp>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class PlayerCombatStateChangeEvent
 */
class PlayerCombatStateChangeEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerCombatStateChangeEvent>
{
public:
    PlayerCombatStateChangeEvent(const ObjectId& playerId, bool isCombatState) :
        playerId_(playerId),
        isCombatState_(isCombatState) {}

private:
    virtual void call(go::Entity& entity) {
        if (entity.isPlayer()) {
            entity.getControllerAs<PlayerController>().evCombatStateChanged(playerId_,
                isCombatState_);
        }        
    }

private:
    const ObjectId playerId_;
    bool isCombatState_;
};


/**
 * @class EmotionEvent
 */
class EmotionEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<EmotionEvent>
{
public:
    EmotionEvent(ObjectId playerId, const std::string& emotion) :
        playerId_(playerId),
        emotion_(emotion) {}

private:
    virtual void call(go::Entity& entity) {
        EmotionCallback* emotionCallback = entity.getController().queryEmotionCallback();
        if (emotionCallback != nullptr) {
            emotionCallback->emotionNotified(playerId_, emotion_);
        }
    }

private:
    const ObjectId playerId_;
    const std::string emotion_;
};


/**
 * @class ChaoEvent
 */
class ChaoEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ChaoEvent>
{
public:
    ChaoEvent(ObjectId playerId, bool isChaoPlayer) :
        playerId_(playerId),
        isChaoPlayer_(isChaoPlayer) {}

private:
    virtual void call(go::Entity& entity) {
        ChaoCallback* chatCallback = entity.getController().queryChaoCallback();
        if (chatCallback != nullptr) {
            chatCallback->chaoChanged(playerId_, isChaoPlayer_);
        }
    }

private:
    const ObjectId playerId_;
    const bool isChaoPlayer_;
};


/**
 * @class ReleaseBeginnerProtectionEvent
 */
class ReleaseBeginnerProtectionEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ReleaseBeginnerProtectionEvent>
{
public:
    ReleaseBeginnerProtectionEvent(ObjectId playerId) :
        playerId_(playerId) {}

private:
    virtual void call(go::Entity& entity) {
        EntityStatusCallback* callback = entity.getController().queryEntityStatusCallback();
        if (callback!= nullptr) {
            callback->begginerProtectionReleased(playerId_);
        }
    }

private:
    const ObjectId playerId_;
};




/**
 * @class TempChaoEvent
 */
class TempChaoEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<TempChaoEvent>
{
public:
    TempChaoEvent(ObjectId playerId, bool isChaoPlayer) :
        playerId_(playerId),
        isChaoPlayer_(isChaoPlayer) {}

private:
    virtual void call(go::Entity& entity) {
        ChaoCallback* chatCallback = entity.getController().queryChaoCallback();
        if (chatCallback != nullptr) {
            chatCallback->tempChaoChanged(playerId_, isChaoPlayer_);
        }
    }

private:
    const ObjectId playerId_;
    const bool isChaoPlayer_;
};


/**
 * @class SayEvent
 */
class SayEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<SayEvent>
{
public:
    SayEvent(const Nickname& nickname, const ChatMessage& message) :
        nickname_(nickname),
        message_(message) {}

private:
    virtual void call(go::Entity& entity) {
        ChatCallback* chatCallback = entity.getController().queryChatCallback();
        if (chatCallback != nullptr) {
            chatCallback->creatureSaid(nickname_, message_);
        }
    }

private:
    const Nickname nickname_;
    const ChatMessage message_;
};


/**
 * @class ReviveEvent
 */
class PlayerReviveEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerReviveEvent>
{
public:
	PlayerReviveEvent(go::Player& player) :
		player_(player) {}

private:
	virtual void call(go::Entity& entity) {
		EntityStatusCallback* entityStatusCallback =
			entity.getController().queryEntityStatusCallback();
		if (entityStatusCallback != nullptr) {
			entityStatusCallback->playerRevived(player_);
		}
	}

private:
	go::Player& player_;
};

/**
 * @class PlayerDiedEvent
 */
class PlayerDiedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerDiedEvent>
{
public:
    PlayerDiedEvent(go::Player& player, const GameObjectInfo& killerInfo) :
        player_(player),
        killerInfo_(killerInfo) {}

private:
    virtual void call(go::Entity& entity) {
        EntityStatusCallback* entityStatusCallback =
            entity.getController().queryEntityStatusCallback();
        if (entityStatusCallback != nullptr) {
            entityStatusCallback->playerDied(player_, killerInfo_);
        }
    }

private:
    go::Player& player_;
    GameObjectInfo killerInfo_;
};


/**
 * @class PlayerLevelUpEvent
 */
class PlayerLevelUpEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerLevelUpEvent>
{
public:
    PlayerLevelUpEvent(const GameObjectInfo& playerInfo,
        bool isMajorLevelup) :
        playerInfo_(playerInfo),
        isMajorLevelup_(isMajorLevelup) {}

private:
    virtual void call(go::Entity& entity) {
        GrowthCallback* growthCallback =
            entity.getController().queryGrowthCallback();
        if (growthCallback  != nullptr) {
            growthCallback->playerLeveledUp(playerInfo_, isMajorLevelup_);
        }
    }

private:
    const GameObjectInfo playerInfo_;
    bool isMajorLevelup_;
};

/**
* @class PartyMemberLevelUpEvent
*/
class PartyMemberLevelUpEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PartyMemberLevelUpEvent>
{
public:
    PartyMemberLevelUpEvent(ObjectId memberId, CreatureLevel level) :
        memberId_(memberId),
        level_(level) {}

private:
    virtual void call(go::Entity& entity) {
        PartyCallback* partyCallback =
            entity.getController().queryPartyCallback();
        if (partyCallback != nullptr) {
            partyCallback->partyMemberLevelup(memberId_, level_);
        }
    }

private:
    const ObjectId memberId_;
    const CreatureLevel level_;
};

} // namespace {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerController);

void PlayerController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


// = EntityControllerAbility overriding

VehicleCallback* PlayerController::queryVehicleMountCallback() 
{
    return &getOwnerAs<go::Player>().getPlayerMoveController();
}


GliderCallback* PlayerController::queryGliderCallback() 
{ 
    return &getOwnerAs<go::Player>().getPlayerMoveController();
}


TradeCallback* PlayerController::queryTradeCallback()
{
    return &getOwnerAs<go::Player>().getPlayerTradeController();
}


QuestCallback* PlayerController::queryQuestCallback()
{
    return &getOwnerAs<go::Player>().getPlayerQuestController();
}


InventoryCallback* PlayerController::queryInventoryCallback()
{
    return &getOwnerAs<go::Player>().getPlayerInventoryController();
}


EquipInventoryCallback* PlayerController::queryEquipInventoryCallback()
{
    return &getOwnerAs<go::Player>().getPlayerInventoryController();
}


AccessoryInventoryCallback* PlayerController::queryAccessoryInventoryCallback()
{
    return &getOwnerAs<go::Player>().getPlayerInventoryController();
}


QuestInventoryCallback* PlayerController::queryQuestInventoryCallback()
{
    return &getOwnerAs<go::Player>().getPlayerInventoryController();
}


HarvestCallback* PlayerController::queryHarvestCallback()
{
    return &getOwnerAs<go::Player>().getPlayerHarvestController();
}


PartyCallback* PlayerController::queryPartyCallback()
{
    return &getOwnerAs<go::Player>().getPlayerPartyController();
}


BuildCallback* PlayerController::queryBuildCallback()
{
    return &getOwnerAs<go::Player>().getPlayerBuildingController();
}


CraftCallback* PlayerController::queryCraftCallback()
{
    return &getOwnerAs<go::Player>().getPlayerItemController();
}


SkillCallback* PlayerController::queryPlayerSkillCallback()
{
    return &getOwnerAs<go::Player>().getPlayerSkillController();
}


//AnchorAbillityCallback* PlayerController::queryAnchorAbillityCallback()
//{
//    return &getOwnerAs<go::Player>().getPlayerAnchorController();
//}
//

GuildCallback* PlayerController::queryGuildCallback()
{
    return &getOwnerAs<go::Player>().getPlayerGuildController();
}


DialogCallback* PlayerController::queryDialogCallback()
{
    return &getOwnerAs<go::Player>().getPlayerNpcController();
}


CastCallback* PlayerController::queryCastCallback()
{
    return &getOwnerAs<go::Player>().getPlayerCastController(); 
}


PassiveSkillCallback* PlayerController::queryPassiveSkillCallback()
{
    return &getOwnerAs<go::Player>().getPlayerSkillController();
}


MailCallback* PlayerController::queryMailCallback()
{
    return &getOwnerAs<go::Player>().getPlayerMailController();
}


ArenaCallback* PlayerController::queryArenaCallback()
{
    return &getOwnerAs<go::Player>().getPlayerArenaController();
}


DeviceCallback* PlayerController::queryDeviceCallback()
{
    return &getOwnerAs<go::Player>().getPlayerDeviceController();
}


TeleportCallback* PlayerController::queryTeleportCallback()
{
    return &getOwnerAs<go::Player>().getPlayerTeleportController();
}


SkillCallback* PlayerController::querySkillCallback()
{
    return &getOwnerAs<go::Player>().getPlayerSkillController();
}


GraveStoneCallback* PlayerController::queryGraveStoneCallback()
{
	return &getOwnerAs<go::Player>().getPlayerGraveStoneController();
}


NpcTalkingCallback* PlayerController::queryNpcTalkingCallback()
{
    return &getOwnerAs<go::Player>().getPlayerNpcController();
}


FieldDuelCallback* PlayerController::queryFieldDuelCallback()
{
    return &getOwnerAs<go::Player>().getPlayerDuelController();
}


BuildingProductionTaskCallback* PlayerController::queryBuildingProductionTaskCallback()
{
    return &getOwnerAs<go::Player>().getPlayerBuildingController();
}


WorldEventCallback* PlayerController::queryWorldEventCallback()
{
    return &getOwnerAs<go::Player>().getPlayerWorldEventController();
}


OutsideInventoryCallback* PlayerController::queryOutsideInventoryCallback()
{
    return &getOwnerAs<go::Player>().getPlayerOutsideInventoryController();
}


PassiveEffectCallback* PlayerController::queryPassiveEffectCallback()
{
    return &getOwnerAs<go::Player>().getPlayerEffectController();
}


AchievementCallback* PlayerController::queryAchievementCallback()
{
    return &getOwnerAs<go::Player>().getPlayerAchievementController();
}


CharacterTitleCallback* PlayerController::queryCharacterTitleCallback()
{
    return &getOwnerAs<go::Player>().getPlayerCharacterTitleController();
}


ItemCallback* PlayerController::queryItemCallback()
{
    return &getOwnerAs<go::Player>().getPlayerItemController();
}

// = EntityController overriding

void PlayerController::spawned(WorldMap& worldMap)
{
    CreatureController::spawned(worldMap);

    fullCreatureStatusChanged();
}


void PlayerController::despawned(WorldMap& worldMap)
{
    CreatureController::despawned(worldMap);
}

// = CreatureController overriding

void PlayerController::died(go::Entity* from)
{
    go::Player& owner = getOwnerAs<go::Player>();
    CreatureState* state = owner.queryCreatureState();
    if (state == nullptr) {
        return;
    }

    if (state->isDied()) {
        return;
    }

    FieldDualPtr fieldDual = getOwner().queryFieldDuelable()->getFieldDualPtr();
    if (fieldDual.get() != nullptr) {
        ObjectId winnerId = from != nullptr ? from->getObjectId() : invalidObjectId;
        fieldDual->stop(fdrtOk, winnerId);        
        return;
    }

	if (from) {
		SNE_LOG_INFO("GameLog PlayerController::died(killerId(%d, %" PRIu64 "), deadManId(%" PRIu64 ")", 
			from->getObjectType(), from->getObjectId(), owner.getObjectId());
	}
	
    if (from != nullptr) {
        Arena* arena = owner.queryArenaMatchable()->getArena();        
        if (arena) {
            arena->killed(*from, owner);
        }        
        else {
            go::Questable* questable = from->queryQuestable();
            if (questable != nullptr) {
                questable->killed(owner);
            }
        }
    }
    state->died();
    CreatureController::died(from);

    const GameObjectInfo& killerInfo = from ? from->getGameObjectInfo() : GameObjectInfo();
    auto event = std::make_shared<PlayerDiedEvent>(owner, killerInfo);
    owner.queryKnowable()->broadcast(event);

}

// = InterestAreaCallback overriding

void PlayerController::entitiesAppeared(const go::EntityMap& entities)
{
    CreatureController::entitiesAppeared(entities);

    WorldMap* worldMap = getOwner().getCurrentWorldMap();
    if (! worldMap) {
        assert(false);
        return;
    }

    const UnionEntityInfos enteredEntities =
        worldMap->toUnionEntityInfos(entities, &getOwner());
    if (! enteredEntities.empty()) {
        evEntitiesAppeared(enteredEntities);
    }
}


void PlayerController::entityAppeared(go::Entity& entity,
    const UnionEntityInfo& entityInfo)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    CreatureController::entityAppeared(entity, entityInfo);

    CreatureState* state = entity.queryCreatureState();
    if (state) {
        if (state->isHidden()) {
            return;
        }
    }

    evEntityAppeared(entityInfo);
}


void PlayerController::entitiesDisappeared(const go::EntityMap& entities)
{
    CreatureController::entitiesDisappeared(entities);

    GameObjects gameObjects;
    gameObjects.reserve(entities.size());

    for (const go::EntityMap::value_type& value : entities) {
        const GameObjectInfo& entityInfo = value.first;
        gameObjects.push_back(entityInfo);
    }

    evEntitiesDisappeared(gameObjects);
}


void PlayerController::entityDisappeared(go::Entity& entity)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    CreatureController::entityDisappeared(entity);

    if (! entity.isSame(getOwner())) {
        evEntityDisappeared(entity.getGameObjectInfo());
    }
}


void PlayerController::entityDisappeared(const GameObjectInfo& info)
{
    CreatureController::entityDisappeared(info);

    if (! getOwner().isSame(info)) {
        evEntityDisappeared(info);
    }
}


// = ChatCallback overriding

void PlayerController::creatureSaid(const Nickname& nickname,
    const ChatMessage& message)
{
    // TODO: 필드 상에 있는지 검사해야 한다
    evCreatureSaid(nickname, message);
}

// = EntityStatusCallback overriding

void PlayerController::playerDied(const go::Player& player, const GameObjectInfo& killerInfo)
{
    evPlayerDied(player.getGameObjectInfo(), killerInfo, player.getPosition());
}


void PlayerController::playerRevived(const go::Player& player)
{
	evPlayerRevived(player.getObjectId(), player.getPosition());
}


void PlayerController::npcDied(const GameObjectInfo& npcInfo,
    const GameObjectInfo& killerInfo, go::GraveStone* graveStone)
{
    if (graveStone != nullptr) {
        go::Player& owner = getOwnerAs<go::Player>();

        owner.queryKnowable()->know(*graveStone);
    }

    GraveStoneInfo baseGraveStoneInfo;
    if (graveStone != nullptr) {
        baseGraveStoneInfo = graveStone->getBaseGraveStoneInfo();
    }

	evNpcDied(npcInfo, killerInfo, baseGraveStoneInfo);
}


void PlayerController::entityThreated(const GameObjectInfo& entityInfo)
{
    evThreated(entityInfo);
}


void PlayerController::entityTargetSelected(const GameObjectInfo& entityInfo, const GameObjectInfo& target,
    bool isAgressive)
{
    evTargetSelected(entityInfo, target, isAgressive);
}


void PlayerController::entityTargetChanged(const GameObjectInfo& entityInfo, const EntityStatusInfo& targetStatusInfo)
{
    go::Player& owner = getOwnerAs<go::Player>();
    if (targetStatusInfo.targetInfo_ == owner.getSelectedTargetOfTarget()) {
        return;
    }
    owner.setTargetOfTarget(targetStatusInfo.targetInfo_);
    evTargetChanged(entityInfo, targetStatusInfo);
}


void PlayerController::creaturePointsRestored(const go::Creature& creature)
{
    evPointsRestored(creature.getGameObjectInfo());
}


void PlayerController::playerCombatStateChanged(bool isCombatState)
{
    go::Player& owner = getOwnerAs<go::Player>();
    if (isCombatState && owner.queryVehicleState()->isVehicleMounting()) {
        return;
    }
    if (isCombatState && owner.queryGliderState()->isGliderMounting()) {
        return;
    }

	if (owner.queryCombatStateable()->changeCombatState(isCombatState)) {
		auto event = std::make_shared<PlayerCombatStateChangeEvent>(owner.getObjectId(), isCombatState);
		owner.queryKnowable()->broadcast(event);
        go::PassiveSkillCastable* passiveSkillCastable = owner.queryPassiveSkillCastable();
        if (passiveSkillCastable) {
            passiveSkillCastable->notifyChangeCondition(pccUseableState);
        }
	}
}


void PlayerController::begginerProtectionReleased()
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<ReleaseBeginnerProtectionEvent>(owner.getObjectId());
    owner.queryKnowable()->broadcast(event);
}


void PlayerController::begginerProtectionReleased(ObjectId playerId)
{
    evBeginnerProtectionReleased(playerId);
}


// = RewardCallback overriding

void PlayerController::expRewarded(ExpPoint rewardExp)
{
    go::Player& owner = getOwnerAs<go::Player>();
    if (owner.getCreatureStatus().isDied()) {
        return;
    }
    go::Rewardable* rewardble = owner.queryRewardable();
    if (rewardble) {
        bool isLevelup = false;
        bool isMajorLevelUp = false;
        rewardble->expReward(isLevelup, isMajorLevelUp, rewardExp);        
        if (isLevelup) {
            auto event = std::make_shared<PlayerLevelUpEvent>(
                owner.getGameObjectInfo(), isMajorLevelUp);
            owner.queryKnowable()->broadcast(event);
            owner.getCreatureStatus().restorePoints();
        }

        if (isMajorLevelUp) {
            go::Partyable* partyable = owner.queryPartyable();
            if (partyable) {
                PartyPtr party = partyable->getParty();
                if (party.get() != nullptr) {
                    auto event = std::make_shared<PartyMemberLevelUpEvent>(
                        owner.getObjectId(), static_cast<go::Creature&>(owner).getCreatureLevel());
                    party->notifyNearEvent(event, owner.getObjectId(), true);
                }
            }
        }
    }
}


void PlayerController::gameMoneyRewarded(GameMoney gameMoney, bool isUp)
{
    go::Player& owner = getOwnerAs<go::Player>();
	if (isUp) {
		owner.queryMoneyable()->upGameMoney(gameMoney);
        permil_t valuePer = owner.queryGuildable()->getGuildEffectValue(estGuildGameMoneyBonus);
        if (0 < valuePer) {
            const GameMoney bonusMoney = gameMoney * valuePer / 1000;
            owner.getController().queryGuildCallback()->addGuildGameMoney(bonusMoney);
        }
	}
	else {
		owner.queryMoneyable()->downGameMoney(gameMoney);
	}
    evMoneyRewarded(owner.queryMoneyable()->getGameMoney());
}


ErrorCode PlayerController::itemRewarded(const BaseItemInfo& baseItemInfo)
{
    go::Player& owner = getOwnerAs<go::Player>();
    return owner.queryInventoryable()->getInventory().addItem(createAddItemInfoByBaseItemInfo(baseItemInfo));	
}


bool PlayerController::questItemRewarded(const QuestItemInfo& questItem)
{
	go::Player& owner = getOwnerAs<go::Player>();
	return owner.queryInventoryable()->getQuestInventory().addItem(questItem);	
}


void PlayerController::chaoticUpRewared(Chaotic chaotic)
{
    go::Player& owner = getOwnerAs<go::Player>();
    
    owner.queryChaoable()->upChaotic(chaotic);
    evChaoticUpdated(owner.queryChaoable()->getChaotic());
}


void PlayerController::changeCharacterClass(CharacterClass cc)
{
    go::Entity& owner = getOwner();
    
    owner.queryCharacterClassable()->changeCharacterClass(cc);
    evCharacterClassChanged(owner.getObjectId(), cc);
}


void PlayerController::skillRewared(SkillTableType tableType, SkillIndex index)
{
    go::Entity& owner = getOwner();
    owner.querySkillLearnable()->learnSkill(tableType, index);
}


ErrorCode PlayerController::updateEquipRewared(EquipCode newEquipCode)
{
    go::Entity& owner = getOwner();
	return owner.queryInventoryable()->getEquipInventory().upgradeRewardEquippedItem(newEquipCode);
}


void PlayerController::characterClassChanged(ObjectId playerId, CharacterClass cc)
{
    evCharacterClassChanged(playerId, cc);
}


// = CheatCallback overriding

void PlayerController::cheatRewardExp(ExpPoint point)
{
    expRewarded(point);
}


void PlayerController::cheatZoneInUsers(const ZoneUserInfos& infos) 
{
    evZoneInUsers(infos);
}


void PlayerController::cheatWorldInUsers(const WorldUserInfos& infos)
{
    evWorldInUsers(infos);
}


void PlayerController::cooldownInfosAdded(const CooltimeInfos& cooltimeInfos)
{
    go::Entity& owner = getOwner();
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAddCooldownInfos(owner.getAccountId(), owner.getObjectId(), cooltimeInfos);
}


void PlayerController::remainEffectsAdded(const RemainEffectInfos& effectInfos)
{
    go::Entity& owner = getOwner();
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAddRemainEffects(owner.getAccountId(), owner.getObjectId(), effectInfos);
}


void PlayerController::actionPlayed(const GameObjectInfo& entityInfo, uint32_t actionCode)
{
    evActionPlayed(entityInfo, actionCode);
}


void PlayerController::pointChanged(const GameObjectInfo& entityInfo,
    PointType pointType, uint32_t currentPoint)
{
    evPointChanged(entityInfo, pointType, currentPoint);
}


void PlayerController::maxPointChanged(const GameObjectInfo& entityInfo,
    PointType pointType, uint32_t currentPoint, uint32_t maxPoint)
{
    evMaxPointChanged(entityInfo, pointType, currentPoint, maxPoint);
}


void PlayerController::creatureStatusChanged(EffectStatusType effectStatusType,
    int32_t currentValue)
{
    evCreatureStatusChanged(effectStatusType, currentValue);
}


void PlayerController::allAttributeChanged(const AttributeRates& attribute, bool isResist)
{
    evAllAttributeChanged(attribute, isResist);
}


void PlayerController::pointsRestored(const GameObjectInfo& entityInfo)
{
    evPointsRestored(entityInfo);
}


void PlayerController::fullCreatureStatusChanged()
{
    go::Creature& owner = getOwnerAs<go::Creature>();
    evFullCreatureStatusInfoChanged(owner.getCreatureStatus().getFullCreatureStatusInfo());
}


void PlayerController::creatureLifeStatsChanged(const GameObjectInfo& entityInfo,
    const LifeStats& lifeStats)
{
    evLifeStatusChanged(entityInfo, lifeStats);
}


//void PlayerController::attributeRateChanged(
//    AttributeRateIndex pointType, AttributeRate currentRate)
//{
//    evAttributeRateChanged(pointType, currentRate);
//}
//
//
//void PlayerController::powerChanged(PowerType pwt, uint32_t currentValue)
//{
//    evPowerChanged(pwt,currentValue);
//}


void PlayerController::chaoticUpdated(Chaotic chaotic)
{
    evChaoticUpdated(chaotic);
}


void PlayerController::shieldCreated(const GameObjectInfo& entityInfo, PointType pointType, uint32_t value)
{
    evShieldCreated(entityInfo, pointType, value);
}


void PlayerController::shieldDestroyed(const GameObjectInfo& entityInfo, PointType pointType)
{
    evShieldDestroyed(entityInfo, pointType);
}


void PlayerController::shieldPointChanged(const GameObjectInfo& entityInfo, 
    PointType pointType, uint32_t currentPoint)
{
    evShieldPointChanged(entityInfo, pointType, currentPoint);
}


void PlayerController::changeChao(bool isChaoPlayer)
{
    go::Player& owner = getOwnerAs<go::Player>();
    ChaoState* chaoState = owner.queryChaoState();
    if (chaoState) {
        if (chaoState->changeChaoState(isChaoPlayer)) {
            auto event = std::make_shared<ChaoEvent>(owner.getObjectId(), isChaoPlayer);
            owner.queryKnowable()->broadcast(event);
        }
    }
}


void PlayerController::changeTempChao(bool isChaoPlayer)
{
    go::Player& owner = getOwnerAs<go::Player>();
    ChaoState* chaoState = owner.queryChaoState();
    if (chaoState) {
        if (chaoState->changeTempChaoState(isChaoPlayer)) {
            auto event = std::make_shared<TempChaoEvent>(owner.getObjectId(), isChaoPlayer);
            owner.queryKnowable()->broadcast(event);
        }
    }
}


void PlayerController::chaoChanged(ObjectId playerId, bool isChaoPlayer)
{
    evChaoChanged(playerId, isChaoPlayer);
}


void PlayerController::tempChaoChanged(ObjectId playerId, bool isChaoPlayer)
{
    evTempChaoChanged(playerId, isChaoPlayer);
}


void PlayerController::emotionNotified(ObjectId playerId, const std::string& emotion)
{
    evEmotionNotified(playerId, emotion);
}


void PlayerController::commandMoved(const Position& position, bool shouldStopDestination)
{
    evBotMovePosition(position, shouldStopDestination);
}


void PlayerController::commandSkillCasted()
{
    evBotCastingSkill();
}

// GrowthCallback overriding

void PlayerController::playerLeveledUp(const GameObjectInfo& creatureInfo,
    bool isMajorLevelup)
{
    evPlayerLeveledUp(creatureInfo, isMajorLevelup);
}

// = sne::srpc::RpcForwarder overriding

void PlayerController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::CheatRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerController, cheat,
    ChatMessage, cheatMessage)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.isValid()) {
        return;
    }

    const size_t size = cheatMessage.find_first_of(L" ");
    std::wstring command = boost::algorithm::to_lower_copy(cheatMessage.substr(0, size));
    const std::wstring params = cheatMessage.substr(size + 1);
	const ErrorCode errorCode = owner.getCheatCommander().execute(command, params);
	
	SNE_LOG_INFO("cheat Character(C%" PRIu64 ", Command:[%W]) ",
		owner.getObjectId(), cheatMessage.c_str());
	onCheat(errorCode, cheatMessage);
}


FORWARD_SRPC_METHOD_2(PlayerController, onCheat,
	ErrorCode, errorCode, ChatMessage, cheatMessage);


FORWARD_SRPC_METHOD_1(PlayerController, evZoneInUsers,
    ZoneUserInfos, infos);


FORWARD_SRPC_METHOD_1(PlayerController, evWorldInUsers,
    WorldUserInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerController, evCheatValueTypeUpdated,
	CheatValueType, cheatType, uint64_t, currentValue);


FORWARD_SRPC_METHOD_1(PlayerController, evCheatCompleteQuestRemoved,
	QuestCode, questCode);


// = GamePlayRpc overriding

RECEIVE_SRPC_METHOD_0(PlayerController, readyToPlay)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }


    Arena* arena = owner.queryArenaMatchable()->getArena();
    if (arena) {
        arena->readyToPlay(owner);
    }
    
    ErrorCode errorCode = ecOk;
    WorldMap* currentWorldMap = owner.getCurrentWorldMap();
    if (currentWorldMap != nullptr) {
        const ObjectPosition spawnPosition = owner.getPosition();
        errorCode = getOwner().spawn(*currentWorldMap, spawnPosition);
        if (isFailed(errorCode)) {
            assert(false);
        }
    }
    else {
        errorCode = ecWorldMapNotFound;
    }

    if (isFailed(errorCode)) {
        //if (currentWorldMap != nullptr) {
        //    if (currentWorldMap->isDungeon()) {
        //        WorldMap& prevWorldMap = WORLD->getGlobalWorldMap();
        //        owner.setWorldMap(*prevWorldMap);
        //        owner.setMapRegion(prevWorldMap->getGlobalMapRegion());
        //        // 이후 클라이언트에서 readyToPlay()를 호출하면 기존 월드맵에 입장하게 된다
        //    }
        //}
    }
    else {       
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncQueryHasNotMail(owner.getObjectId());    
    }

    onReadyToPlay(errorCode);

    if (isSucceeded(errorCode) && ! owner.queryArenaMatchable()->getArena()) {        
        WORLD_EVENT_SERVICE->login(owner);        
    }
}


RECEIVE_SRPC_METHOD_0(PlayerController, queryServerTime)
{
    onQueryServerTime(getTime());
}


FORWARD_SRPC_METHOD_1(PlayerController, onReadyToPlay,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_1(PlayerController, onQueryServerTime,
    sec_t, serverTime);


// = rpc::CharacterInterestAreaRpc overriding

FORWARD_SRPC_METHOD_1(PlayerController, evEntitiesAppeared,
    UnionEntityInfos, entityInfos);


FORWARD_SRPC_METHOD_1(PlayerController, evEntityAppeared,
    UnionEntityInfo, entityInfo);


FORWARD_SRPC_METHOD_1(PlayerController, evEntitiesDisappeared,
    GameObjects, entities);


FORWARD_SRPC_METHOD_1(PlayerController, evEntityDisappeared,
    GameObjectInfo, entityInfo);

// = rpc::EntityQueryRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerController, selectTarget,
    GameObjectInfo, entityInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    if (! entityInfo.isValid()) {
        owner.queryTargetSelectable()->unselectTarget();
        onSelectTarget(ecOk, EntityStatusInfo(entityInfo), EntityStatusInfo());
        return;
    }

    go::Entity* entity = owner.queryKnowable()->getEntity(entityInfo);
    if (! entity) {
        onSelectTarget(ecEntityNotFound, EntityStatusInfo(entityInfo), EntityStatusInfo());
        return;
    }

    const go::Entity* targetOfTarget = owner.queryTargetSelectable()->selectTarget(entityInfo);
    onSelectTarget(ecOk, WORLD->getEntityStatusInfo(entity), WORLD->getEntityStatusInfo(targetOfTarget));

    if (entity->isGraveStone()) {
        go::GraveStone& graveStone = static_cast<go::GraveStone&>(*entity);
        graveStone.restartGraveStoneDespawnTask();
    }
}


FORWARD_SRPC_METHOD_3(PlayerController, onSelectTarget,
    ErrorCode, errorCode, EntityStatusInfo, targetInfo, EntityStatusInfo, targetOfTargetInfo);

// = rpc::EntityActionRpc overriding

FORWARD_SRPC_METHOD_2(PlayerController, evActionPlayed,
    GameObjectInfo, entityInfo, uint32_t, actionCode);

// = rpc::CharacterSocialRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerController, say,
    ChatMessage, message)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    auto event = std::make_shared<SayEvent>(owner.getNickname(), message);
    owner.queryKnowable()->broadcast(event);
}


FORWARD_SRPC_METHOD_2(PlayerController, evCreatureSaid,
    Nickname, nickname, ChatMessage, message);

// = rpc::EntityStatusRpc overriding
RECEIVE_SRPC_METHOD_0(PlayerController, commitSuicide)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Creature& owner = getOwnerAs<go::Creature>();    

    owner.getCreatureStatus().reduceHp(owner.getCreatureStatus().getMaxPoints().hp_, true);
    gc::LifeStatusCallback* lifeStatusCallback = 
        owner.getController().queryLifeStatusCallback();
    if (lifeStatusCallback) {
        lifeStatusCallback->died(&owner);
    }
}


RECEIVE_SRPC_METHOD_0(PlayerController, releaseBeginnerProtection)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }
    
    if (owner.queryProtectionable()->isBeginnerProtection()) {
        owner.queryProtectionable()->releaseBeginnerProtection();
    }
}


FORWARD_SRPC_METHOD_3(PlayerController, evPlayerDied,
    GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, ObjectPosition, position);


FORWARD_SRPC_METHOD_3(PlayerController, evNpcDied,
    GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, GraveStoneInfo, graveStoneInfo);


FORWARD_SRPC_METHOD_1(PlayerController, evThreated,
    GameObjectInfo, entityInfo);


FORWARD_SRPC_METHOD_3(PlayerController, evTargetSelected,
    GameObjectInfo, entityInfo, GameObjectInfo, targetInfo, bool, isAggressive);


FORWARD_SRPC_METHOD_2(PlayerController, evTargetChanged,
    GameObjectInfo, entityInfo, EntityStatusInfo, targetStatusInfo);


FORWARD_SRPC_METHOD_1(PlayerController, evBeginnerProtectionReleased,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_2(PlayerController, evCombatStateChanged,
	ObjectId, playerId, bool, isCombatState)


// = rpc::CreatureReviveRpc overriding

RECEIVE_SRPC_METHOD_0(PlayerController, revive)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }    

    if (owner.queryArenaMatchable()->getArena()) {
        return;
    }

    if (owner.getEffectScriptApplier().hasEffectScipts(estExecution)) {
        return;
    }

    const ErrorCode errorCode = owner.queryLiveable()->revive();
	if (isSucceeded(errorCode)) {
		auto event = std::make_shared<PlayerReviveEvent>(owner);
		owner.queryKnowable()->broadcast(event, true);
	}
    
    onRevive(errorCode, owner.getPosition());
}


FORWARD_SRPC_METHOD_2(PlayerController, onRevive,
    ErrorCode, errorCode, ObjectPosition, position);


FORWARD_SRPC_METHOD_2(PlayerController, evPlayerRevived,
	ObjectId, reviverId, ObjectPosition, position);

// = rpc::ActionBarRpc overriding

RECEIVE_SRPC_METHOD_3(PlayerController, saveActionBar,
    ActionBarIndex, abiIndex, ActionBarPosition, abpIndex, DataCode, code)
{
    sne::server::Profiler profiler(__FUNCTION__);
    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    const ErrorCode errorCode = owner.saveActionBar(abiIndex, abpIndex, code);
    onSaveActionBar(errorCode, abiIndex, abpIndex, code);
}


RECEIVE_SRPC_METHOD_1(PlayerController, lockActionBar,
    bool, isLocked)
{
    sne::server::Profiler profiler(__FUNCTION__);
    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    owner.setActionBarLock(isLocked);
}


FORWARD_SRPC_METHOD_4(PlayerController, onSaveActionBar,
    ErrorCode, errorCode, ActionBarIndex, abiIndex,
    ActionBarPosition, abpIndex, DataCode, code);


// = GrowthRpc overriding

FORWARD_SRPC_METHOD_4(PlayerController, evPlayerLeveledUpInfo,
    CurrentLevel, levelInfo, ExpPoint, exp, ExpPoint, rewardExp, SkillPoint, skillPoint);


FORWARD_SRPC_METHOD_1(PlayerController, evExpPointUpdated,
    ExpPoint, currentExp);


FORWARD_SRPC_METHOD_2(PlayerController, evPlayerLeveledUp,
    GameObjectInfo, playerInfo, bool, isMajorLevelUp);

// = rpc::StatsRpc overriding

FORWARD_SRPC_METHOD_3(PlayerController, evPointChanged,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, currentPoint);


FORWARD_SRPC_METHOD_4(PlayerController, evMaxPointChanged,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, currentPoint, uint32_t, maxPoint);


FORWARD_SRPC_METHOD_2(PlayerController, evCreatureStatusChanged,
    EffectStatusType, effectStatusType, int32_t, currentValue);


FORWARD_SRPC_METHOD_2(PlayerController, evAllAttributeChanged,
    AttributeRates, rates, bool, isResist);


FORWARD_SRPC_METHOD_1(PlayerController, evPointsRestored,
    GameObjectInfo, playerInfo);


FORWARD_SRPC_METHOD_1(PlayerController, evFullCreatureStatusInfoChanged,
    FullCreatureStatusInfo, stats);


FORWARD_SRPC_METHOD_2(PlayerController, evLifeStatusChanged,
    GameObjectInfo, entityInfo, LifeStats, stats);


FORWARD_SRPC_METHOD_2(PlayerController, evChaoChanged,
    ObjectId, playerId, bool, isChao);


FORWARD_SRPC_METHOD_2(PlayerController, evTempChaoChanged,
    ObjectId, playerId, bool, isChao);


FORWARD_SRPC_METHOD_1(PlayerController, evChaoticUpdated,
    Chaotic, chaotic);


FORWARD_SRPC_METHOD_3(PlayerController, evShieldCreated,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, shieldPoint);


FORWARD_SRPC_METHOD_2(PlayerController, evShieldDestroyed,
    GameObjectInfo, entityInfo, PointType, pointType);


FORWARD_SRPC_METHOD_3(PlayerController, evShieldPointChanged,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, currentPoint);

// = rpc::DungeonRpc overriding


RECEIVE_SRPC_METHOD_1(PlayerController, enterDungeon,
    ObjectId, dungeonId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    WorldPosition spawnPosition;
    std::string mapData;

    if (owner.queryVehicleState()->isVehicleMounting()) {
        onEnterDungeon(ecDungeonNotEnterdVehicleMount, spawnPosition, mapData, invalidPartyId);
        return;
    }

    if (owner.queryGliderState()->isGliderMounting()) {
        onEnterDungeon(ecDungeonNotEnterdGliderMount, spawnPosition, mapData, invalidPartyId);
        return;
    }

    const GameObjectInfo dungeonInfo(otDungeon, dungeonId);
    go::Dungeon* dungeon =
        static_cast<go::Dungeon*>(owner.queryKnowable()->getEntity(dungeonInfo));
    if (! dungeon) {
        onEnterDungeon(ecDungeonNotFound, spawnPosition, mapData, invalidPartyId);
        return;
    }


    ErrorCode errorCode = dungeon->enter(owner);
    if (isFailed(errorCode)) {
        onEnterDungeon(errorCode, spawnPosition, mapData, invalidPartyId);
        return;
    }

    WorldMap* dungeonWorldMap = dungeon->getWorldMap();
    PartyId partyId = invalidPartyId;
    if (dungeonWorldMap != nullptr) {
        assert(isDungeon(dungeonWorldMap->getMapType()));

        errorCode = getOwner().despawn();
        if (isSucceeded(errorCode)) {
            Vector2 offset = getDirection(dungeon->getPosition().heading_) * 10.0f;
            WorldPosition worldPosition = owner.getWorldPosition();
            worldPosition.x_ += offset.x;
            worldPosition.y_ += offset.y;
            owner.queryPositionable()->setPosition(worldPosition);

            spawnPosition = dungeonWorldMap->getCharacterSpawnPosition();
            mapData = dungeon->getMapData();

            owner.setWorldMap(*dungeonWorldMap);
            owner.setMapRegion(dungeonWorldMap->getGlobalMapRegion());
            owner.queryPositionable()->setPosition(spawnPosition);
            lastDungeonId_ = dungeonId;
            // todo 다른 던전이 나오면 수정            
            partyId = static_cast<go::RandomDungeon*>(dungeon)->getPartyId();;
            
        }
    }

    onEnterDungeon(errorCode, spawnPosition, mapData, partyId);
}


RECEIVE_SRPC_METHOD_0(PlayerController, leaveDungeon)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    WorldMap* currentWorldMap = owner.getCurrentWorldMap();
    if (! currentWorldMap) {
        assert(false);
        return;
    }

    if (! currentWorldMap->isDungeon()) {
        onLeaveDungeon(ecDungeonNotEntered, WorldPosition());
        return;
    }

    const WorldPosition worldPosition = owner.getWorldPosition();
    WorldMap& prevWorldMap = WORLD->getGlobalWorldMap();

    const ErrorCode errorCode = getOwner().despawn();
    if (isFailed(errorCode)) {
        assert(false);
        onLeaveDungeon(errorCode, WorldPosition());
    }

    owner.setWorldMap(prevWorldMap);
    owner.setMapRegion(prevWorldMap.getGlobalMapRegion());

    owner.queryPositionable()->setPosition(worldPosition);
    lastDungeonId_ = invalidObjectId;

    onLeaveDungeon(ecOk, worldPosition);
}


FORWARD_SRPC_METHOD_4(PlayerController, onEnterDungeon,
    ErrorCode, errorCode, WorldPosition, spawnPosition, std::string, mapData, PartyId, partyId);


FORWARD_SRPC_METHOD_2(PlayerController, onLeaveDungeon,
    ErrorCode, errorCode, WorldPosition, spawnPosition);

// = rpc::RewardRpc overriding

FORWARD_SRPC_METHOD_1(PlayerController, evMoneyRewarded, 
    GameMoney, money);


FORWARD_SRPC_METHOD_2(PlayerController, evCharacterClassChanged,
    ObjectId, playerId, CharacterClass, characterClass);

FORWARD_SRPC_METHOD_2(PlayerController, evBotMovePosition,
    Position, position, bool, shouldStopDestination);


FORWARD_SRPC_METHOD_0(PlayerController, evBotCastingSkill);

// = rpc::EmotionRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerController, notifyEmotion,
    std::string, emotion)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Player& owner = getOwnerAs<go::Player>();
    if (! owner.canRequest()) {
        return;
    }

    auto event = std::make_shared<EmotionEvent>(owner.getObjectId(), emotion);
    owner.queryKnowable()->broadcast(event);
}


FORWARD_SRPC_METHOD_2(PlayerController, evEmotionNotified,
    ObjectId, playerId, std::string, emotion);

// = rpc::PropertyRpc overriding

RECEIVE_SRPC_METHOD_0(PlayerController, loadProperties)
{
    go::Player& owner = getOwnerAs<go::Player>();
    owner.loadProperties();
}


RECEIVE_SRPC_METHOD_2(PlayerController, saveProperties,
    std::string, config, std::string, prefs)
{
    go::Player& owner = getOwnerAs<go::Player>();
    owner.saveProperties(config, prefs);
}


FORWARD_SRPC_METHOD_2(PlayerController, onLoadProperties,
    std::string, config, std::string, prefs);


}}} // namespace gideon { namespace zoneserver { namespace gc   {
