#pragma once

#include "Creature.h"
#include "ability/VehicleMountable.h"
#include "ability/GliderMountable.h"
#include "ability/SkillCastable.h"
#include "ability/Rewardable.h"
#include "ability/Questable.h"
#include "ability/GraveStoneable.h"
#include "ability/Inventoryable.h"
#include "ability/Buildable.h"
#include "ability/Partyable.h"
#include "ability/Chaoable.h"
#include "ability/Guildable.h"
#include "ability/CastCheckable.h"
#include "ability/ItemCastable.h"
#include "ability/ArenaMatchable.h"
#include "ability/Moneyable.h"
#include "ability/Networkable.h"
#include "ability/Protectionable.h"
#include "ability/Cheatable.h"
#include "ability/Gliderable.h"
#include "ability/Vehicleable.h"
#include "ability/Teleportable.h"
#include "ability/CombatStateable.h"
#include "ability/Storeable.h"
#include "ability/SkillLearnable.h"
#include "ability/CharacterClassable.h"
#include "ability/FieldDuelable.h"
#include "ability/TargetSelectable.h"
#include "ability/WeaponUseable.h"
#include "ability/CharacterTitleable.h"
#include "ability/Achievementable.h"
#include "../../service/time/GameTimer.h"
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <gideon/cs/shared/data/UnionEntityInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/PlayerInfo.h>

namespace sne { namespace base {
class Session;
}} // namespace sne { namespace base {

