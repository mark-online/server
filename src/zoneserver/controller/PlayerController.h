#pragma once

#include "CreatureController.h"
#include "callback/ChatCallback.h"
#include "callback/EntityStatusCallback.h"
#include "callback/RewardCallback.h"
#include "callback/GrowthCallback.h"
#include "callback/CheatCallback.h"
#include "callback/StatsCallback.h"
#include "callback/BotCommandCallback.h"
#include "callback/ChaoCallback.h"
#include "callback/EmotionCallback.h"
#include "callback/CooldownCallback.h"
#include "callback/DialogCallback.h"
#include "callback/EntityActionCallback.h"
#include "callback/PassiveSkillCallback.h"
#include <gideon/cs/shared/rpc/player/CheatRpc.h>
#include <gideon/cs/shared/rpc/player/GamePlayRpc.h>
#include <gideon/cs/shared/rpc/player/EntityInterestAreaRpc.h>
#include <gideon/cs/shared/rpc/player/EntityQueryRpc.h>
#include <gideon/cs/shared/rpc/player/EntityActionRpc.h>
#include <gideon/cs/shared/rpc/player/CreatureSocialRpc.h>
#include <gideon/cs/shared/rpc/player/EntityStatusRpc.h>
#include <gideon/cs/shared/rpc/player/CreatureReviveRpc.h>
#include <gideon/cs/shared/rpc/player/ActionBarRpc.h>
#include <gideon/cs/shared/rpc/player/RewardRpc.h>
#include <gideon/cs/shared/rpc/player/BotCommandRpc.h>
#include <gideon/cs/shared/rpc/player/StatsRpc.h>
#include <gideon/cs/shared/rpc/player/DungeonRpc.h>
#include <gideon/cs/shared/rpc/player/ChaoRpc.h>
#include <gideon/cs/shared/rpc/player/EmotionRpc.h>
#include <gideon/cs/shared/rpc/player/PropertyRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {

namespace gideon { namespace zoneserver {
class WorldMap;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerController
 * Controller for Player
 */
class ZoneServer_Export PlayerController : public CreatureController,
    public rpc::CheatRpc,
    public rpc::GamePlayRpc,
    public rpc::EntityInterestAreaRpc,
    public rpc::EntityQueryRpc,
    public rpc::EntityActionRpc,
    public rpc::CreatureSocialRpc,
    public rpc::EntityStatusRpc,
    public rpc::CreatureReviveRpc,
    public rpc::ActionBarRpc,
    public rpc::RewardRpc,
    public rpc::StatsRpc,
    public rpc::DungeonRpc,
    public rpc::ChaoRpc,
    public rpc::BotCommandRpc,
    public rpc::EmotionRpc,
    public rpc::ProperyRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver,
    private ChatCallback,
    private EntityStatusCallback,
    private RewardCallback,
    private GrowthCallback,
    private CheatCallback,
    private StatsCallback,
    private ChaoCallback,
    private BotCommandCallback,
    private EmotionCallback,
    private CooldownCallback,
    private EntityActionCallback
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerController);

public:
    PlayerController() :
        lastDungeonId_(invalidObjectId) {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

public:
    // = EntityControllerAbility overriding
    virtual VehicleCallback* queryVehicleMountCallback();
    virtual GliderCallback* queryGliderCallback();
    virtual ChatCallback* queryChatCallback() { return this; }
    virtual EntityStatusCallback* queryEntityStatusCallback() { return this; }
    virtual RewardCallback* queryRewardCallback() {return this;}
    virtual GrowthCallback* queryGrowthCallback() {return this;}
    virtual CheatCallback* queryCheatCallback() {return this;}
    virtual StatsCallback* queryStatsCallback() {return this;}
    virtual ChaoCallback* queryChaoCallback() {return this;}
    virtual BotCommandCallback* queryBotCommandCallback() {return this;}
    virtual EmotionCallback* queryEmotionCallback() { return this; }
    virtual CooldownCallback* queryCooldownCallback() { return this; }
    virtual TradeCallback* queryTradeCallback();
    virtual InventoryCallback* queryInventoryCallback();
    virtual EquipInventoryCallback* queryEquipInventoryCallback();
    virtual AccessoryInventoryCallback* queryAccessoryInventoryCallback();
    virtual QuestInventoryCallback* queryQuestInventoryCallback();
    virtual QuestCallback* queryQuestCallback();
    virtual HarvestCallback* queryHarvestCallback();
    virtual PartyCallback* queryPartyCallback();
    virtual BuildCallback* queryBuildCallback();
    virtual CraftCallback* queryCraftCallback();
    virtual SkillCallback* queryPlayerSkillCallback();
    //virtual AnchorAbillityCallback* queryAnchorAbillityCallback();
    virtual GuildCallback* queryGuildCallback();
    virtual DialogCallback* queryDialogCallback();
    virtual EntityActionCallback* queryEntityActionCallback() {return this;}
    virtual CastCallback* queryCastCallback();
    virtual PassiveSkillCallback* queryPassiveSkillCallback();
    virtual MailCallback* queryMailCallback();
	virtual ArenaCallback* queryArenaCallback();
    virtual DeviceCallback* queryDeviceCallback();
	virtual TeleportCallback* queryTeleportCallback();
    virtual SkillCallback* querySkillCallback();
	virtual GraveStoneCallback* queryGraveStoneCallback();
    virtual NpcTalkingCallback* queryNpcTalkingCallback();
    virtual FieldDuelCallback* queryFieldDuelCallback();
    virtual BuildingProductionTaskCallback* queryBuildingProductionTaskCallback();
    virtual WorldEventCallback* queryWorldEventCallback();
    virtual OutsideInventoryCallback* queryOutsideInventoryCallback();
    virtual PassiveEffectCallback* queryPassiveEffectCallback();
    virtual AchievementCallback* queryAchievementCallback();
    virtual CharacterTitleCallback* queryCharacterTitleCallback();
    virtual ItemCallback* queryItemCallback();

private:
    // = EntityController overriding
    virtual void spawned(WorldMap& worldMap);
    virtual void despawned(WorldMap& worldMap);

protected:
    // = CreatureController overriding
    virtual void died(go::Entity* from);

private:
    // = InterestAreaCallback overriding
    virtual void entitiesAppeared(const go::EntityMap& entities);
    virtual void entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo);
    virtual void entitiesDisappeared(const go::EntityMap& entities);
    virtual void entityDisappeared(go::Entity& entity);
    virtual void entityDisappeared(const GameObjectInfo& info);

