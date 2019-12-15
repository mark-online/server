#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/rpc/PlayerRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace clientnet {
class ServerProxy;
}} // namespace gideon { namespace clientnet {

namespace gideon { namespace bot {

/**
 * @class BotPlayer
 */
class BotPlayer :
    public rpc::PlayerRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(BotPlayer);

public:
    BotPlayer(clientnet::ServerProxy& serverProxy);

    void initialize(const UserId& userId, FullCharacterInfo& characterInfo);
    void reset();

    void worldEntered(const ObjectPosition& position);
    
public:
    bool isInitialized() const {
        return characterInfo_ != nullptr;
    }

    bool isEnteredIntoWorld() const {
        return isInitialized();
    }

    bool isEnterWorld() const {
        return isEnterWorld_;
    }

    bool isReadyToPlay() const {
        return isReadyToPlay_;
    }

    const ObjectPosition& getPosition() const {
        assert(isInitialized());
        return characterInfo_->position_;
    }

    bool shouldMove() const {
       return isMoving_;
    }

    void moveTo(const Position& destination);
    void move();

    bool canCastManaSkill() const;
    void castManaSkill();

private:
    bool isApproachedToDestination(float32_t distance) const;
    bool setDestination(const Position& destination);
    float32_t calcCurrentPosition(ObjectPosition& position,
        GameTime elapsedTime);
    

public:
    // = rpc::CheatRpc overriding
    OVERRIDE_SRPC_METHOD_1(cheat,
        ChatMessage, cheatMessage);

	OVERRIDE_SRPC_METHOD_2(onCheat,
		ErrorCode, errorCode, ChatMessage, cheatMessage);

	OVERRIDE_SRPC_METHOD_1(evZoneInUsers,
		ZoneUserInfos, infos);
    OVERRIDE_SRPC_METHOD_1(evWorldInUsers,
        WorldUserInfos, infos);

	OVERRIDE_SRPC_METHOD_2(evCheatValueTypeUpdated,
		CheatValueType, cheatType, uint64_t, currentValue);
	OVERRIDE_SRPC_METHOD_1(evCheatCompleteQuestRemoved,
		QuestCode, questCode);
	
public:
    // = rpc::GamePlayRpc overriding
    OVERRIDE_SRPC_METHOD_0(readyToPlay);
    OVERRIDE_SRPC_METHOD_0(queryServerTime);

    OVERRIDE_SRPC_METHOD_1(onReadyToPlay,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onQueryServerTime,
        sec_t, serverTime);