namespace gideon { namespace zoneserver {
class PlayerInventories;
class Inventory;
class EquipInventory;
class BankAccount;
class PlayerState;
class QuestRepository;
class QuestInventory;
class AccessoryInventory;
class Arena;
class CheatCommander;
class AchievementRepository;

}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace gc {
class PlayerController;
class PlayerItemController;
class PlayerMoveController;
class PlayerQuestController;
class PlayerSkillController;
class PlayerInventoryController;
class PlayerGraveStoneController;
class PlayerAnchorController;
class PlayerTradeController;
class PlayerHarvestController;
class PlayerTreasureController;
class PlayerGuildController;
class PlayerNpcController;
class PlayerCastController;
class PlayerEffectController;
class PlayerMailController;
class PlayerAuctionController;
class PlayerArenaController;
class PlayerBuildingController;
class PlayerOutsideInventoryController;
class PlayerDeviceController;
class PlayerTeleportController;
class PlayerPartyController;
class PlayerDuelController;
class PlayerWorldEventController;
class PlayerAchievementController;
class PlayerCharacterTitleController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

class GuildSkillRepository;
class SkillCastableAbility;
class ItemCastableAbility;
class PassiveSkillManager;

/**
 * @struct TempChaoInfo
 */
struct TempChaoInfo
{
    Chaotic tempChaotic_;

    TempChaoInfo() :
        tempChaotic_(minChaotic) {}

    bool isChao() const;

    void hit();

    void upChaotic(Chaotic downChao);
    void downChaotic(Chaotic downChao);
    void setChaotic(Chaotic downChao);
};


/**
 * @class Player
 * 플레이어
 */
class ZoneServer_Export Player : public Creature,
    private Cheatable,
    private Chaoable,
    private SkillCastable,
    private Rewardable,
    private Questable,
    private Inventoryable,
    private GraveStoneable,
    private Partyable,
    private Guildable,
    private ItemCastable,
    private CastCheckable,
    private ArenaMatchable,
    private Moneyable,
    private Networkable,
    private Protectionable,
    private Teleportable,
    private Gliderable,
    private Vehicleable,
    private CombatStateable,
    private Storeable,
    private SkillLearnable,
    private CharacterClassable,
    private FieldDuelable,
    private TargetSelectable,
    private WeaponUseable,
    private CharacterTitleable,
    private Achievementable
{
    typedef Creature Parent;

    typedef sne::core::Set<InvenType> OutsideInvenOpenInfos;
    
public:
    enum { peaceTime = 10000 };
    Player(std::unique_ptr<gc::EntityController> controller);
    virtual ~Player();

    ErrorCode initialize(AccountId accountId, FullCharacterInfo& characterInfo,
        sne::base::Session* session, AccountGrade accountGrade);
    virtual void finalize();

    void logoutNow();

public:
    void setActionBarLock(bool isLocked);

    void loadProperties();
    void saveProperties(const std::string& config, const std::string& prefs);

public:
    void setTargetOfTarget(const GameObjectInfo& entityInfo);
    const GameObjectInfo& getSelectedTargetOfTarget() const;

public:
    void saveCharacterStats();
    ErrorCode saveActionBar(ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code);
 
public:
    bool canRequest() const {
        return isValid();
    }

    bool isValid() const;
    bool isInArenaMap() const;
    bool isSameTeamInArena(go::Entity& target) const;
    bool isFieldDueling() const;
    bool isMyDueler(go::Entity& target) const;
    bool isMyPartyMember(go::Entity& target) const;
    bool isMyGuildMember(go::Entity& target) const;

    
    ZoneId getZoneId() const {
        assert(characterInfo_ != nullptr);
        return characterInfo_->zoneId_;
    }

    const PlayerInfo& getPlayerInfo() const {
        return playerInfo_;
    }

    CheatCommander& getCheatCommander() {
        return *cheatCommander_.get();
    }

    void upAchievementPoint(AchievementPoint point) {
        characterInfo_->upAchievementPoint(point);
    }

    AchievementPoint getAchievementPoint() const {
        return characterInfo_->achievementPoint_;
    }

private:
    void reset();
    void initPlayerController();
    void initInventory(MoreCharacterInfo& moreCharacterInfo);
    void initBankAccount();
    void initQuestRepository();
    void initStatus(MoreCharacterInfo& moreCharacterInfo);
    bool initSkill();
    void initCharacterSkillManager();
    void initCooldown();
    void downChaotic_i(Chaotic chaotic);
    void upChaotic_i(Chaotic chaotic);

    void restoreSpeedAndScale();

    void handleExpiredTasks();

public:
    gc::PlayerTradeController& getPlayerTradeController() {
        return *tradeController_;
    }
    const gc::PlayerTradeController& getPlayerTradeController() const {
        return *tradeController_;
    }
    gc::PlayerItemController& getPlayerItemController() {
        return *itemController_;
    }
    const gc::PlayerItemController& getPlayerItemController() const {
        return *itemController_;
    }
    gc::PlayerHarvestController& getPlayerHarvestController() {
        return *harvestController_;
    }
    const gc::PlayerHarvestController& getPlayerHarvestController() const {
        return *harvestController_;
    }
    gc::PlayerTreasureController& getPlayerTreasureController() {
        return *treasureController_;
    }
    const gc::PlayerTreasureController& getPlayerTreasureController() const {
        return *treasureController_;
    }
    gc::PlayerMailController& getPlayerMailController() {
        return *mailController_;
    }
    const gc::PlayerMailController& getPlayerMailController() const {
        return *mailController_;
    }
    gc::PlayerAuctionController& getPlayerAuctionController() {
        return *auctionController_;
    }
    const gc::PlayerAuctionController& getPlayerAuctionController() const {
        return *auctionController_;
    }
    gc::PlayerNpcController& getPlayerNpcController() {
        return *npcController_;
    }
    const gc::PlayerNpcController& getPlayerNpcController() const {
        return *npcController_;
    }
    
    gc::PlayerDeviceController& getPlayerDeviceController() {
        return *deviceController_;
    }

    const gc::PlayerDeviceController& getPlayerDeviceController() const {
        return *deviceController_;
    }

    const gc::PlayerCastController& getPlayerCastController() const {
        return *castEventController_;
    }

    gc::PlayerCastController& getPlayerCastController() {
        return *castEventController_;
    }

    gc::PlayerSkillController& getPlayerSkillController();

    gc::PlayerMoveController& getPlayerMoveController();
    const gc::PlayerMoveController& getPlayerMoveController() const;

    gc::PlayerEffectController& getPlayerEffectController();
    const gc::PlayerEffectController& getPlayerEffectController() const;

    gc::PlayerAnchorController& getPlayerAnchorController() {
        return *anchorController_;
    }
    const gc::PlayerAnchorController& getPlayerAnchorController() const {
        return *anchorController_;
    }

    gc::PlayerBuildingController& getPlayerBuildingController() {
        return *buildingController_;
    }
    const gc::PlayerBuildingController& getPlayerBuildingController() const {
        return *buildingController_;
    }

    gc::PlayerOutsideInventoryController& getPlayerOutsideInventoryController() {
        return *outsideInventoryController_;
    }
    const gc::PlayerOutsideInventoryController& getPlayerOutsideInventoryController() const {
        return *outsideInventoryController_;
    }

    const gc::PlayerQuestController& getPlayerQuestController() const {
        return *questController_;
    }
    gc::PlayerQuestController& getPlayerQuestController() {
        return *questController_;
    }

    const gc::PlayerTeleportController& getPlayerTeleportController() const {
        return *teleportController_;
    }
    gc::PlayerTeleportController& getPlayerTeleportController() {
        return *teleportController_;
    }

    const gc::PlayerGraveStoneController& getPlayerGraveStoneController() const {
        return *graveStoneController_;
    }
    gc::PlayerGraveStoneController& getPlayerGraveStoneController() {
        return *graveStoneController_;
    }

    gc::PlayerArenaController& getPlayerArenaController() {
        return *arenaController_;
    }
    const gc::PlayerArenaController& getPlayerArenaController() const {
        return *arenaController_;
    }

    gc::PlayerGuildController& getPlayerGuildController() {
        return *guildController_;
    }
    const gc::PlayerGuildController& getPlayerGuildController() const {
        return *guildController_;
    }

    gc::PlayerInventoryController& getPlayerInventoryController() {
        return *inventoryController_;
    }
    const gc::PlayerInventoryController& getPlayerInventoryController() const {
        return *inventoryController_;
    }

    gc::PlayerPartyController& getPlayerPartyController() {
        return *partyController_;
    }
    const gc::PlayerPartyController& getPlayerPartyController() const {
        return *partyController_;
    }

    gc::PlayerDuelController& getPlayerDuelController() {
        return *duelController_;
    }
    const gc::PlayerDuelController& getPlayerDuelController() const {
        return *duelController_;
    }

    gc::PlayerWorldEventController& getPlayerWorldEventController() {
        return *worldEventController_;
    }
    const gc::PlayerWorldEventController& getPlayerWorldEventController() const {
        return *worldEventController_;
    }

    gc::PlayerAchievementController& getPlayerAchievementController() {
        return *achievementController_;
    }

    const gc::PlayerAchievementController& getPlayerAchievementController() const {
        return *achievementController_;
    }

    gc::PlayerCharacterTitleController& getPlayerCharacterTitleController() {
        return *characterTitleController_;
    }

    const gc::PlayerCharacterTitleController& getPlayerCharacterTitleController() const {
        return *characterTitleController_;
    }

private:
    virtual std::unique_ptr<gc::PlayerTradeController> createPlayerTradeController();
    virtual std::unique_ptr<gc::PlayerItemController> createPlayerItemController();
    virtual std::unique_ptr<gc::PlayerHarvestController> createPlayerHarvestController();
    virtual std::unique_ptr<gc::PlayerTreasureController> createPlayerTreasureController();
    virtual std::unique_ptr<gc::PlayerNpcController> createPlayerNpcController();
    virtual std::unique_ptr<gc::PlayerCastController> createPlayerCastController();
    virtual std::unique_ptr<gc::PlayerMailController> createPlayerMailController();
    virtual std::unique_ptr<gc::PlayerAuctionController> createPlayerAuctionController();
    virtual std::unique_ptr<gc::PlayerAnchorController> createPlayerAnchorController();
    virtual std::unique_ptr<gc::PlayerBuildingController> createPlayerBuildingController();
    virtual std::unique_ptr<gc::PlayerOutsideInventoryController> createPlayerOutsideInventoryController();
    virtual std::unique_ptr<gc::PlayerDeviceController> createPlayerDeviceController();
    virtual std::unique_ptr<gc::PlayerQuestController> createPlayerQuestController();
    virtual std::unique_ptr<gc::PlayerTeleportController> createPlayerTeleportController();
    virtual std::unique_ptr<gc::PlayerGraveStoneController> createPlayerGraveStoneController();
    virtual std::unique_ptr<gc::PlayerArenaController> createPlayerArenaController();
    virtual std::unique_ptr<gc::PlayerGuildController> createPlayerGuildController();
    virtual std::unique_ptr<gc::PlayerInventoryController> createPlayerInventoryController();
    virtual std::unique_ptr<gc::PlayerPartyController> createPlayerPartyController();
    virtual std::unique_ptr<gc::PlayerDuelController> createPlayerDuelController();
    virtual std::unique_ptr<gc::PlayerWorldEventController> createPlayerWorldEventController();
    virtual std::unique_ptr<gc::PlayerAchievementController> createPlayerAchievementController();
    virtual std::unique_ptr<gc::PlayerCharacterTitleController> createPlayerCharacterTitleController();

private: // = Creature overriding
    virtual void initState(CreatureStateInfo& stateInfo);
    virtual void frenze(int32_t size, bool isRelease);
    virtual float32_t getCurrentDefaultSpeed() const;
    virtual CreatureLevel getCreatureLevel() const {
        assert(characterInfo_ != nullptr);
        return characterInfo_->currentLevel_.level_;
    }
    const DebuffBuffEffectInfoSet getDebuffBuffEffectInfoSet() const;

    AccountId getAccountId() const {
        return accountId_;
    }	

    virtual bips_t getMissChance(const Entity& target) const;
    virtual bips_t getDodgeChance() const;
    virtual bips_t getParryChance() const;
    virtual bips_t getBlockChance() const;
    virtual bips_t getPhysicalCriticalChance() const;
    virtual bips_t getMagicCriticalChance() const;

private: // = Entity overriding
    virtual ErrorCode spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition);
    virtual ErrorCode respawn(WorldMap& worldMap);
    virtual ErrorCode despawn();
    virtual ObjectPosition getNextSpawnPosition() const;
    virtual const Nickname& getNickname() const {
        return characterInfo_->nickname_;
    }

    virtual std::unique_ptr<EffectHelper> createEffectHelper();

public: // = EntityAbility overriding
    virtual SkillCastable* querySkillCastable() { return this; }
    virtual ItemCastable* queryItemCastable() { return this; }
    virtual Rewardable* queryRewardable() { return this; }
    virtual Inventoryable* queryInventoryable() { return this; }
    virtual Questable* queryQuestable() { return this; }
    virtual const Questable* queryQuestable() const { return this; }
    virtual Partyable* queryPartyable() { return this; }
    virtual Chaoable* queryChaoable() { return this; }
    virtual const Chaoable* queryChaoable() const { return this; }
    virtual GraveStoneable* queryGraveStoneable() { return this; }
    virtual CastCheckable* queryCastCheckable() { return this; }
    virtual ArenaMatchable* queryArenaMatchable() { return this; }
    virtual Moneyable* queryMoneyable() { return this; }
    virtual Guildable* queryGuildable() { return this; }
    virtual Protectionable* queryProtectionable() { return this; }
    virtual const Protectionable* queryProtectionable() const { return this; }
    virtual Networkable* queryNetworkable() { return this; }
    virtual Cheatable* queryCheatable() { return this; }
    virtual Gliderable* queryGliderable() { return this; }
    virtual Vehicleable* queryVehicleable() { return this; }
    virtual Teleportable* queryTeleportable() { return this; }
    virtual CombatStateable* queryCombatStateable() { return this; }
    virtual Storeable* queryStoreable() { return this; }
    virtual SkillLearnable* querySkillLearnable() { return this; }
    virtual CharacterClassable* queryCharacterClassable() { return this; }
    virtual FieldDuelable* queryFieldDuelable() { return this; }
    virtual TargetSelectable* queryTargetSelectable() { return this; }
    virtual const WeaponUseable* queryWeaponUseable() const {
        return this;
    }
    virtual CharacterTitleable* queryCharacterTitleable() {
        return this;
    }
    virtual Achievementable* queryAchievementable() {
        return this;
    }

    virtual MoveState* queryMoveState();
    virtual CreatureState* queryCreatureState();
    virtual SkillCasterState* querySkillCasterState();
    virtual TradeState* queryTradeState();
    virtual ItemManageState* queryItemManageState();
    virtual HarvestState* queryHarvestState();
    virtual TreasureState* queryTreasureState();
    virtual GliderState* queryGliderState();
    virtual VehicleState* queryVehicleState();
    virtual ChaoState* queryChaoState();
    virtual CastState* queryCastState();
    virtual CombatState* queryCombatState();
    virtual FieldDuelState* queryFieldDuelState();
    virtual const FieldDuelState* queryFieldDuelState() const;
    
private:
    virtual void initializeCharacterTitle(ErrorCode errorCode, const CharacterTitleCodeSet& titleCodeSet);
    virtual ErrorCode selectCharacterTitle(CharacterTitleCode titleCode);
    
private: // = CharacterClassable overriding
    virtual CharacterClass getCharacterClass() const {
        return characterInfo_->characterClass_;
    }    
    virtual void changeCharacterClass(CharacterClass characterClass);

private: // = Achievementable overriding
    virtual void initializeAchievement(ErrorCode errorCode, const ProcessAchievementInfoMap& processInfoMap,
        const CompleteAchievementInfoMap& completeInfoMap);
    virtual void updateAchievement(AchievementMissionType missionType, Entity* target);
    virtual void updateAchievement(AchievementMissionType missionType, uint32_t param1, uint32_t param2);

private: // = SkillLearnable overriding
    virtual ErrorCode learnSkill(SkillTableType skillTableType, SkillIndex index);
    virtual ErrorCode unlearnSkill(SkillTableType skillTableType, SkillIndex index);
    virtual ErrorCode learnSkills(const SkillCodes& skillCodes);
    virtual ErrorCode resetLearnedSkills();

    virtual SkillPoint getSkillPoint() const {
        return characterInfo_->sp_;
    }
    void addSkill(SkillCode skillCode);
    void removeSkill(SkillTableType skillTableType, SkillIndex index);
    ErrorCode checkLearnSkills(SkillPoint& needSkillPoint, const SkillCodes& skillCodes);

private: // = Storeable overriding
    virtual void sellItem(uint64_t& currentValue, uint32_t& buyBackIndex, BuyBackItemInfo& buyBackInfo, ObjectId itemId, uint8_t itemCount,
        CostType ct, uint32_t totalValue);

    virtual ErrorCode buyItem(const BaseItemInfo& itemInfo, uint64_t& currentValue,
        CostType ct, uint32_t totalValue);
    virtual ErrorCode buyBackItem(CostType& ct, uint64_t& currentValue, uint32_t index);
    virtual ErrorCode repairGlider(ObjectId gliderId);
    virtual const BuyBackItemInfoMap queryBuyBackItemInfoMap() const;

private: // = CombatStateable overriding
    virtual bool changeCombatState(bool isCombatState);

    virtual bool updateAttackTime(GameTime cooltime = 0);
    virtual void updateLockPeaceTime(GameTime delayTime);

private: // = Vehicleable overriding
    virtual VehicleCode getVehicleCode() const;
    virtual EntityVehicleInfo getEntityVehicle() const;

    virtual void addVehicle(const VehicleInfo& info);

    virtual ErrorCode selectVehicle(ObjectId selectId);

private: // = Gliderble overriding
    virtual GliderCode getGliderCode() const;
    virtual uint32_t getGliderDurability() const;

    virtual void addGlider(const GliderInfo& info);
    virtual void decreaseGliderDurability(uint32_t value);

    virtual ErrorCode selectGlider(ObjectId selectId);

private: // = Teleportable overriding
    virtual const Nickname& getCallee() const {
        return callee_;
    }	
    virtual const WorldPosition& getRecallPosition() const {
        return recallPosition_;
    }
    virtual const BindRecallInfo* getBindRecallInfo(ObjectId linkId) const;

    virtual void setRecallPosition(const WorldPosition& worldPosition) {
        recallPosition_ = worldPosition;
    }
    virtual void setCallee(const Nickname& nickname) {
        callee_ = nickname;
    }
    virtual void removeBindRecall(ObjectId linkId);

    virtual ErrorCode addBindRecall(const BindRecallInfo& info);
    
private:
    virtual void setFieldDuel(FieldDualPtr ptr = FieldDualPtr());

    virtual FieldDualPtr getFieldDualPtr();

private:
    // networkable
    virtual void logout();

private: // = Protectionable
    virtual bool isBeginnerProtection() const {
        return characterInfo_->isBeginnerProtection_;
    }

    virtual void releaseBeginnerProtection(bool shouldNotify);

private: // = Chaoable overriding
    Chaotic getChaotic() const {
        return characterInfo_->chaotic_;
    }
    virtual bool isChao() const;
    virtual bool isTempChao() const;
    virtual bool isRealChao() const;
    virtual void hitToPlayer();
    virtual void standGraveStone();
    virtual void downChaotic(Chaotic chaotic);
    virtual void upChaotic(Chaotic chaotic);

private: // = Inventoryable overriding
    virtual Inventory& getInventory() const;
    virtual Inventory& getVehicleInventory() const;
    virtual EquipInventory& getEquipInventory() const;
    virtual AccessoryInventory& getAccessoryInventory() const;
    virtual QuestInventory& getQuestInventory() const;
    virtual BankAccount& getBankAccount() const {
        return *bankAccount_;
    }
    virtual AccessoryIndex getAccessoryIndex(ObjectId itemId) const;
    virtual ObjectId getEquipedItemId(EquipPart part) const {
        return characterInfo_->equipments_[part];
    }

    virtual InventoryInfo getInventoryInfo() const;

    virtual bool canExtendSlot(bool isCashSlot, uint8_t extendCount) const;
    virtual bool isOpenOutsideInventory(InvenType invenType) const;
    virtual bool isEquipAccessory(AccessoryIndex index) const;

    virtual uint8_t extendSlot(bool isCashSlot, uint8_t extendCount);
    virtual void robbed(LootInvenItemInfoMap& infoMap);

    virtual void openOutsideInventory(InvenType invenType);
    virtual void closeOutsideInventory(InvenType invenType);
    virtual void setAccessoryIndex(AccessoryIndex index, ObjectId itemId);
    
private: // = Questable overriding
    virtual QuestRepository& getQuestRepository() {
        return *questRepository_.get();
    }
    virtual const QuestRepository& getQuestRepository() const {
        return *questRepository_.get();
    }
    virtual void killed(go::Entity& victim);
    virtual ErrorCode canAddItem(QuestCode& questCode, QuestItemCode questItemCode) const;

private: // = ArenaMatchable overriding
    virtual Arena* getArena() {
        return arena_;
    }

    virtual ArenaPoint getArenaPoint() const {
        return characterInfo_->arenaPoint_;
    }

    virtual CharacterArenaPlayResults& getCharacterArenaPlayResults() {
        return characterInfo_->arenaPlayResults_;
    }

    virtual ErrorCode reviveInArena(const ObjectPosition& position);

    virtual void setArena(Arena* arena);
    virtual void upArenaPoint(ArenaPoint arenaPoint);
    virtual void downArenaPoint(ArenaPoint arenaPoint);
    virtual void setArenaPoint(ArenaPoint arenaPoint);

private: // = Partyable overriding
    virtual bool isPartyMember() const;
    virtual bool isSameParty(PartyId partyId) const;
    virtual void setParty(PartyPtr party);    
    virtual PartyPtr getParty();
    virtual const PartyPtr getParty() const;
    
private: // = Moneyable oervrding
    virtual GameMoney getGameMoney() const {
        return characterInfo_->gameMoney_;
    }

    virtual ForgeCoin getForgeCoin() const {
        return characterInfo_->forgeCoin_;
    }

    virtual GameMoney getBankGameMoney() const {
        return characterInfo_->bankAccountInfo_.gameMoney_;
    }

    virtual EventCoin getEventCoin() const {
        return characterInfo_->eventCoin_;
    }

    virtual void upGameMoney(GameMoney gameMoney);
    virtual void downGameMoney(GameMoney gameMoney);
    virtual void setGameMoney(GameMoney gameMoney);

    virtual void upForgeCoin(ForgeCoin forgeCoin);
    virtual void downForgeCoin(ForgeCoin forgeCoin);
    virtual void setForgeCoin(ForgeCoin forgeCoin);

    virtual void upEventCoin(EventCoin eventCoin);
    virtual void downEventCoin(EventCoin eventCoin);
    virtual void setEventCoin(EventCoin eventCoin);

private: // = Positionable overriding
    virtual void setPosition(const ObjectPosition& position);
    virtual void setHeading(Heading heading);

    // = Moveable overriding
    virtual std::unique_ptr<gc::MoveController> createMoveController();
    virtual void reserveReleaseDownSpeed() {
         shouldReleaseDownSpeedScript_ = true;        
    }
    virtual void reserveReleaseUpSpeed() {
        shouldReleaseUpSpeedScript_ = true;        
    }

 private: // = Liveable overriding
    virtual std::unique_ptr<CreatureStatus> createCreatureStatus();
    virtual ErrorCode revive(bool skipTimeCheck = false);
    virtual bool reviveByEffect(HitPoint& refillPoint, permil_t perRefillHp);

private: // = SkillCastable overriding
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, SkillCode skillCode);
    virtual ErrorCode castAt(const Position& targetPosition, SkillCode skillCode);
    virtual void cancel(SkillCode skillCode);
    virtual void cancelConcentrationSkill(SkillCode skillCode);
    virtual void cancelAll();
    virtual void consumePoints(const Points& points);
    virtual void consumeMaterialItem(const BaseItemInfo& itemInfo);
    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const GameObjectInfo& targetInfo) const;
    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const Position& targetPosition) const;
    virtual float32_t getLongestSkillDistance() const;
    virtual bool isUsing(SkillCode skillCode) const;
    virtual bool canCast(SkillCode skillCode) const;