    // = ChatCallback overriding
    virtual void creatureSaid(const Nickname& nickname,
        const ChatMessage& message);

    // = EntityStatusCallback overriding
    virtual void playerDied(const go::Player& player, const GameObjectInfo& killerInfo);
	virtual void playerRevived(const go::Player& player);
    virtual void npcDied(const GameObjectInfo& npcInfo, const GameObjectInfo& killerInfo, go::GraveStone* graveStone);
    virtual void entityThreated(const GameObjectInfo& entityInfo);
    virtual void entityTargetSelected(const GameObjectInfo& entityInfo, const GameObjectInfo& target, bool isAgressive);
    virtual void entityTargetChanged(const GameObjectInfo& entityInfo, const EntityStatusInfo& targetStatusInfo);
    virtual void creaturePointsRestored(const go::Creature& creature);
    virtual void playerCombatStateChanged(bool isCombatState);
    virtual void begginerProtectionReleased();
    virtual void begginerProtectionReleased(ObjectId playerId);

    // = RewardCallback overriding
    virtual void expRewarded(ExpPoint rewardExp);
    virtual void gameMoneyRewarded(GameMoney gameMoney, bool isUp);
    virtual ErrorCode itemRewarded(const BaseItemInfo& baseItemInfo);
	virtual bool questItemRewarded(const QuestItemInfo& questItem);
    virtual void chaoticUpRewared(Chaotic chaotic);
    virtual void changeCharacterClass(CharacterClass cc);
    virtual void skillRewared(SkillTableType tableType, SkillIndex index);
	virtual ErrorCode updateEquipRewared(EquipCode newEquipCode);

    virtual void characterClassChanged(ObjectId playerId, CharacterClass cc);
    // = GrowthCallback overriding
    virtual void playerLeveledUp(const GameObjectInfo& creatureInfo,
        bool isMajorLevelup);

    // = CheatCallback overriding
    virtual void cheatRewardExp(ExpPoint point);
    virtual void cheatZoneInUsers(const ZoneUserInfos& infos);
    virtual void cheatWorldInUsers(const WorldUserInfos& infos);

    // = CooldownCallback overriding
    virtual void cooldownInfosAdded(const CooltimeInfos& cooltimeInfos);
    virtual void remainEffectsAdded(const RemainEffectInfos& effectInfos);

    // = EntityActionCallback overriding
    virtual void actionPlayed(const GameObjectInfo& entityInfo, uint32_t actionCode);