    // = rpc::CreatureInterestAreaRpc overriding
    OVERRIDE_SRPC_METHOD_1(evEntitiesAppeared,
        UnionEntityInfos, entityInfos);
    OVERRIDE_SRPC_METHOD_1(evEntityAppeared,
        UnionEntityInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_1(evEntitiesDisappeared,
        GameObjects, entities);
    OVERRIDE_SRPC_METHOD_1(evEntityDisappeared,
        GameObjectInfo, entityInfo);

    // = rpc::EntityQueryRpc overriding
    OVERRIDE_SRPC_METHOD_1(selectTarget,
        GameObjectInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_3(onSelectTarget,
        ErrorCode, errorCode, EntityStatusInfo, targetInfo, EntityStatusInfo, targetOfTargetInfo);

    // = rpc::CreatureMovementRpc overriding
    OVERRIDE_SRPC_METHOD_1(goToward,
        ObjectPosition, destination);
    OVERRIDE_SRPC_METHOD_1(move,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(stop,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(turn,
        Heading, heading);
    OVERRIDE_SRPC_METHOD_1(jump,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_0(run);
    OVERRIDE_SRPC_METHOD_0(walk);
    OVERRIDE_SRPC_METHOD_0(fallen);
    OVERRIDE_SRPC_METHOD_0(landing);
    OVERRIDE_SRPC_METHOD_1(environmentEntered,
        EnvironmentType, type);
    OVERRIDE_SRPC_METHOD_0(environmentLeft);

    OVERRIDE_SRPC_METHOD_1(onLanding,
        HitPoint, damage);

    OVERRIDE_SRPC_METHOD_1(correctionPosition,
        ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(regionEntered,
        RegionCode, regionCode);
    OVERRIDE_SRPC_METHOD_1(regionLeft,
        RegionCode, regionCode);

    OVERRIDE_SRPC_METHOD_2(evEntityGoneToward,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityMoved,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityStopped,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityTeleported,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEntityTurned,
        GameObjectInfo, entityInfo, Heading, heading);
    OVERRIDE_SRPC_METHOD_2(evEntityJumped,
        GameObjectInfo, entityInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_1(evRun,
        GameObjectInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_1(evWalked,
        GameObjectInfo, entityInfo);

    // = rpc::CreatureSocialRpc overriding
    OVERRIDE_SRPC_METHOD_1(say,
        ChatMessage, message);

    OVERRIDE_SRPC_METHOD_2(evCreatureSaid,
        Nickname, nickname, ChatMessage, message);

    // = rpc::VehicleRpc overriding
    OVERRIDE_SRPC_METHOD_1(dismountVehicle,
        bool, isForce);

    OVERRIDE_SRPC_METHOD_1(onDismountVehicle,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_3(evVehicleDismounted,
        GameObjectInfo, entityInfo, float32_t, speed, bool, isForce);

    // = rpc::GliderRpc overriding
    OVERRIDE_SRPC_METHOD_0(dismountGlider);

    OVERRIDE_SRPC_METHOD_2(onDismountGlider,
        ErrorCode, errorCode, uint32_t, currentDurability);

    OVERRIDE_SRPC_METHOD_2(evGliderDismounted,
        GameObjectInfo, creatureInfo, float32_t, speed);
    

    // = rpc::SkillRpc overriding
    OVERRIDE_SRPC_METHOD_1(learnSkills,
        SkillCodes, skillCodes);
    OVERRIDE_SRPC_METHOD_0(resetLearnedSkills);

    OVERRIDE_SRPC_METHOD_2(onLearnSkills,
        ErrorCode, errorCode, SkillPoint, currentSkillPoint);
    OVERRIDE_SRPC_METHOD_3(onResetLearnedSkills,
        ErrorCode, errorCode, GameMoney, currentGameMoney, SkillPoint, currentSkillPoint);

    OVERRIDE_SRPC_METHOD_2(evConcentrationSkillCancelled,
        GameObjectInfo, entityInfo, SkillCode, skillCode);
	OVERRIDE_SRPC_METHOD_2(evConcentrationSkillCompleted,
		GameObjectInfo, entityInfo, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_2(evSkillLearned,
        SkillCode, removeSkillCode, SkillCode, addSkillCode);
    OVERRIDE_SRPC_METHOD_2(evPassiveSkillActivated,
        GameObjectInfo, entityInfo, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_0(evAllSkillRemoved);

    // = rpc::InventoryRpc overriding
    OVERRIDE_SRPC_METHOD_3(moveInventoryItem,
        InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_3(switchInventoryItem,
        InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_2(removeInventoryItem,
        InvenType, invenType, ObjectId, itemId1);
    OVERRIDE_SRPC_METHOD_1(equipItem,
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(unequipItem,
        ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_2(equipAccessoryItem,
        ObjectId, itemId, AccessoryIndex, index);
    OVERRIDE_SRPC_METHOD_2(unequipAccessoryItem,
        ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_4(divideItem,
        InvenType, invenType, ObjectId, itemId, uint8_t, count, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_3(movePlayerInvenItemToBankInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, SlotId, bankInvenSlot);
    OVERRIDE_SRPC_METHOD_3(moveBankInvenItemToPlayerInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, bankItemId, SlotId, playerInvenSlot);
    OVERRIDE_SRPC_METHOD_3(switchBankInvenItemAndPlayerInvenItem,
        GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, ObjectId, bankItemId);
	OVERRIDE_SRPC_METHOD_2(moveInvenItemToVehicleInvenItem,
		ObjectId, playerItemId, SlotId, vehicleInvenSlot);
	OVERRIDE_SRPC_METHOD_2(moveVehicleInvenItemToPlayerInvenItem,
		ObjectId, vehicleItemId, SlotId, playerInvenSlot);
	OVERRIDE_SRPC_METHOD_2(switchVehicleInvenItemAndPlayerInvenItem,
		ObjectId, playerItemId, ObjectId, vehicleItemId);
    OVERRIDE_SRPC_METHOD_2(depositGameMoney,
        GameObjectInfo, targetInfo, GameMoney, invenGameMoney);
    OVERRIDE_SRPC_METHOD_2(withdrawGameMoney,
        GameObjectInfo, targetInfo, GameMoney, bankGameMoney);
    
    OVERRIDE_SRPC_METHOD_4(onMoveInventoryItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_4(onSwitchInventoryItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_3(onRemoveInventoryItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1);

    OVERRIDE_SRPC_METHOD_2(onEquipItem,
        ErrorCode, errorCode, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_3(onUnequipItem,
        ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_4(onDivideItem,
        ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, uint8_t, count);

    OVERRIDE_SRPC_METHOD_3(onEquipAccessoryItem,
        ErrorCode, errorCode, ObjectId, itemId, AccessoryIndex, index);
    OVERRIDE_SRPC_METHOD_3(onUnequipAccessoryItem,
        ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_1(onMovePlayerInvenItemToBankInvenItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onMoveBankInvenItemToPlayerInvenItem,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onSwitchBankInvenItemAndPlayerInvenItem,
        ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_1(onMoveInvenItemToVehicleInvenItem,
		ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_1(onMoveVehicleInvenItemToPlayerInvenItem,
		ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_1(onSwitchVehicleInvenItemAndPlayerInvenItem,
		ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_3(onDepositGameMoney,
        ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney);
    OVERRIDE_SRPC_METHOD_3(onWithdrawGameMoney,
        ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney);


    OVERRIDE_SRPC_METHOD_2(evItemEquipped,
        GameObjectInfo, creatureInfo, EquipCode, equipCode);
    OVERRIDE_SRPC_METHOD_2(evItemUnequipped,
        GameObjectInfo, creatureInfo, EquipCode, equipCode);

    OVERRIDE_SRPC_METHOD_3(evInventoryItemCountUpdated,
        InvenType, invenType, ObjectId, itemId, uint8_t, ItemCount);
    OVERRIDE_SRPC_METHOD_2(evInventoryItemAdded,
        InvenType, invenType, ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_3(evInventoryEquipItemChanged,
        ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount);
    OVERRIDE_SRPC_METHOD_3(evEquipItemEnchanted,
        ObjectId, itemId, SocketSlotId, id, EquipSocketInfo, socketInfo);
    OVERRIDE_SRPC_METHOD_2(evEquipItemUnenchanted,
        ObjectId, itemId, SocketSlotId, id);


    OVERRIDE_SRPC_METHOD_2(evInventoryItemRemoved,
        InvenType, invenType, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_3(evInventoryItemMoved,
        InvenType, invenType, ObjectId, itemId, SlotId, slotId);
    OVERRIDE_SRPC_METHOD_3(evInventoryItemSwitched,
        InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);
    OVERRIDE_SRPC_METHOD_3(evUnequipItemReplaced,
        ObjectId, equipItemId, SlotId, slotId, EquipPart, unequipPrt);
    OVERRIDE_SRPC_METHOD_2(evEquipItemReplaced,
        ObjectId, equipItemId, EquipPart, equipPrt);
    OVERRIDE_SRPC_METHOD_2(evInventoryWithEquipItemReplaced,
        ObjectId, equipItemId1, ObjectId, equipItemId2);
    OVERRIDE_SRPC_METHOD_3(evUnequipAccessoryItemReplaced, 
        ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPrt);
    OVERRIDE_SRPC_METHOD_2(evEquipAccessoryItemReplaced,
        ObjectId, itemId, AccessoryIndex, equipPrt);
    OVERRIDE_SRPC_METHOD_4(evInventoryWithAccessoryItemReplaced,
        ObjectId, unequipItemId, AccessoryIndex, unequipIndex,
        ObjectId, equipItemId, AccessoryIndex, equipIndex);
    OVERRIDE_SRPC_METHOD_3(evInventoryInfoUpdated, 
        InvenType, invenTpe, bool, isCashSlot, uint8_t, currentExtendSlot);

    OVERRIDE_SRPC_METHOD_1(evQuestItemAdded, 
        QuestItemInfo, questItemInfo);
    OVERRIDE_SRPC_METHOD_1(evQuestItemRemoved,
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(evQuestItemUsableCountUpdated,
        ObjectId, itemId, uint8_t, usableCount);
	OVERRIDE_SRPC_METHOD_2(evQuestItemCountUpdated,
		ObjectId, itemId, uint8_t, stackCount);

    // = rpc::ItemRpc overriding
    OVERRIDE_SRPC_METHOD_3(unenchantEquip,
        GameObjectInfo, checkObjectInfo, ObjectId, equipItemId, SocketSlotId, openSlotIndex);
	OVERRIDE_SRPC_METHOD_1(useVehicleItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(useGliderItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(selectVehicleItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(selectGliderItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(repairGliderItem,
		ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_1(queryLootItemInfoMap,
		GameObjectInfo, targetInfo);

    OVERRIDE_SRPC_METHOD_4(onUnenchantEquip,
        ErrorCode, errorCode, ObjectId, equipItemId, uint8_t, openSlotIndex, GameMoney, currentGameMoney);
	OVERRIDE_SRPC_METHOD_2(onUseVehicleItem,
		ErrorCode, errorCode, VehicleInfo, info);
	OVERRIDE_SRPC_METHOD_2(onUseGliderItem,
		ErrorCode, errorCode, GliderInfo, info);
	OVERRIDE_SRPC_METHOD_2(onSelectVehicleItem,
		ErrorCode, errorCode, ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_2(onSelectGliderItem,
		ErrorCode, errorCode, ObjectId, itemId);
	OVERRIDE_SRPC_METHOD_3(onRepairGliderItem,
		ErrorCode, errorCode, ObjectId, itemId, GameMoney, currentGameMoney);
	OVERRIDE_SRPC_METHOD_3(onLootItemInfoMap,
		ErrorCode, errorCode, GameObjectInfo, targetInfo, LootInvenItemInfoMap, infoMap);

	OVERRIDE_SRPC_METHOD_2(buyItem,
		ObjectId, npcId, BaseItemInfo, itemInfo);
	OVERRIDE_SRPC_METHOD_3(sellItem,
		ObjectId, npcId, ObjectId, itemId, uint8_t, itemCount);
    OVERRIDE_SRPC_METHOD_1(queryBuyBackItemInfos,
        ObjectId, npcId);
    OVERRIDE_SRPC_METHOD_2(buyBackItem,
        ObjectId, npcId, uint32_t, index);


    OVERRIDE_SRPC_METHOD_5(onBuyItem,
        ErrorCode, errorCode, ObjectId, npcId, BaseItemInfo, itemInfo,
        CostType, costType, uint64_t, currentCostValue);
    OVERRIDE_SRPC_METHOD_5(onSellItem,
        ErrorCode, errorCode, uint32_t, buyBackIndex, BuyBackItemInfo, buyBackInfo, CostType, costType, uint64_t, currentValue);
    OVERRIDE_SRPC_METHOD_2(onBuyBackItemInfos,
        ErrorCode, errorCode, BuyBackItemInfoMap, itemMap);
    OVERRIDE_SRPC_METHOD_4(onBuyBackItem,
        ErrorCode, errorCode, uint32_t, index, CostType, ct, uint64_t, currentValue);


    OVERRIDE_SRPC_METHOD_1(acquireHarvest,
        DataCode, itemCode);
    OVERRIDE_SRPC_METHOD_2(onAcquireHarvest,
        ErrorCode, errorCode, DataCode, itemCode);

    OVERRIDE_SRPC_METHOD_3(evHarvestRewarded,
        ObjectId, haravestId, BaseItemInfos, itemInfos,
        QuestItemInfos, questItemInfos);

    OVERRIDE_SRPC_METHOD_1(closeTreasure,
        ObjectId, treasureId);
    
    OVERRIDE_SRPC_METHOD_2(onCloseTreasure,
        ErrorCode, errorCode, ObjectId, treasureId);
    
    // = rpc::EntityStatusRpc overriding
    OVERRIDE_SRPC_METHOD_0(commitSuicide);
    OVERRIDE_SRPC_METHOD_0(releaseBeginnerProtection);

    OVERRIDE_SRPC_METHOD_3(evPlayerDied,
        GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_3(evNpcDied,
        GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, GraveStoneInfo, graveStoneInfo);
    OVERRIDE_SRPC_METHOD_1(evThreated,
        GameObjectInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_3(evTargetSelected,
        GameObjectInfo, entityInfo, GameObjectInfo, targetInfo, bool, isAggressive);
    OVERRIDE_SRPC_METHOD_2(evTargetChanged,
        GameObjectInfo, entityInfo, EntityStatusInfo, targetStatusInfo);
    OVERRIDE_SRPC_METHOD_1(evPointsRestored,
        GameObjectInfo, playerInfo);
    OVERRIDE_SRPC_METHOD_1(evBeginnerProtectionReleased,
        ObjectId, playerId);
	OVERRIDE_SRPC_METHOD_2(evCombatStateChanged,
		ObjectId, playerId, bool, isCombatState);


    // = rpc::CreatureReviveRpc overriding
    OVERRIDE_SRPC_METHOD_0(revive);

    OVERRIDE_SRPC_METHOD_2(onRevive,
        ErrorCode, errorCode, ObjectPosition, position);

	OVERRIDE_SRPC_METHOD_2(evPlayerRevived,
		ObjectId, reviverId, ObjectPosition, position);

    
    OVERRIDE_SRPC_METHOD_3(saveActionBar,
        ActionBarIndex, abiIndex, ActionBarPosition, abpIndex, DataCode, code);
    OVERRIDE_SRPC_METHOD_1(lockActionBar,
        bool, isLocked);

    // = rpc::ActionBarRpc overriding
    OVERRIDE_SRPC_METHOD_4(onSaveActionBar,
        ErrorCode, errorCode, ActionBarIndex, abiIndex,
        ActionBarPosition, abpIndex, DataCode, code);

    // = rpc::GrowthRpc overriding
    OVERRIDE_SRPC_METHOD_4(evPlayerLeveledUpInfo,
        CurrentLevel, levelInfo, ExpPoint, exp, ExpPoint, rewardExp, SkillPoint, skillPoint);
    OVERRIDE_SRPC_METHOD_1(evExpPointUpdated,
        ExpPoint, currentExp);
    OVERRIDE_SRPC_METHOD_2(evPlayerLeveledUp,
        GameObjectInfo, playerInfo, bool, isMajorLevelUp);

    // = rpc::StatsRpc overriding
    OVERRIDE_SRPC_METHOD_3(evPointChanged,
        GameObjectInfo, creatureInfo, PointType, pointType, 
        uint32_t, currentPoint);
    OVERRIDE_SRPC_METHOD_4(evMaxPointChanged,
        GameObjectInfo, creatureInfo, PointType, pointType, 
        uint32_t, currentPoint, uint32_t, maxPoint);
    OVERRIDE_SRPC_METHOD_2(evCreatureStatusChanged,
        EffectStatusType, effectStatusType, int32_t, currentValue);
    OVERRIDE_SRPC_METHOD_2(evAllAttributeChanged,
        AttributeRates, rates, bool, isResist);
    OVERRIDE_SRPC_METHOD_1(evFullCreatureStatusInfoChanged,
        FullCreatureStatusInfo, stats);
    OVERRIDE_SRPC_METHOD_2(evLifeStatusChanged,
        GameObjectInfo, entityInfo, LifeStats, stats);
    OVERRIDE_SRPC_METHOD_3(evShieldCreated,
        GameObjectInfo, entityInfo, PointType, pointType, 
        uint32_t, shieldPoint);
    OVERRIDE_SRPC_METHOD_2(evShieldDestroyed,
        GameObjectInfo, entityInfo, PointType, pointType);
    OVERRIDE_SRPC_METHOD_3(evShieldPointChanged,
        GameObjectInfo, entityInfo, PointType, pointType, 
        uint32_t, currentPoint);

    // = rpc::Chao overriding
    OVERRIDE_SRPC_METHOD_2(evChaoChanged,
        ObjectId, playerId, bool, isChao);
    OVERRIDE_SRPC_METHOD_2(evTempChaoChanged,
        ObjectId, playerId, bool, isChao);
    OVERRIDE_SRPC_METHOD_1(evChaoticUpdated,
        Chaotic, chaotic);

    // = rpc::TradeRpc overriding
    OVERRIDE_SRPC_METHOD_1(requestTrade,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(respondTrade,
        ObjectId, playerId, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_0(cancelTrade);
    OVERRIDE_SRPC_METHOD_1(toggleTradeReady,
        GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_0(toggleTradeConform);
    OVERRIDE_SRPC_METHOD_1(addTradeItem, 
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_1(removeTradeItem, 
        ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(switchTradeItem, 
        ObjectId, invenItemId, ObjectId, tradeItemId);

    OVERRIDE_SRPC_METHOD_2(onRequestTrade,
        ErrorCode, erroCode, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_3(onRespondTrade,
        ErrorCode, erroCode, ObjectId, playerId, bool, isAnwser);
    OVERRIDE_SRPC_METHOD_1(onCancelTrade,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_3(onToggleTradeReady,
        ErrorCode, errorCode, bool, isReady, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_2(onToggleTradeConform,
        ErrorCode, errorCode, bool, isConform);
    OVERRIDE_SRPC_METHOD_2(onAddTradeItem, 
        ErrorCode, errorCode, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_2(onRemoveTradeItem, 
        ErrorCode, errorCode, ObjectId, itemId);
    OVERRIDE_SRPC_METHOD_3(onSwitchTradeItem, 
        ErrorCode, errorCode, ObjectId, invenItemId, ObjectId, tradeItemId);

    OVERRIDE_SRPC_METHOD_1(evTradeRequested,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(evTradeReponded,
        ObjectId, playerId, bool, isAnwser);
    OVERRIDE_SRPC_METHOD_1(evTradeCancelled,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(evTradeReadyToggled,
        bool, isReady, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_1(evTradeConformToggled,
        bool, isConform);
    OVERRIDE_SRPC_METHOD_1(evTradeItemAdded,
        ItemInfo, itemInfo);
    OVERRIDE_SRPC_METHOD_1(evTradeItemRemoved,
        ObjectId, itemId);

    // = rpc::DungeonRpc overriding
    OVERRIDE_SRPC_METHOD_1(enterDungeon,
        ObjectId, dungeonId);
    OVERRIDE_SRPC_METHOD_0(leaveDungeon);

    OVERRIDE_SRPC_METHOD_4(onEnterDungeon,
        ErrorCode, errorCode, WorldPosition, spawnPosition, std::string, mapData, PartyId, partyId);
    OVERRIDE_SRPC_METHOD_2(onLeaveDungeon,
        ErrorCode, errorCode, WorldPosition, spawnPosition);

    // = QuestRpc overriding
    OVERRIDE_SRPC_METHOD_2(acceptQuest,
        QuestCode, questCode, GameObjectInfo, objectInfo);
    OVERRIDE_SRPC_METHOD_1(cancelQuest,
        QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_2(completeTransportMission,
        QuestCode, questCode, QuestMissionCode, questMissionCode);
    OVERRIDE_SRPC_METHOD_3(completeQuest,
        QuestCode, questCode, ObjectId, npcId, DataCode, selectItemCode);
    OVERRIDE_SRPC_METHOD_4(completeTransportQuest,
        QuestCode, questCode, QuestMissionCode, questMissionCode, ObjectId, npcId, DataCode, selectItemCode);
    OVERRIDE_SRPC_METHOD_2(doContentsQuest, 
        QuestContentMissionType, contentMissionType, DataCode, dataCode);
    OVERRIDE_SRPC_METHOD_1(pushQuestToParty, 
        QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_3(responsePushQuestToParty, 
        QuestCode, questCode, GameObjectInfo, pusherInfo, bool, isAccept);

    OVERRIDE_SRPC_METHOD_2(onAcceptQuest,
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_2(onCancelQuest,
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_3(onCompleteTransportMission,
        ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode);
    OVERRIDE_SRPC_METHOD_3(onCompleteQuest,
        ErrorCode, errorCode, QuestCode, questCode, DataCode, selectItemCode);
    OVERRIDE_SRPC_METHOD_5(onCompleteTransportQuest,
        ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode, 
        ObjectId, npcId, DataCode, selectItemCode);
    OVERRIDE_SRPC_METHOD_2(onPushQuestToParty, 
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_3(onResponsePushQuestToParty, 
        ErrorCode, errorCode, QuestCode, questCode, bool, isAccept);

    OVERRIDE_SRPC_METHOD_3(evQuestMissionUpdated,
        QuestCode, questCode, QuestMissionCode, missionCode,
        QuestGoalInfo, questGoalInfo);
    OVERRIDE_SRPC_METHOD_2(evQuestToPartyPushed, 
        QuestCode, questCode, GameObjectInfo, member);
    OVERRIDE_SRPC_METHOD_3(evQuestToPartyPushResult, 
        QuestCode, questCode, GameObjectInfo, member, QuestToPartyResultType, resultType);
    
    // = TeleportRpc
    OVERRIDE_SRPC_METHOD_2(teleportToRegion,
        MapCode, worldMapCode, RegionCode, spawnRegionCode);
    OVERRIDE_SRPC_METHOD_2(teleportToPosition,
        MapCode, worldMapCode, Position, position);
    OVERRIDE_SRPC_METHOD_1(addBindRecall,
        ObjectId, bindlingId);
    OVERRIDE_SRPC_METHOD_1(removeBindRecall,
        ObjectId, bindlingId);
	OVERRIDE_SRPC_METHOD_1(setRecallToPlayer,
		Nickname, nickname);
	OVERRIDE_SRPC_METHOD_1(responseRecall,
		bool, isAnswer);

    OVERRIDE_SRPC_METHOD_5(onTeleportTo,
        ErrorCode, errorCode, MapCode, worldMapCode, ObjectPosition, spawnPosition,
        MigrationTicket, migrationTicket, GameMoney, currentGameMoney);
    OVERRIDE_SRPC_METHOD_2(onAddBindRecall,
        ErrorCode, ErrorCode, BindRecallInfo, bindRecallInfo);
    OVERRIDE_SRPC_METHOD_2(onRemoveBindRecall,
        ErrorCode, errorCode, ObjectId, bindlingId);
	OVERRIDE_SRPC_METHOD_1(onSetRecallToPlayer,
		ErrorCode, errorCode);
	OVERRIDE_SRPC_METHOD_2(onResponseRecall,
		ErrorCode, errorCode, MigrationTicket, migrationTicket);


	OVERRIDE_SRPC_METHOD_2(evBindRecallTicket,
		ErrorCode, errorCode, MigrationTicket, migrationTicket);
	OVERRIDE_SRPC_METHOD_1(evRecallRequested,
		Nickname, callerNickname);


    OVERRIDE_SRPC_METHOD_1(evMoneyRewarded,
        GameMoney, gameMoney);

    // = PartyRpc 
    OVERRIDE_SRPC_METHOD_1(queryPartyMemberSubInfo,
        ObjectId, memberId);
   OVERRIDE_SRPC_METHOD_6(onPartyMemberSubInfo,
        ObjectId, memberId, CharacterClass, characterClass,
        CreatureLevel, level, HitPoints, hitPoints, ManaPoints, manaPoints, Position, position);
    OVERRIDE_SRPC_METHOD_2(evPartyMemberLevelup,
        ObjectId, objectId, CreatureLevel, level);
    OVERRIDE_SRPC_METHOD_2(evPartyMemberMoved,
        ObjectId, objectId, Position, position);

    OVERRIDE_SRPC_METHOD_2(createAnchor,
        ObjectId, itemId, ObjectPosition, position);
    //OVERRIDE_SRPC_METHOD_1(buildAnchor,
    //    GameObjectInfo, anchorInfo);
    //OVERRIDE_SRPC_METHOD_3(putfuelItem,
    //    GameObjectInfo, anchorInfo, ObjectId, itemId, SlotId, ancorInvenId);
    //OVERRIDE_SRPC_METHOD_3(popfuelItem,
    //    GameObjectInfo, anchorInfo, ObjectId, itemId, SlotId, invenId);
    //OVERRIDE_SRPC_METHOD_1(queryInsideAnchorInfo,
    //    GameObjectInfo, anchorInfo);
    //OVERRIDE_SRPC_METHOD_3(excuteAnchorEffectByPosition,
    //    GameObjectInfo, anchorInfo, uint8_t, index, Position, position);
    //OVERRIDE_SRPC_METHOD_3(excuteAnchorEffectByTarget,
    //    GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo);


    OVERRIDE_SRPC_METHOD_1(onCreateAnchor,
        ErrorCode, errorCode);
    //OVERRIDE_SRPC_METHOD_2(onBuildAnchor,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo);
    //OVERRIDE_SRPC_METHOD_3(onPutfuelItem,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemInfo, itemInfo);
    //OVERRIDE_SRPC_METHOD_3(onPopfuelItem,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ObjectId, itemId);
    //OVERRIDE_SRPC_METHOD_3(onQueryInsideAnchorInfo,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemMap, itemMap);
    //OVERRIDE_SRPC_METHOD_4(onExcuteAnchorEffectByPosition,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, Position, position);
    //OVERRIDE_SRPC_METHOD_4(onExcuteAnchorEffectByTarget,
    //    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo);

    //OVERRIDE_SRPC_METHOD_3(evAnchorStartBuilt,
    //    GameObjectInfo, anchorInfo, ObjectId, ownerId, sec_t, startBuildTime);
    //OVERRIDE_SRPC_METHOD_1(evAnchorBroken,
    //    GameObjectInfo, anchorInfo);
    //OVERRIDE_SRPC_METHOD_1(evAnchorReconnaissanced,
    //    Positions, positions);
    //OVERRIDE_SRPC_METHOD_3(evAnchorPositionFired,
    //    GameObjectInfo, anchorInfo, uint8_t, index, Position, position);
    //OVERRIDE_SRPC_METHOD_3(evAnchorTargetFired,
    //    GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo);

    OVERRIDE_SRPC_METHOD_2(evCharacterClassChanged,
        ObjectId, playerId, CharacterClass, characterClass);

    OVERRIDE_SRPC_METHOD_2(evBotMovePosition,
        Position, position, bool, shouldStopDestination);
    OVERRIDE_SRPC_METHOD_0(evBotCastingSkill);

    OVERRIDE_SRPC_METHOD_1(notifyEmotion,
        std::string, emotion);

    OVERRIDE_SRPC_METHOD_2(evEmotionNotified,
        ObjectId, playerId, std::string, emotion);

    // = rpc::GuildRpc overriding
    OVERRIDE_SRPC_METHOD_1(purchaseVault,
        BaseVaultInfo, vaultInfo);
    OVERRIDE_SRPC_METHOD_1(activateGuildSkill,
        SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_0(deactivateGuildSkills);

    OVERRIDE_SRPC_METHOD_2(onPurchaseVault,
        ErrorCode, errorCode, BaseVaultInfo, vaultInfo);
    OVERRIDE_SRPC_METHOD_2(onActivateGuildSkill,
        ErrorCode, errorCode, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_1(onDeactivateGuildSkills,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_2(evGuildCreated,
        ObjectId, characterId, BaseGuildInfo, guildInfo);
    OVERRIDE_SRPC_METHOD_2(evGuildJoined,
        ObjectId, characterId, BaseGuildInfo, guildInfo);
    OVERRIDE_SRPC_METHOD_1(evGuildLeft,
        ObjectId, characterId);
    OVERRIDE_SRPC_METHOD_1(evGuildSkillActivated,
        SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_0(evGuildSkillsDeactivated);

    // = rpc::PropertyRpc overriding
    OVERRIDE_SRPC_METHOD_0(loadProperties);
    OVERRIDE_SRPC_METHOD_2(saveProperties,
        std::string, config, std::string, prefs);

    OVERRIDE_SRPC_METHOD_2(onLoadProperties,
        std::string, config, std::string, prefs);

    // = rpc::NpcDialogRpc overriding
    OVERRIDE_SRPC_METHOD_1(openDialog,
        GameObjectInfo, npcInfo);
    OVERRIDE_SRPC_METHOD_1(closeDialog,
        GameObjectInfo, npcInfo);

    OVERRIDE_SRPC_METHOD_2(evDialogOpened,
        GameObjectInfo, npc, GameObjectInfo, requester);
    OVERRIDE_SRPC_METHOD_2(evDialogClosed,
        GameObjectInfo, npc, GameObjectInfo, requester);

    // = rpc::ActionRpc overriding
    OVERRIDE_SRPC_METHOD_2(evActionPlayed,
        GameObjectInfo, entityInfo, uint32_t, actionCode);

    // = rpc::NpcTalkingRpc overriding
    OVERRIDE_SRPC_METHOD_2(evTalkedFrom,
        GameObjectInfo, talker, NpcTalkingCode, talkingCode);

    // = rpc::CastRpc override overriding
    OVERRIDE_SRPC_METHOD_1(startCasting,
        StartCastInfo, startInfo);
    OVERRIDE_SRPC_METHOD_0(cancelCasting);

    OVERRIDE_SRPC_METHOD_2(onStartCasting,
        ErrorCode, errorCode, StartCastInfo, startInfo);

    OVERRIDE_SRPC_METHOD_1(evCasted,
        StartCastResultInfo, startInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCanceled,
        CancelCastResultInfo, cancelInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCompleted,
        CompleteCastResultInfo, completeInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCompleteFailed,
        FailCompletedCastResultInfo, failInfo);

    OVERRIDE_SRPC_METHOD_3(evEffectApplied,
        GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
        SkillEffectResult, skillEffectResult);
    OVERRIDE_SRPC_METHOD_3(evItemEffected,
        GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
        ItemEffectResult, itemEffectResult);
    OVERRIDE_SRPC_METHOD_3(evMesmerizationEffected,
        GameObjectInfo, entityInfo, MesmerizationType, mezt, bool, isActivate);
    OVERRIDE_SRPC_METHOD_3(evCreatureTransformed,
        GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);
    OVERRIDE_SRPC_METHOD_3(evCreatureMutated,
        GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);
    OVERRIDE_SRPC_METHOD_1(evCreatureReleaseHidden,
        UnionEntityInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_2(evCreatureMoveSpeedChanged,
        GameObjectInfo, entityInfo, float32_t, currentSpeed);

    OVERRIDE_SRPC_METHOD_3(evCreatureFrenzied,
        GameObjectInfo, entityInfo, float32_t, currnetScale, bool, isActivate);
    OVERRIDE_SRPC_METHOD_2(evCreatureKnockbacked,
        GameObjectInfo, entityInfo, Position, position);
    OVERRIDE_SRPC_METHOD_1(evCreatureKnockbackReleased,
        GameObjectInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_2(evCreatureDashing,
        GameObjectInfo, entityInfo, Position, position);
    OVERRIDE_SRPC_METHOD_1(evPlayerGraveStoneStood,
        GraveStoneInfo, graveStoneInfo);
    OVERRIDE_SRPC_METHOD_3(evReviveEffected,
        GameObjectInfo, entityInfo, HitPoint, currentPoint, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_2(evEffectAdded,
        GameObjectInfo, entityInfo, DebuffBuffEffectInfo, info);
    OVERRIDE_SRPC_METHOD_3(evEffectRemoved,
        GameObjectInfo, entityInfo, DataCode, dataCode, bool, isCaster);
    OVERRIDE_SRPC_METHOD_3(evEffectHit,
        GameObjectInfo, casterInfo, GameObjectInfo, targetInfo, DataCode, dataCode);

    OVERRIDE_SRPC_METHOD_0(evSelfGraveStoneRemoved);

public:
    // = rpc::DeviceRpc overriding
    OVERRIDE_SRPC_METHOD_1(evDeviceDeactivated,
        ObjectId, deviceId);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    FullCharacterInfo* characterInfo_;
    UserId userId_;
    bool isEnterWorld_;
    bool isReadyToPlay_;
    bool isDestinationChanged_;
	bool shouldStopDestination_;
    bool isMoving_;
    bool isCastSkill_;
    float32_t moveTolerance_;
    ObjectPosition destination_;
    GameTime lastMoveTick_;
};

}} // namespace gideon { namespace bot {