private: // = CastCheckable overriding
    virtual bool checkCastableNeedSkill(SkillCode needSkillCode) const;
    virtual bool checkCastableUsableState(SkillUseableState needUsableState) const;
    virtual bool checkCastableEquip(EquipPart checekEquipPart, SkillCastableEquipType checkAllowedType) const;
    virtual bool checkCastableNeedItem(const BaseItemInfo& needItem) const;
    virtual bool checkCastableEffectCategory(EffectStackCategory category) const;
    virtual ErrorCode checkCastablePoints(PointType pt, bool isPercent,
        bool isCheckUp, uint32_t checkValue) const;

private: // = PassiveSkillCastable overriding
    virtual void notifyChangeCondition(PassiveCheckCondition condition);
    virtual PassiveSkillManager& getPassiveSkillManager() {
        return *passiveSkillManager_.get();
    }

private: // = Rewardable overriding
    virtual ExpPoint getRewardExpPoint() const;

    virtual void expReward(bool& isLevelUp, bool& isMajorLevelUp, ExpPoint rewardExp);
    virtual ErrorCode addRewardSkill(SkillCode skillCode);
    virtual ErrorCode addCharacterTitle(CharacterTitleCode characterTitleCode);

private: // = ItemCastable overriding
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, ObjectId objectId);
    virtual ErrorCode castAt(const Position& targetPosition, ObjectId objectId);
    virtual void insertCastItem(ObjectId itemId, DataCode dataCode);
    virtual void removeCastItem(ObjectId itemId);
    virtual ErrorCode checkItemCasting(ObjectId itemId,
        const GameObjectInfo& targetInfo) const;
    virtual ErrorCode checkItemCasting(ObjectId itemId,
        const Position& targetPosition) const;
    virtual void cancel(ObjectId objectId);
    virtual bool checkQuestItem(ObjectId objectId) const;
    virtual void useQuestItem(ObjectId objectId);
    virtual bool checkElementItem(ObjectId objectId) const;
    virtual void useElementItem(ObjectId objectId);