    // = StatsCallback overriding
    virtual void pointChanged(const GameObjectInfo& entityInfo,
        PointType pointType, uint32_t currentPoint);
    virtual void maxPointChanged(const GameObjectInfo& entityInfo,
        PointType pointType, uint32_t currentPoint, uint32_t maxPoint);
    virtual void creatureStatusChanged(EffectStatusType effectStatusType,
        int32_t currentValue);
    virtual void allAttributeChanged(const AttributeRates& attribute, bool isResist);
    virtual void pointsRestored(const GameObjectInfo& entityInfo);
    virtual void fullCreatureStatusChanged();
    virtual void creatureLifeStatsChanged(const GameObjectInfo& entityInfo,
        const LifeStats& lifeStats);
    virtual void chaoticUpdated(Chaotic chaotic);
    virtual void shieldCreated(const GameObjectInfo& entityInfo, PointType pointType, uint32_t value);
    virtual void shieldDestroyed(const GameObjectInfo& entityInfo, PointType pointType);
    virtual void shieldPointChanged(const GameObjectInfo& entityInfo, 
        PointType pointType, uint32_t currentPoint);

    // = ChaoCallback overriding
    virtual void changeChao(bool isChaoPlayer);
    virtual void changeTempChao(bool isChaoPlayer);
    virtual void chaoChanged(ObjectId playerId, bool isChaoPlayer);
    virtual void tempChaoChanged(ObjectId playerId, bool isChaoPlayer);

    // = EmotionCallback overriding
    virtual void emotionNotified(ObjectId playerId, const std::string& emotion);

    // = BotCommandCallback overriding
    virtual void commandMoved(const Position& position, bool shouldStopDestination);
    virtual void commandSkillCasted();

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

    // = rpc::CharacterInterestAreaRpc overriding
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

    // = rpc::EntityActionRpc overriding
    OVERRIDE_SRPC_METHOD_2(evActionPlayed,
        GameObjectInfo, entityInfo, uint32_t, actionCode);

    // = rpc::CreatureSocialRpc overriding
    OVERRIDE_SRPC_METHOD_1(say,
        ChatMessage, message);

    OVERRIDE_SRPC_METHOD_2(evCreatureSaid,
        Nickname, nickname, ChatMessage, message);

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

    // = rpc::ActionBarRpc overriding
    OVERRIDE_SRPC_METHOD_3(saveActionBar,
        ActionBarIndex, abiIndex, ActionBarPosition, abpIndex, DataCode, code);
    OVERRIDE_SRPC_METHOD_1(lockActionBar,
        bool, isLocked);

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
    OVERRIDE_SRPC_METHOD_1(evPointsRestored,
        GameObjectInfo, playerInfo);
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

    // = rpc::DungeonRpc overriding
    OVERRIDE_SRPC_METHOD_2(evChaoChanged,
        ObjectId, playerId, bool, isChao);
    OVERRIDE_SRPC_METHOD_2(evTempChaoChanged,
        ObjectId, playerId, bool, isChao);
    OVERRIDE_SRPC_METHOD_1(evChaoticUpdated,
        Chaotic, chaotic);

    // = rpc::DungeonRpc overriding
    OVERRIDE_SRPC_METHOD_1(enterDungeon,
        ObjectId, dungeonId);
    OVERRIDE_SRPC_METHOD_0(leaveDungeon);

    OVERRIDE_SRPC_METHOD_4(onEnterDungeon,
        ErrorCode, errorCode, WorldPosition, spawnPosition, std::string, mapData, PartyId, partyId);
    OVERRIDE_SRPC_METHOD_2(onLeaveDungeon,
        ErrorCode, errorCode, WorldPosition, spawnPosition);

    // = rpc::RewardRpc overriding
    OVERRIDE_SRPC_METHOD_1(evMoneyRewarded, 
        GameMoney, money);
    OVERRIDE_SRPC_METHOD_2(evCharacterClassChanged,
        ObjectId, playerId, CharacterClass, characterClass);
    OVERRIDE_SRPC_METHOD_2(evBotMovePosition,
        Position, position, bool, shouldStopDestination);
    OVERRIDE_SRPC_METHOD_0(evBotCastingSkill);

    // = rpc::EmotionRpc overriding
    OVERRIDE_SRPC_METHOD_1(notifyEmotion,
        std::string, emotion);

    OVERRIDE_SRPC_METHOD_2(evEmotionNotified,
        ObjectId, playerId, std::string, emotion);

    // = rpc::PropertyRpc overriding
    OVERRIDE_SRPC_METHOD_0(loadProperties);
    OVERRIDE_SRPC_METHOD_2(saveProperties,
        std::string, config, std::string, prefs);

    OVERRIDE_SRPC_METHOD_2(onLoadProperties,
        std::string, config, std::string, prefs);

private:
    ErrorCode fillBindRecall(BindRecallInfo& bindRecallInfo, ObjectId buildingId);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

private:
    ObjectId lastDungeonId_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