private: // = Tickable overriding
    virtual void tick(GameTime diff);

private: // = Skillable overriding
    virtual std::unique_ptr<gc::SkillController> createSkillController();

private: // = Effectable overriding
    virtual std::unique_ptr<gc::CreatureEffectController> createEffectController();

private: // = Guildable overriding
    virtual void setGuildInfo(const BaseGuildInfo& guildInfo);

    virtual GuildId getGuildId() const;
    virtual int32_t getGuildEffectValue(EffectScriptType type) const;
    virtual void activateSkill(SkillCode skillCode);
    virtual void deactivateSkills();

private: // = TargetSelectable overriding
    virtual const Entity* selectTarget(const GameObjectInfo& targetInfo);
    virtual void unselectTarget();
    virtual Entity* getSelectedTarget();
    virtual const GameObjectInfo& getSelectedTargetInfo() const;

private: // = Cheatable overriding
    virtual ErrorCode updateCheatGameMoney(ValueChangetype type, GameMoney value);
    virtual ErrorCode updateCheatArenaPoint(ValueChangetype type, ArenaPoint value);
    virtual ErrorCode updateCheatEventCoin(ValueChangetype type, EventCoin value);
    virtual ErrorCode updateCheatForgeCoin(ValueChangetype type, ForgeCoin value);
    virtual ErrorCode updateCheatExp(ValueChangetype type, ExpPoint value);
    virtual ErrorCode updateCheatSpeed(ValueChangetype type, float32_t value);
    virtual ErrorCode updateCheatLevel(ValueChangetype type, bool isMajorLevel, uint8_t value);
    virtual ErrorCode updateCheatSkillPoint(ValueChangetype type, SkillPoint value);
    virtual ErrorCode updateCheatPoint(ValueChangetype type, PointType pointType, uint32_t value);
    virtual ErrorCode updateChao(ValueChangetype type, bool isTempChao, Chaotic value);

    virtual ErrorCode setPlayerStatus(CheatValueType valueType, int32_t value);
    virtual ErrorCode setZeroAllCurrentStatus();
    virtual ErrorCode restoreCurrentStatus();

    virtual ErrorCode addCheatItem(DataCode itemCode, uint8_t itemCount);
    virtual ErrorCode addCheatSkill(SkillCode skillCode);
    virtual ErrorCode addCheatQuest(QuestCode questCode);
    virtual ErrorCode addCheatCharacterTitle(CharacterTitleCode titleCode);

    virtual ErrorCode removeCheatQuest(QuestCode questCode);

    virtual ErrorCode resetCheatCoolTime();

    virtual ErrorCode toCheatPlayer(Entity& target);
    virtual ErrorCode toCheatMe(Entity& target);
    virtual ErrorCode toCheatPosition(const Position& target);

    virtual ErrorCode whos();

    virtual ErrorCode refillCheatPoints();

    virtual ErrorCode clearAcceptQuest(QuestCode questCode);

private: // = WeaponUseable overriding
    virtual bool isMeleeWeaponEquipped() const;
    virtual bool isShieldEquipped() const;

private:
    sne::base::Session* session_;

    AccountId accountId_;
    AccountGrade accountGrade_;
    FullCharacterInfo* characterInfo_;
    PlayerInfo playerInfo_;
    GameTime lastSaveStatsTime_;

    std::unique_ptr<PlayerInventories> inventory_;
    std::unique_ptr<QuestInventory> questInventory_;
    std::unique_ptr<BankAccount> bankAccount_;
    std::unique_ptr<Inventory> vehicleInventory_;
    std::unique_ptr<QuestRepository> questRepository_;

    std::unique_ptr<SkillCastableAbility> skillCastableAbility_;
    std::unique_ptr<ItemCastableAbility> itemCastableAbility_;
    std::unique_ptr<CharacterSkillManager> characterSkillManager_;
    std::unique_ptr<PassiveSkillManager> passiveSkillManager_;
    std::unique_ptr<AchievementRepository> achievementRepository_;
    std::unique_ptr<PlayerState> playerState_;
    std::unique_ptr<CheatCommander> cheatCommander_;
    std::unique_ptr<GuildSkillRepository> guildSkillReposity_;

    std::unique_ptr<gc::PlayerTradeController> tradeController_;
    std::unique_ptr<gc::PlayerItemController> itemController_;
    std::unique_ptr<gc::PlayerInventoryController> inventoryController_;
    std::unique_ptr<gc::PlayerQuestController> questController_;
    std::unique_ptr<gc::PlayerHarvestController> harvestController_;
    std::unique_ptr<gc::PlayerTreasureController> treasureController_;
    std::unique_ptr<gc::PlayerGraveStoneController> graveStoneController_;
    std::unique_ptr<gc::PlayerPartyController> partyController_;
    std::unique_ptr<gc::PlayerAnchorController> anchorController_;
    std::unique_ptr<gc::PlayerGuildController> guildController_;
    std::unique_ptr<gc::PlayerNpcController> npcController_;
    std::unique_ptr<gc::PlayerCastController> castEventController_;
    std::unique_ptr<gc::PlayerMailController> mailController_;
    std::unique_ptr<gc::PlayerAuctionController> auctionController_;
    std::unique_ptr<gc::PlayerArenaController> arenaController_;
    std::unique_ptr<gc::PlayerBuildingController> buildingController_;
    std::unique_ptr<gc::PlayerOutsideInventoryController> outsideInventoryController_;
    std::unique_ptr<gc::PlayerDeviceController> deviceController_;
    std::unique_ptr<gc::PlayerTeleportController> teleportController_;
    std::unique_ptr<gc::PlayerDuelController> duelController_;
    std::unique_ptr<gc::PlayerWorldEventController> worldEventController_;
    std::unique_ptr<gc::PlayerAchievementController> achievementController_;
    std::unique_ptr<gc::PlayerCharacterTitleController> characterTitleController_;
    
    FieldDualPtr fieldDuelPtr_;
    GameTime updateChaobleTime_; // 이것보다 시간이 많아야 카오가 된다

    HitPoint duelHitPoint_;
    ManaPoint duelManaPoint_;

    PartyPtr party_;

    TempChaoInfo tempChaoInfo_;
    float32_t originalSpeed_;
        
    Arena* arena_;
    OutsideInvenOpenInfos outsideInvenOpenInfos_;

    GameObjectInfo selectedTargetInfo_;
    GameObjectInfo selectedTargetOfTargetInfo_;
 
    GameTime combatExpireTime_; // 스킬을 쓰거나 적용된시간
    GameTime releaseLockPeaceTime_;
    GameTimeTracker oneSecondTracker_;
    
    Nickname callee_;
    WorldPosition recallPosition_;

    bool shouldReleaseDownSpeedScript_;
    bool shouldReleaseUpSpeedScript_;

    CharacterTitleCodeSet characterTitleCodeSet_;    
    BuyBackItemInfoMap buyBackItemMap_;
    uint32_t buyBackIndex_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
