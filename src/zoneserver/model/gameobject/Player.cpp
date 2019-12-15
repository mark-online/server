#include "ZoneServerPCH.h"
#include "Player.h"
#include "GraveStone.h"
#include "status/PlayerStatus.h"
#include "../achievement/AchievementRepository.h"
#include "../state/impl/PlayerState.h"
#include "../quest/QuestRepository.h"
#include "ability/impl/SkillCastableAbility.h"
#include "ability/impl/ItemCastableAbility.h"
#include "ability/TargetSelectable.h"
#include "skilleffect/PassiveSkillManager.h"
#include "skilleffect/CreatureEffectScriptApplier.h"
#include "skilleffect/GuildSkillRepository.h"
#include "../time/CoolDownTimer.h"
#include "../item/PlayerInventories.h"
#include "../item/QuestInventory.h"
#include "../bank/BankAccount.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../controller/PlayerController.h"
#include "../../controller/player/PlayerEffectController.h"
#include "../../controller/player/PlayerMailController.h"
#include "../../controller/player/PlayerAuctionController.h"
#include "../../controller/player/PlayerArenaController.h"
#include "../../controller/player/PlayerItemController.h"
#include "../../controller/player/PlayerSkillController.h"
#include "../../controller/player/PlayerEffectController.h"
#include "../../controller/player/PlayerMoveController.h"
#include "../../controller/player/PlayerTradeController.h"
#include "../../controller/player/PlayerQuestController.h"
#include "../../controller/player/PlayerInventoryController.h"
#include "../../controller/player/PlayerGuildController.h"
#include "../../controller/player/PlayerHarvestController.h"
#include "../../controller/player/PlayerTreasureController.h"
#include "../../controller/player/PlayerGraveStoneController.h"
#include "../../controller/player/PlayerPartyController.h"
#include "../../controller/player/PlayerAnchorController.h"
#include "../../controller/player/PlayerNpcController.h"
#include "../../controller/player/PlayerBuildingController.h"
#include "../../controller/player/PlayerOutsideInventoryController.h"
#include "../../controller/player/PlayerCastController.h"
#include "../../controller/player/PlayerDeviceController.h"
#include "../../controller/player/PlayerTeleportController.h"
#include "../../controller/player/PlayerDuelController.h"
#include "../../controller/player/PlayerWorldEventController.h"
#include "../../controller/player/PlayerAchievementController.h"
#include "../../controller/player/PlayerCharacterTitleController.h"
#include "../../controller/GraveStoneController.h"
#include "../../controller/MoveController.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../controller/callback/EntityEvents.h"
#include "../../service/time/GameTimer.h"
#include "../../service/item/ItemOptionService.h"
#include "../../service/cheat/CheatCommander.h"
#include "../../service/mail/MailService.h"
#include "../../service/party/Party.h"
#include "../../service/party/PartyService.h"
#include "../../service/guild/GuildService.h"
#include "../../service/arena/mode/Arena.h"
#include "../../service/skill/helper/PlayerEffectHelper.h"
#include "../../service/teleport/TeleportService.h"
#include "../../service/movement/MovementManager.h"
#include "../../service/world_event/WorldEventService.h"
#include "../../service/cheat/CheatCommander.h"
#include "../../helper/InventoryHelper.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "../../s2s/ZoneArenaServerProxy.h"
#include "../../user/ZoneUserManager.h"
#include "../../helper/Utils.h"
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/ExpTable.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/PlayerActiveSkillTable.h>
#include <gideon/cs/datatable/PlayerPassiveSkillTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/QuestItemTable.h>
#include <gideon/cs/datatable/VehicleTable.h>
#include <gideon/cs/datatable/GliderTable.h>
#include <gideon/cs/datatable/CharacterStatusTable.h>
#include <gideon/cs/datatable/CharacterDefaultSkillTable.h>
#include <gideon/cs/shared/data/CombatRatingInfo.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace go {

namespace {

inline bool hasNeedSkill(CharacterSkillManager& skillManager, SkillCode needSkillCode)
{
    if (isValidSkillCode(needSkillCode)) {
        //SkillIndex needIndex = getSkillIndex(needSkillCode);

        SkillCode skillCode = invalidSkillCode;
        {
            SkillIndex index = getSkillIndex(needSkillCode);
            SkillTableType tableType = getSkillTableType(needSkillCode);

            skillCode = skillManager.getCurrentSkillCode(tableType, index);
        }
        if (! isValidSkillCode(skillCode)) {
            return false;
        }

        const SkillLevel needLevel = getSkillLevel(needSkillCode);
        if (needLevel > getSkillLevel(skillCode)) {
            return false;
        }   
    }

    return true;
}


inline bool fillSkillLearnInfo(datatable::SkillLearnInfo& learnInfo, SkillCode skillCode)
{   
    SkillTableType skillTableType = getSkillTableType(skillCode);
    if (skillTableType == sttActivePlayer) {
        const datatable::PlayerActiveSkillTemplate* skillTemplate = PLAYER_ACTIVE_SKILL_TABLE->getPlayerSkill(skillCode);
        if (skillTemplate) {
            learnInfo = skillTemplate->learnInfo_;
            return true;
        }
    }
    else if (skillTableType == sttPassivePlayer) {
        const datatable::PassiveSkillTemplate* skillTemplate = PLAYER_PASSIVE_SKILL_TABLE->getPlayerSkill(skillCode);
        if (skillTemplate) {
            learnInfo = skillTemplate->learnInfo_;
            return true;
        }        
    }

    return false;
}

/**
 * @class PlayerLogoutTask
 */
class LogoutTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<LogoutTask>
{
public:
    LogoutTask(go::Player& player) :
        player_(player) {}

private:
    virtual void run() {
        player_.logoutNow();
    }

private:
    go::Player& player_;
};


/**
 * @class LoadCharacterPropertiesRequestFutureObserver
 */
class LoadCharacterPropertiesRequestFutureObserver : public sne::base::FutureObserver,
    public sne::core::ThreadSafeMemoryPoolMixin<LoadCharacterPropertiesRequestFutureObserver>
{
public:
    LoadCharacterPropertiesRequestFutureObserver(gc::PlayerController& controller) :
        controller_(controller) {}

private:
    virtual void update(const sne::base::Future& future) {
        const serverbase::LoadCharacterPropertiesRequestFuture& requestFuture =
            static_cast<const serverbase::LoadCharacterPropertiesRequestFuture&>(future);

        assert(controller_.getOwnerAs<go::Player>().getUnionEntityInfo().getObjectId() == requestFuture.characterId_);
        controller_.onLoadProperties(requestFuture.config_, requestFuture.prefs_);
    }

    virtual void deleteFutureObserver() {
        delete this;
    }

private:
    gc::PlayerController& controller_;
};


/**
 * @class GuildOffineEvent
 */
class GuildOffineEvent : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<GuildOffineEvent>
{
public:
    GuildOffineEvent(GuildId guildId, ObjectId characterId) :
        guildId_(guildId),
        characterId_(characterId) {}

private:
    virtual void run() {
        GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId_);
        if (guild.get() != nullptr) {
            guild->offline(characterId_);
        }
    }

private:
    GuildId guildId_;
    ObjectId characterId_;
};


/**
 * @class LeaveArenaEvent
 */
class LeaveArenaEvent : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<LeaveArenaEvent>
{
public:
    LeaveArenaEvent(Arena& arena, AccountId accountId, ObjectId characterId) :
        arena_(arena),
        accountId_(accountId),
        characterId_(characterId) {}

private:
    virtual void run() {
        arena_.leave(accountId_, characterId_);        
    }

private:
    Arena& arena_;
    AccountId accountId_;
    ObjectId characterId_;
};


/**
 * @class ReleaseSyncMailEvent
 */
class ReleaseSyncMailEvent : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ReleaseSyncMailEvent>
{
public:
    ReleaseSyncMailEvent(ObjectId characterId) :
        characterId_(characterId) {}

private:
    virtual void run() {
        MAIL_SERVICE->releaseSyncMail(characterId_);
    }
private:
    ObjectId characterId_;
};


bool isBindRecallItem(DataCode itemCode)
{
    if (! isValidDataCode(itemCode)) {
        return false;
    }
    return false;
}


bool shouldApplyChao(const go::Entity& target)
{
    if (ZONE_SERVICE->isArenaServer()) {
        return false;
    }


    if (target.isPlayer()) {
        return target.queryChaoable()->isChao();
    }
    else if (target.isNpc()) {
        return ! isCritter(getNpcType(target.getEntityCode()));
    }

    return false;
}

    
inline bool isCastableSkillFromEquip(EquipPart checkEquipPart, SkillCastableEquipType checkAllowedType, 
    const EquipPartTypeMap& equipPartTypeMap)
{
    bool isSuceessCheckEquipAllowed = false;
    bool isSuceessCheckEquipPart = false;

    if (checkAllowedType == scetAny) {
        isSuceessCheckEquipAllowed = true;
    }
    if (checkEquipPart == epInvalid) {
        isSuceessCheckEquipPart = true;
    }

    if (! isSuceessCheckEquipPart) {
        if (equipPartTypeMap.find(checkEquipPart) == equipPartTypeMap.end()) {
            return false;
        }
        isSuceessCheckEquipPart = true;
    }

    if (isSuceessCheckEquipAllowed && isSuceessCheckEquipPart) {
        return true;
    }    

    for (const EquipPartTypeMap::value_type& value : equipPartTypeMap) {   
        EquipType equipType = value.second;
        if (! isSuceessCheckEquipAllowed) {
            if (isSkillCastableEquipType(checkAllowedType, equipType)) {
                isSuceessCheckEquipAllowed = true;
            }
        }

        if (isSuceessCheckEquipAllowed && isSuceessCheckEquipPart) {
            return true;
        }    
    }
    
    return false;
}


inline bool checkUseableState(SkillUseableState skillUsableState,
    CreatureStateType characterState)
{
    if (susAlaways == skillUsableState) {
        return true;   
    }
    if (susNotCast == skillUsableState) {
        return false;
    }

    CreatureStateInfo stateInfo;
    stateInfo.states_ = characterState;

    if (susCombat == skillUsableState) {
        return stateInfo.hasState(cstCombat);
    }
    else if (susPeace == skillUsableState) {
        return ! stateInfo.hasState(cstCombat);
    }
    else if (susHidden == skillUsableState) {
        return stateInfo.hasState(cstHide);
    }
    return false;
}


inline bool fillEquipCode(CreatureEquipments& equipments,
    const FullCharacterInfo& characterInfo)
{
    for (int i = epFirst; i <= epEnd; ++i) {
        const ObjectId itemId = characterInfo.equipments_[i];
        if (! isValidObjectId(itemId)) {
            continue;
        }

        const ItemInfo* itemInfo = characterInfo.inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            return false;
        }

        equipments[i] = itemInfo->itemCode_;
    }
    return true;
}

} // namespace

// = TempChaoInfo

bool TempChaoInfo::isChao() const
{    
    return tempChaotic_ > 0;
}


void TempChaoInfo::hit()
{
    if (tempChaotic_ == 0) {
        tempChaotic_ = toChaotic(10);
    }
    else if (tempChaotic_ > 0) {
        tempChaotic_ = toChaotic(tempChaotic_ + 30);
        const Chaotic maxTempChaotic = toChaotic(500);
        if (tempChaotic_ > maxTempChaotic) {
            tempChaotic_ = maxTempChaotic;
        }
    }
}

void TempChaoInfo::upChaotic(Chaotic upChao)
{
    tempChaotic_ = toChaotic(tempChaotic_ + upChao);	
}



void TempChaoInfo::downChaotic(Chaotic downChao)
{
    if (tempChaotic_ > downChao) {
        tempChaotic_ = toChaotic(tempChaotic_ - downChao);
    }
    else {
        tempChaotic_ = minChaotic;
    }
}


void TempChaoInfo::setChaotic(Chaotic upChao)
{
    tempChaotic_ = toChaotic(upChao);	
}

// = Player

Player::Player(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller)),
    session_(nullptr),
    accountId_(invalidAccountId),
    characterInfo_(nullptr),
    accountGrade_(agPlayer),
    lastSaveStatsTime_(0),
    arena_(nullptr),
    combatExpireTime_(0),
    releaseLockPeaceTime_(0),
    buyBackIndex_(0)
{
}


Player::~Player()
{
}


ErrorCode Player::initialize(AccountId accountId, FullCharacterInfo& characterInfo,
    sne::base::Session* session, AccountGrade accountGrade)
{
    assert(isValidAccountId(accountId));
    assert(characterInfo.isValid());

    // TODO: DB에서 읽어올 것(& 캐릭터 생성시 DB 저장)
    const FactionCode factionCode = playerFactionCode;

    GuildMarkCode guildMarkCode = invalidGuildMarkCode;    
    SkillCodes skillCodes;
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(characterInfo.guildId_);
    if (guild.get() != nullptr) {
        guild->online(*this);
        guildMarkCode = guild->getGuildMarkCode();
        skillCodes = guild->getGuildSkillCodes();
    } 

    {
        std::lock_guard<LockType> lock(getLock());

        accountId_ = accountId;
        accountGrade_ = accountGrade;
        characterInfo_ = &characterInfo;
        selectedTargetInfo_.reset();
        selectedTargetOfTargetInfo_.reset();


        if (!Parent::initialize(otPc, characterInfo.objectId_, factionCode)) {
            return ecCharacterInvalid;
        }

        lastSaveStatsTime_ = GAME_TIMER->msec();
        playerInfo_ = PlayerInfo(accountId_, characterInfo_->objectId_, characterInfo_->nickname_,
            characterInfo_->guildId_, characterInfo_->characterClass_, characterInfo_->currentLevel_.level_);
        UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
        originalSpeed_ = GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"player_forward_speed") / 100.0f;
        combatExpireTime_ = 0;
        updateChaobleTime_ = 0;
        if (characterInfo_->stateInfo_.hasState(cstCombat)) {
            combatExpireTime_ = GAME_TIMER->msec() + peaceTime;
        }
        buyBackIndex_ = 0;
        shouldReleaseDownSpeedScript_ = false;
        shouldReleaseUpSpeedScript_ = false;

        restoreSpeedAndScale();

        MoreCharacterInfo& moreCharacterInfo = entityInfo.asCharacterInfo();
        moreCharacterInfo.selectTitleCode_ = characterInfo_->selectTitleCode_;
        static_cast<CharacterInfo&>(moreCharacterInfo) = *characterInfo_;

        if (guild.get() != nullptr) {
            moreCharacterInfo.guildName_ = guild->getGuildName();
            moreCharacterInfo.guildMarkCode_ = guild->getGuildMarkCode();
        }
        else {
            moreCharacterInfo.guildName_.clear();
            moreCharacterInfo.guildMarkCode_ = invalidGuildMarkCode;
        }

        if (! fillEquipCode(moreCharacterInfo.equipments_, *characterInfo_)) {
            return ecCharacterEquipIsNotValid;
        }

        initInventory(moreCharacterInfo);
        initBankAccount();
        initQuestRepository();
        initStatus(moreCharacterInfo);       		
        initPlayerController();

        if (! initSkill()) {
            return ecSkillInvalidSkill;
        }

        moreCharacterInfo.currentPoints_ = characterInfo_->currentPoints_;
        characterInfo_->currentPoints_ = characterInfo_->currentPoints_;

        cheatCommander_= std::make_unique<CheatCommander>(*this, accountGrade);
        guildSkillReposity_= std::make_unique<GuildSkillRepository>(*this);
        guildSkillReposity_->initialize(skillCodes);
        session_ = session;
        if (session_ != nullptr) {
            sne::sgp::RpcingExtension* extension =
                session_->getImpl().getExtension<sne::sgp::RpcingExtension>();
            getControllerAs<gc::PlayerController>().registerRpc(*extension);
            getPlayerMoveController().registerRpc(*extension);
            getPlayerSkillController().registerRpc(*extension);
            getPlayerInventoryController().registerRpc(*extension);
            getPlayerItemController().registerRpc(*extension);
            getPlayerQuestController().registerRpc(*extension);
            getPlayerTradeController().registerRpc(*extension);
            getPlayerHarvestController().registerRpc(*extension);
            getPlayerTreasureController().registerRpc(*extension);
            getPlayerGraveStoneController().registerRpc(*extension);
            getPlayerPartyController().registerRpc(*extension);
            getPlayerAnchorController().registerRpc(*extension);
            getPlayerGuildController().registerRpc(*extension);
            getPlayerNpcController().registerRpc(*extension);
            getPlayerCastController().registerRpc(*extension);
            getPlayerEffectController().registerRpc(*extension);
            getPlayerMailController().registerRpc(*extension);
            getPlayerAuctionController().registerRpc(*extension);
            getPlayerArenaController().registerRpc(*extension);
            getPlayerBuildingController().registerRpc(*extension);
            getPlayerOutsideInventoryController().registerRpc(*extension);
            getPlayerDeviceController().registerRpc(*extension);
            getPlayerTeleportController().registerRpc(*extension);
            getPlayerDuelController().registerRpc(*extension);
            getPlayerWorldEventController().registerRpc(*extension);
            getPlayerAchievementController().registerRpc(*extension);
            getPlayerCharacterTitleController().registerRpc(*extension);
        }

        oneSecondTracker_.reset(1000);

        if (ZONE_SERVICE->isArenaServer()) {
            playerState_->revived();
            getCreatureStatus().restorePoints();
        }
        initCooldown();
    }
    WORLD->queryDBQueryPlayerCallback()->registPlayer(*this);
    ZONE_SERVICE->getCommunityServerProxy().z2m_initPlayerInfo(playerInfo_);

    return ecOk;
}


void Player::finalize()
{
    unselectTarget();
   
    if (characterInfo_ != nullptr) {
        if (isValidGuildId(characterInfo_->guildId_)) {
            (void)TASK_SCHEDULER->schedule(std::make_unique<GuildOffineEvent>(
                characterInfo_->guildId_, characterInfo_->objectId_));
        }  
        if (! ZONE_SERVICE->isArenaServer()) {
            ZONE_SERVICE->getArenaServerProxy().z2a_logoutChallenger(characterInfo_->objectId_);
        }
        if (arena_) {
            (void)TASK_SCHEDULER->schedule(std::make_unique<LeaveArenaEvent>(
                *arena_, accountId_, characterInfo_->objectId_));
        }

        (void)TASK_SCHEDULER->schedule(std::make_unique<ReleaseSyncMailEvent>(
            characterInfo_->objectId_));

        
        if (fieldDuelPtr_.get() != nullptr) {
            fieldDuelPtr_->stop(fdrtLogoutPlayer, invalidObjectId);
        }

        WORLD_EVENT_SERVICE->logout(characterInfo_->objectId_);
        WORLD->queryDBQueryPlayerCallback()->unregistPlayer(characterInfo_->objectId_);
    }

    if (playerState_.get() != nullptr) {
        playerState_->finalize();
    }
    
    {
        std::lock_guard<LockType> lock(getLock());

        accountGrade_ = agPlayer;
        
        if (characterInfo_ != nullptr) {
            getCoolDownTimer().saveDB();
            getEffectScriptApplier().saveDB();         
        }

        if (passiveSkillManager_.get() != nullptr) {
            passiveSkillManager_->finalize();
        }

        if (skillCastableAbility_.get() != nullptr) {
            skillCastableAbility_->finalize();
        }

        if (characterSkillManager_.get() != nullptr) {
            characterSkillManager_->finalize();
        }

        reset();
    }

    Parent::finalize();
}


void Player::logoutNow()
{
    (void)ZONEUSER_MANAGER->logout(accountId_);

    if (session_ != nullptr) {
        session_->disconnectGracefully();
    }
}


void Player::logout()
{
    // FYI: 클라이언트가 패킷을 받을 시간적 여유를 주기 위함
    const msec_t logoutDelay = 100;
    (void)TASK_SCHEDULER->schedule(std::make_unique<LogoutTask>(*this), logoutDelay);
}


ErrorCode Player::revive(bool skipTimeCheck)
{
    WorldMap* worldMap = nullptr;
    {
        std::lock_guard<LockType> lock(getLock());

        initCastingDelay();
        worldMap = getCurrentWorldMap();
        if (! worldMap) {
            return ecWorldMapNotFound;
        }

        if (! getCreatureStatus().isDied()) {
            return ecCreatureCannotRevive;
        }

        restoreSpeedAndScale();
        permil_t valuePer = getGuildEffectValue(estReviveTimeDown);

        const ErrorCode errorCode = worldMap->checkRespawn(*this, valuePer, skipTimeCheck);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        resetSpeedRate_i();
    }

    playerState_->revived();
    const ObjectPosition spawnPosition = worldMap->getNearestGravePosition(getPosition(), isChao());
    const ErrorCode errorCode =
        TELEPORT_SERVICE->teleportPosition(WorldPosition(spawnPosition, worldMap->getMapCode()), *this);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    getCreatureStatus().restorePoints(true);
    
    return ecOk;
}


bool Player::reviveByEffect(HitPoint& refillPoint, permil_t perRefillHp)
{
    std::lock_guard<LockType> lock(getLock());

    if (! getCreatureStatus().isDied()) {
        return false;
    }

    restoreSpeedAndScale();
    resetSpeedRate_i();
    refillPoint = toHitPoint((getCurrentCreatureStatusInfo().points_.hp_ * perRefillHp) / 1000);
    playerState_->revived();
    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    MoreCharacterInfo& moreCharacterInfo = entityInfo.asCharacterInfo();
    characterInfo_->currentPoints_.hp_ = refillPoint;
    moreCharacterInfo.currentPoints_.hp_ = refillPoint;
    return true;
}


ErrorCode Player::reviveInArena(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLock());

    initCastingDelay();

    if (! getCreatureStatus().isDied()) {
        return ecCreatureCannotRevive;
    }

    restoreSpeedAndScale();

    const ErrorCode errorCode =
        TELEPORT_SERVICE->teleportPosition(WorldPosition(position, getCurrentWorldMap()->getMapCode()), *this);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    resetSpeedRate_i();
    getCreatureStatus().restorePoints(true);
    playerState_->revived();

    return ecOk;
}


void Player::setArena(Arena* arena) 
{
    std::lock_guard<LockType> lock(getLock());

    arena_ = arena;
}


ErrorCode Player::learnSkills(const SkillCodes& skillCodes)
{
    {
        std::lock_guard<LockType> lock(getLock());

        SkillPoint needPoint = toSkillPoint(0);
        const ErrorCode errorCode = checkLearnSkills(needPoint, skillCodes);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        for (SkillCode skillCode : skillCodes) { 
            const SkillTableType learnSkillTableType = getSkillTableType(skillCode);
            const SkillIndex learnSkillIndex = getSkillIndex(skillCode);
            const SkillCode currentSkillCode = characterSkillManager_->getCurrentSkillCode(learnSkillTableType, learnSkillIndex);
            removeSkill(learnSkillTableType, learnSkillIndex);
            addSkill(skillCode);
            gc::SkillCallback* callback = getController().queryPlayerSkillCallback();
            if (callback) {
                callback->skillLearned(currentSkillCode, skillCode);
            }
        }
        characterInfo_->sp_ = toSkillPoint(characterInfo_->sp_ - needPoint);
    }

    notifyChangeCondition(pccLearnSkill);
    saveCharacterStats();
    return ecOk;
}


ErrorCode Player::resetLearnedSkills()
{
    const datatable::CharacterDefaultSkillInfo* defaultSkillInfo =
        CHARACTER_DEFAULT_SKILL_TABLE->getCharacterDefaultSkillInfo(characterInfo_->selectEquipType_);
    if (! defaultSkillInfo) {
        return ecServerInternalError;
    }

    {
        std::lock_guard<LockType> lock(getLock());

        if (isInArenaMap()) {
           return ecSkillNotUnlearnSkillInArena;
       }

        if (defaultSkillInfo->hasOnlyDefaultSkill(characterInfo_->skills_)) {
            return ecSkillNotExistLearnSkill;
        }
        const SkillPoint skillPoint = characterInfo_->getRewardedSkillPoint();
        const GameMoney fee = byGold(1); // 1gold
        if (characterInfo_->gameMoney_ < fee) {
            return ecSkillUnleranNotEnoughMoney;
        }
        CreatureSkills copiedSkills = characterInfo_->skills_;
        for (SkillCode skillCode : copiedSkills) {
            const SkillTableType tableType = getSkillTableType(skillCode);
            const SkillIndex skillIndex = getSkillIndex(skillCode);
            removeSkill(tableType, skillIndex);
        }
        assert(characterInfo_->skills_.empty());

        gc::SkillCallback* skillCallback = getController().queryPlayerSkillCallback();
        if (skillCallback) {
            skillCallback->skillAllRemoved();
        }
        characterInfo_->skills_.clear();
        characterInfo_->sp_ = skillPoint;

        for (const datatable::DefaultSkillInfo& skillInfo : defaultSkillInfo->getCharacterDefaultSkills()) {
            addSkill(skillInfo.skillCode_);
            if (skillCallback) {
                skillCallback->skillLearned(invalidSkillCode, skillInfo.skillCode_);
            }
        }
        downGameMoney(fee);
    }

    notifyChangeCondition(pccLearnSkill);
    saveCharacterStats();
    return ecOk;
}


ErrorCode Player::learnSkill(SkillTableType skillTableType, SkillIndex index)
{
    SkillCode learnSkillCode = characterSkillManager_->generateSkillCode(skillTableType, index);
    if (! isValidSkillCode(learnSkillCode)) {
        return ecSkillMaxSkillNotLearn;
    }
    
    datatable::SkillLearnInfo learnInfo;
    if (! fillSkillLearnInfo(learnInfo, learnSkillCode)) {
        return ecSkillInvalidSkill;
    }
    
    if (! checkCastableNeedSkill(learnInfo.needLearnSkillCode1_)) {
        return ecSkillHasNotNeedSkill;
    }

    if (! checkCastableNeedSkill(learnInfo.needLearnSkillCode2_)) {
        return ecSkillHasNotNeedSkill;
    }

    if (! learnInfo.canLearnCharacterClass(characterInfo_->characterClass_)) {
        return ecSkillLearnNotCharacterClass;
    }

    SkillCode currentSkillCode = characterSkillManager_->getCurrentSkillCode(skillTableType, index);
    
    const SkillPoint needPoint = getLearnSkillPoint(characterInfo_->characterClass_, learnInfo.learnCharacterClass_, learnInfo.needSp_);

    if (characterInfo_->sp_ < needPoint) {
        return ecSkillNotEnoughSkillPoint;
    }

    characterInfo_->sp_ = toSkillPoint(characterInfo_->sp_ - needPoint);

    removeSkill(skillTableType, index);
    addSkill(learnSkillCode);

    notifyChangeCondition(pccLearnSkill);

    gc::SkillCallback* callback = getController().queryPlayerSkillCallback();
    if (callback) {
        callback->skillLearned(currentSkillCode, learnSkillCode);
    }

    return ecOk;
}


ErrorCode Player::unlearnSkill(SkillTableType skillTableType, SkillIndex index)
{
    std::lock_guard<LockType> lock(getLock());

    // TODO: 지우는 스킬이 필요한 스킬이 있을경우 초기화 못함

    if (getGameMoney() < unlearnSkillFee) {
        return ecSkillUnleranNotEnoughMoney;
    }

    SkillCode unlearnSkillCode = characterSkillManager_->getCurrentSkillCode(skillTableType, index);
    if (! isValidSkillCode(unlearnSkillCode)) {
        return ecSkillInvalidSkill;
    }

    datatable::SkillLearnInfo skillLearnInfo;
    if (! fillSkillLearnInfo(skillLearnInfo, unlearnSkillCode)) {
        return ecSkillInvalidSkill;
    }

    if (checkCastableNeedSkill(skillLearnInfo.nextSkillCode1_)) {
        return ecSkillHasNextSkill;
    }

    if (checkCastableNeedSkill(skillLearnInfo.nextSkillCode2_)) {
        return ecSkillHasNextSkill;
    }
     
    SkillLevel skillLevelCount = getSkillLevel(unlearnSkillCode);
    SkillPoint unlearnedSkillPoint = toSkillPoint(0);
    for (SkillLevel level = 1 ; level <= skillLevelCount; ++level) {
        const SkillCode skillCode = makeSkillCode(skillTableType, index, level);
        datatable::SkillLearnInfo learnInfo;
        if (! fillSkillLearnInfo(learnInfo, skillCode)) {
            return ecSkillInvalidSkill;
        }
        const SkillPoint neededSkillPoint = getLearnSkillPoint(characterInfo_->characterClass_,
            learnInfo.learnCharacterClass_, learnInfo.needSp_);
        unlearnedSkillPoint = toSkillPoint(unlearnedSkillPoint + neededSkillPoint);
    }
    characterInfo_->sp_ = toSkillPoint(characterInfo_->sp_ + unlearnedSkillPoint);

    downGameMoney(unlearnSkillFee);
    removeSkill(skillTableType, index);
    notifyChangeCondition(pccLearnSkill);

    gc::SkillCallback* callback = getController().queryPlayerSkillCallback();
    if (callback) {
        callback->skillRemoved(unlearnSkillCode);
    }
   
    return ecOk;
}

void Player::addSkill(SkillCode skillCode)
{
    characterSkillManager_->updateSkill(skillCode); 
    if (sttPassivePlayer == getSkillTableType(skillCode)) {
        passiveSkillManager_->learn(skillCode);
    }
    else {
        skillCastableAbility_->learn(skillCode);
    }
    characterInfo_->skills_.insert(skillCode);
}


void Player::removeSkill(SkillTableType skillTableType, SkillIndex index)
{
    SkillCode unlearnSkillCode = characterSkillManager_->getCurrentSkillCode(skillTableType, index);
    if (! isValidSkillCode(unlearnSkillCode)) {
        return;
    }   

    characterSkillManager_->removeSkill(skillTableType, index);
    characterInfo_->skills_.erase(unlearnSkillCode);
    if (isValidSkillCode(unlearnSkillCode)) {
        if (sttPassivePlayer == skillTableType) {
            passiveSkillManager_->unlearn(unlearnSkillCode);
        }
        else {
            skillCastableAbility_->cancel(unlearnSkillCode);
            skillCastableAbility_->unlearn(unlearnSkillCode);
        }
    }
}


ErrorCode Player::checkLearnSkills(SkillPoint& needSkillPoint, const SkillCodes& skillCodes)
{
    if (isInArenaMap()) {
        return ecSkillNotLearnSkillInArena;
    }

    needSkillPoint = toSkillPoint(0);
    CharacterSkillManager skillManager = *characterSkillManager_;
    for (SkillCode skillCode : skillCodes) { 
        if (! isValidSkillCode(skillCode)) {
            return ecSkillInvalidSkill;
        }

        const SkillTableType learnSkillTableType = getSkillTableType(skillCode);
        const SkillIndex learnSkillIndex = getSkillIndex(skillCode);
        const SkillLevel learnSkillLevel = getSkillLevel(skillCode);
        const SkillCode currentSkillCode = skillManager.getCurrentSkillCode(learnSkillTableType, learnSkillIndex);
        SkillLevel currentSkillLevel = getSkillLevel(currentSkillCode);
        if (learnSkillLevel <= currentSkillLevel) {
            return ecSkillMaxSkillNotLearn;
        }

        currentSkillLevel = currentSkillLevel + 1;
        for (SkillLevel s = currentSkillLevel; s <= learnSkillLevel; ++s) {
            SkillCode learnSkillCode = makeSkillCode(learnSkillTableType, learnSkillIndex, s);
            datatable::SkillLearnInfo learnInfo;
            if (! fillSkillLearnInfo(learnInfo, learnSkillCode)) {
                return ecSkillInvalidSkill;
            }
            if (! hasNeedSkill(skillManager, learnInfo.needLearnSkillCode1_)) {
                return ecSkillHasNotNeedSkill;
            }

            if (! hasNeedSkill(skillManager, learnInfo.needLearnSkillCode2_)) {
                return ecSkillHasNotNeedSkill;
            }

            if (! learnInfo.canLearnCharacterClass(characterInfo_->characterClass_)) {
                return ecSkillLearnNotCharacterClass;
            }

            needSkillPoint = toSkillPoint(needSkillPoint + getLearnSkillPoint(characterInfo_->characterClass_, learnInfo.learnCharacterClass_, learnInfo.needSp_));
            skillManager.removeSkill(learnSkillTableType, learnSkillIndex);
            skillManager.updateSkill(learnSkillCode);
        }

    }
    if (characterInfo_->sp_ < needSkillPoint) {
        return ecSkillNotEnoughSkillPoint;
    }

    return ecOk;
}


void Player::setActionBarLock(bool isLocked)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->actionBars_.setActionBarLock(isLocked);
}


void Player::setGuildInfo(const BaseGuildInfo& guildInfo)
{
    SkillCodes skillCodes;
    if (guildInfo.isValid()) {
        GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildInfo.guildId_);
        if (guild.get()) {
            skillCodes = guild->getGuildSkillCodes();
        }
    }

    {
        std::lock_guard<LockType> lock(getLock());

        UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
        MoreCharacterInfo& moreCharacterInfo = entityInfo.asCharacterInfo();
        moreCharacterInfo.guildId_ = guildInfo.guildId_;
        moreCharacterInfo.guildMarkCode_ = guildInfo.guildMarkCode_;
        moreCharacterInfo.guildName_ = guildInfo.guildName_;
        if (guildInfo.isValid()) {
            guildSkillReposity_->initialize(skillCodes);
        }
        else {
            guildSkillReposity_->clear();
        }
    }
}


GuildId Player::getGuildId() const
{
    std::lock_guard<LockType> lock(getLock());

    const UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    const MoreCharacterInfo& moreCharacterInfo = entityInfo.asCharacterInfo();
    return moreCharacterInfo.guildId_;
}


int32_t Player::getGuildEffectValue(EffectScriptType type) const
{
    if (guildSkillReposity_.get() != nullptr) {
        return guildSkillReposity_->getEffectValue(type);
    }
    return 0;
}


void Player::activateSkill(SkillCode skillCode)
{
    if (guildSkillReposity_.get() != nullptr) {
        guildSkillReposity_->activateSkill(skillCode);
    }
}


void Player::deactivateSkills()
{
    if (guildSkillReposity_.get() != nullptr) {
        guildSkillReposity_->deactivateSkills();
    }
}


void Player::setTargetOfTarget(const GameObjectInfo& entityInfo)
{
    std::lock_guard<LockType> lock(getLock());

    selectedTargetOfTargetInfo_ = entityInfo;
}


const GameObjectInfo& Player::getSelectedTargetOfTarget() const
{
    std::lock_guard<LockType> lock(getLock());

    return selectedTargetOfTargetInfo_;
}


void Player::saveCharacterStats()
{
    if (ZONE_SERVICE->isArenaServer()) {
        return;
    }
    if (characterInfo_ != nullptr) {
        // TODO: 시간 갱신은 특정 비율이하일때만 깍는다(현재는 무조건 시간으로 깍임)
        bool isActionBarLock = false;
        CreatureStateInfo state;
        {
            std::lock_guard<LockType> lock(getLock());

            if (characterInfo_->stateInfo_.hasState(cstCombat)) {
                state.toggleState(cstCombat);
            }
            if (characterInfo_->stateInfo_.hasState(cstDead)) {
                state.toggleState(cstDead);
            }

            isActionBarLock = characterInfo_->actionBars_.isActionBarLock_;
        }


        const DBCharacterStats saveInfo(characterInfo_->currentLevel_.level_,
            characterInfo_->currentLevel_.levelStep_, characterInfo_->exp_,
            uint8_t(state.states_), characterInfo_->sp_,
            characterInfo_->currentPoints_, getWorldPosition(), WORLD->getZoneId(),
            characterInfo_->gameMoney_,  characterInfo_->bankAccountInfo_.gameMoney_,
            characterInfo_->chaotic_, isActionBarLock,
            characterInfo_->characterClass_, characterInfo_->arenaPoint_,
            characterInfo_->eventCoin_, characterInfo_->forgeCoin_);

        DatabaseGuard db(SNE_DATABASE_MANAGER);

        db->asyncSaveCharacterStats(accountId_, getObjectId(), saveInfo);
    }
}


ErrorCode Player::saveActionBar(ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code)
{
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncSaveActionBar(accountId_, getObjectId(), abiIndex, abpIndex, code);
    }

    return ecOk;
}


bool Player::isValid() const
{
    std::lock_guard<LockType> lock(getLock());

    return isValidAccountId(accountId_) && (characterInfo_ != nullptr) &&
        characterInfo_->isValid();
}


bool Player::isInArenaMap() const
{
    return arena_ != nullptr;
}


bool Player::isSameTeamInArena(go::Entity& target) const
{
    if (! target.isPlayer()) {
        return false;
    }

    return arena_->isSameTeam(target.getObjectId(), getObjectId());
}



bool Player::isFieldDueling() const
{
    return queryFieldDuelState()->isFieldDueling();
}


bool Player::isMyDueler(go::Entity& target) const
{
    const FieldDuelState* fieldDuelState = target.queryFieldDuelState();
    if (! fieldDuelState) {
        return false;
    }

    return fieldDuelState->isMyDueler(queryFieldDuelState()->getFieldDuelId());
}


bool Player::isMyPartyMember(go::Entity& target) const
{
    go::Partyable* partyabye = target.queryPartyable();
    if (partyabye) {        
        if (party_.get() != nullptr) {
            if (partyabye->getParty() == party_) {
                return true;
            }
        }
    }
    return false;
}


bool Player::isMyGuildMember(go::Entity& target) const
{
    if (! target.isPlayer()) {
        return false;
    }
    GuildId casterGuildId = getGuildId();
    GuildId targetGuildId = target.queryGuildable()->getGuildId();
    if (! isValidGuildId(casterGuildId) || ! isValidGuildId(targetGuildId)) {
        return false;
    }
    return casterGuildId == targetGuildId;
}


Inventory& Player::getInventory() const
{
    return *inventory_;
}


Inventory& Player::getVehicleInventory() const
{
    return *vehicleInventory_;
}


EquipInventory& Player::getEquipInventory() const
{
    return *inventory_;
}


AccessoryInventory& Player::getAccessoryInventory() const
{
    return *inventory_;
}


QuestInventory& Player::getQuestInventory() const 
{
    return *questInventory_;
}


InventoryInfo Player::getInventoryInfo() const
{
    std::lock_guard<LockType> lock(getLock());

    assert(characterInfo_ != nullptr);
    return characterInfo_->inventory_;
}


ExpPoint Player::getRewardExpPoint() const
{
    std::lock_guard<LockType> lock(getLock());

    // TODO skill
    //const CreatureStatusInfo& gameStats = getCurrentCreatureStatusInfo();
    //const int pointDiffValue = 50;
    //const ExpPoint pointExp = toExpPoint((gameStats.maxPoints_.hp_ + 
    //    gameStats.maxPoints_.mp_) / pointDiffValue);
    //const ExpPoint attributeExp = toExpPoint(static_cast<int>(gameStats.getTotalAttributeRate()) * 100);
    //const ExpPoint attackExp = toExpPoint(gameStats.attackDamage_ / 10);
    return toExpPoint(0);//toExpPoint((pointExp + attributeExp + attackExp) / 20);
}


AccessoryIndex Player::getAccessoryIndex(ObjectId itemId) const
{    
    for (int i = 0; i < aiCount; ++i) {
        if (characterInfo_->characterAccessories_[i] == itemId) {
            return toAccessoryIndex(i);
        }
    }
    return aiInvalid;
}


VehicleCode Player::getVehicleCode() const
{
    std::lock_guard<LockType> lock(getLock());

    const VehicleInfo* info =
        characterInfo_->vehicleInventory_.getCurrentVehicleInfo();
    if (info) {
        return info->vehicleCode_;
    }
    return invalidVehicleCode;
}


EntityVehicleInfo Player::getEntityVehicle() const
{
    std::lock_guard<LockType> lock(getLock());

    EntityVehicleInfo entityVehicleInfo;
    const VehicleInfo* info = 
        characterInfo_->vehicleInventory_.getCurrentVehicleInfo();
    if (info) {
        entityVehicleInfo.vehicleCode_ = info->vehicleCode_;
        entityVehicleInfo.harnessCode_ = info->harnessInfo_.harnessCode_;
    }
    return entityVehicleInfo;
}


GliderCode Player::getGliderCode() const
{
    std::lock_guard<LockType> lock(getLock());

    const GliderInfo* info = characterInfo_->gliderInventory_.getCurrentGliderInfo();
    if (info) {
        return info->gliderCode_;
    }
    return invalidGliderCode;
}


uint32_t Player::getGliderDurability() const
{
    std::lock_guard<LockType> lock(getLock());

    GliderInfo* gliderInfo = characterInfo_->gliderInventory_.getCurrentGliderInfo();
    if (! gliderInfo) {
        return 0;
    }
    return gliderInfo->durability_;
}


void Player::setAccessoryIndex(AccessoryIndex index, ObjectId itemId)
{
    assert(gideon::isValid(index));
    characterInfo_->characterAccessories_[index] = itemId;
}


bool Player::isEquipAccessory(AccessoryIndex index) const
{
    assert(gideon::isValid(index));
    return isValidObjectId(characterInfo_->characterAccessories_[index]);
}


void Player::upGameMoney(GameMoney gameMoney)
{    
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->upGameMoney(gameMoney);
}


void Player::downGameMoney(GameMoney gameMoney)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->downGameMoney(gameMoney);
}


void Player::setGameMoney(GameMoney gameMoney)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->gameMoney_ = gameMoney;
}


void Player::upForgeCoin(ForgeCoin forgeCoin)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->upForgeCoin(forgeCoin);
}


void Player::downForgeCoin(ForgeCoin forgeCoin)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->downForgeCoin(forgeCoin);
}


void Player::setForgeCoin(ForgeCoin forgeCoin)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->forgeCoin_ = forgeCoin;
}


void Player::upArenaPoint(ArenaPoint arenaPoint)
{
    {
        std::lock_guard<LockType> lock(getLock());

        permil_t perValue = guildSkillReposity_->getEffectValue(estArenaPointBonus);
        if (0 < perValue) {
            arenaPoint += ((arenaPoint * perValue) / 1000);
        }
        characterInfo_->upArenaPoint(arenaPoint);    
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateArenaPoint(accountId_, getObjectId(), characterInfo_->arenaPoint_);
    }
}


void Player::downArenaPoint(ArenaPoint arenaPoint)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->downArenaPoint(arenaPoint);
}


void Player::setArenaPoint(ArenaPoint arenaPoint)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->arenaPoint_ = arenaPoint;
}

void Player::upEventCoin(EventCoin eventCoin)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->eventCoin_ += eventCoin;
}


void Player::downEventCoin(EventCoin eventCoin)
{
    std::lock_guard<LockType> lock(getLock());

    if (characterInfo_->eventCoin_ > eventCoin) {
        characterInfo_->eventCoin_ -= eventCoin;
    }
    else {
        characterInfo_->eventCoin_ = 0;
    }
}


void Player::setEventCoin(EventCoin eventCoin)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->eventCoin_ = eventCoin;
}


void Player::upChaotic(Chaotic chaotic)
{
    std::lock_guard<LockType> lock(getLock());

    upChaotic_i(chaotic);
}


void Player::downChaotic(Chaotic chaotic)
{
    std::lock_guard<LockType> lock(getLock());

    downChaotic_i(chaotic);
}



ErrorCode Player::buyItem(const BaseItemInfo& itemInfo, uint64_t& currentValue,
    CostType ct, uint32_t totalValue)
{
    std::lock_guard<LockType> lock(getLock());

    switch (ct) {
    case ctGameMoney:
        if (characterInfo_->gameMoney_ < totalValue) {
            return ecStoreNotEnoughGameMoney;
        }
        break;   
    case ctArenaPoint:
        if (characterInfo_->arenaPoint_ < totalValue) {
            return ecStoreNotEnoughArenaPoint;
        }
        break;
    case ctEventCoin:
        if (characterInfo_->eventCoin_ < totalValue) {
            return ecStoreNotEnoughEventCoin;
        }
        break;
    case ctForgeCoin:
        if (characterInfo_->forgeCoin_ < totalValue) {
            return ecStoreNotEnoughEventCoin;	
        }
        break;
    }

    const ErrorCode errorCode = inventory_->addItem(createAddItemInfoByBaseItemInfo(itemInfo));    

    if (isSucceeded(errorCode)) {
        switch (ct) {
        case ctGameMoney:
            downGameMoney(totalValue);
            currentValue = getGameMoney();
            break;        
        case ctArenaPoint:
            downArenaPoint(totalValue);
            currentValue = getArenaPoint();
            break;
        case ctEventCoin:
            downEventCoin(totalValue);
            currentValue = getEventCoin();
            break;
        case ctForgeCoin:
            downForgeCoin(totalValue);
            currentValue = getForgeCoin();
            break;
        }
    }

    return errorCode;
}


void Player::sellItem(uint64_t& currentValue, uint32_t& buyBackIndex, BuyBackItemInfo& buyBackInfo, ObjectId itemId, uint8_t itemCount,
    CostType ct, uint32_t totalValue)
{
    std::lock_guard<LockType> lock(getLock());

    ItemInfo itemInfo = inventory_->getItemInfo(itemId);
    itemInfo.count_ = itemCount;
    buyBackInfo.itemCode_ = itemInfo.itemCode_;
    buyBackInfo.count_ = itemInfo.count_;
    if (itemInfo.isAccessory()) {
        buyBackInfo.accessoryItemInfo_ = itemInfo.accessoryItemInfo_;
    }
    else if (itemInfo.isEquipment()) {
        buyBackInfo.equipItemInfo_ = itemInfo.equipItemInfo_;
    }
    buyBackInfo.costType_ = ct;
    buyBackInfo.cost_ = totalValue;

    inventory_->useItemsByItemId(itemId, itemCount); 

    switch (ct) {
    case ctGameMoney:
        upGameMoney(totalValue);
        currentValue = getGameMoney();
        break;        
    case ctArenaPoint:
        upArenaPoint(totalValue);
        currentValue = getArenaPoint();
        break;
    case ctEventCoin:
        upEventCoin(totalValue);
        currentValue = getEventCoin();
        break;
    case ctForgeCoin:
        upForgeCoin(totalValue);
        currentValue = getForgeCoin();
        break;
    }    
    buyBackItemMap_.emplace(++buyBackIndex_, buyBackInfo);
    const size_t buyBackItemCount = GIDEON_PROPERTY_TABLE->getPropertyValue<size_t>(L"buy_back_count");
    if (buyBackItemCount < buyBackItemMap_.size()) {
        buyBackItemMap_.erase(buyBackItemMap_.begin());
    }
    buyBackIndex = buyBackIndex_;
}


ErrorCode Player::buyBackItem(CostType& ct, uint64_t& currentValue, uint32_t index)
{
    std::lock_guard<LockType> lock(getLock());

    BuyBackItemInfoMap::iterator pos = buyBackItemMap_.find(index);
    if (pos == buyBackItemMap_.end()) {
        return ecStoreNotExistBuyBackItem;
    }

    const BuyBackItemInfo& itemInfo = (*pos).second;
    ct = itemInfo.costType_;
    switch (itemInfo.costType_) {
    case ctGameMoney:
        if (characterInfo_->gameMoney_ < itemInfo.cost_) {
            return ecStoreNotEnoughGameMoney;
        }
        break;   
    case ctArenaPoint:
        if (characterInfo_->arenaPoint_ < itemInfo.cost_) {
            return ecStoreNotEnoughArenaPoint;
        }
        break;
    case ctEventCoin:
        if (characterInfo_->eventCoin_ < itemInfo.cost_) {
            return ecStoreNotEnoughEventCoin;
        }
        break;
    case ctForgeCoin:
        if (characterInfo_->forgeCoin_ < itemInfo.cost_) {
            return ecStoreNotEnoughEventCoin;	
        }
        break;
    }

    if (! inventory_->canAddItemByBaseItemInfo(itemInfo)) {
        return ecInventoryIsFull;
    }
    
    inventory_->addItem(createAddItemInfoByBuyBackItemInfo(itemInfo));

    switch (itemInfo.costType_) {
    case ctGameMoney:
        downGameMoney(itemInfo.cost_);
        currentValue = getGameMoney();
        break;        
    case ctArenaPoint:
        downArenaPoint(itemInfo.cost_);
        currentValue = getArenaPoint();
        break;
    case ctEventCoin:
        downEventCoin(itemInfo.cost_);
        currentValue = getEventCoin();
        break;
    case ctForgeCoin:
        downForgeCoin(itemInfo.cost_);
        currentValue = getForgeCoin();
        break;
    }

    buyBackItemMap_.erase(index);
    
    return ecOk;
}

void Player::robbed(LootInvenItemInfoMap& infoMap)
{
    inventory_->robbed(infoMap);
    GameMoney dropMoney = getGameMoney() / 10; // 10% 드랍
    GameMoney deleteMoney = getGameMoney() / 20; // 5% 지우기
    if (dropMoney + deleteMoney > 0) {
        gc::RewardCallback* callback = getController().queryRewardCallback();
        callback->gameMoneyRewarded(dropMoney + deleteMoney, false);
        LootInvenItemInfo itemInfo(liitGameMoney);
        itemInfo.money_ = dropMoney;
        infoMap.emplace(toLootInvenId(infoMap.size() + 1), itemInfo);
    }
}


void Player::handleExpiredTasks()
{
    bool isMinChaotic = false;
    bool isChangeChaoState = false;
    bool isTempChangeChaoState = false;
    bool isMinTempChaotic = false;

    FieldDualPtr fieldDualPtr;
    {
        std::lock_guard<LockType> lock(getLock());

        fieldDualPtr = fieldDuelPtr_;
    }

    if (fieldDualPtr.get() != nullptr) {
        if (fieldDualPtr->shouldStopByDistance(*this)) {
            fieldDualPtr->stop(fdrtCancelDistance, invalidObjectId);
        }
    }

    {
        {
            std::lock_guard<LockType> lock(getLock());

            const GameTime saveTime = 10 * 60 * 1000;
            const GameTime now = GAME_TIMER->msec();
            if (lastSaveStatsTime_ + saveTime <= now) {
                lastSaveStatsTime_ += saveTime;
                saveCharacterStats();
            }
        }

        if (getCreatureStatus().isDied()) {
            return;
        }

        {
            std::lock_guard<LockType> lock(getLock());

            if (tempChaoInfo_.isChao()) {
                tempChaoInfo_.downChaotic(toChaotic(1));
                isTempChangeChaoState = true;
                if (! tempChaoInfo_.isChao()) {
                    isMinTempChaotic = true;
                }
            }
            if (characterInfo_->chaotic_ > 0) {
                downChaotic_i(toChaotic(1));
                isChangeChaoState = true;
                if (characterInfo_->chaotic_ == 0) {
                    isMinChaotic = true;
                }
            }  
            const GameTime now = GAME_TIMER->msec();
            if (combatExpireTime_ != 0 && now > combatExpireTime_) {
                gc::EntityStatusCallback* callback = getController().queryEntityStatusCallback();
                if (callback) {
                    callback->playerCombatStateChanged(false);
                }
                combatExpireTime_ = 0;
            }
        }
    }

    if (! queryCombatState()->isCombating()) {
        getCreatureStatus().fillMp(getCreatureStatus().getManaPointPer());
    }

    if (queryMoveState()->isInLava()) {
        getCreatureStatus().reduceHp(toHitPoint(100), true);
    }
    
    if (! ZONE_SERVICE->isArenaServer()) {
        if (isChangeChaoState && isMinChaotic) {
            gc::ChaoCallback* chaoCallback = getController().queryChaoCallback();
            if (chaoCallback) {
                chaoCallback->changeChao(false);
            }
        }
        if (! isRealChao() && isTempChangeChaoState && isMinTempChaotic) {
            gc::ChaoCallback* chaoCallback = getController().queryChaoCallback();
            if (chaoCallback) {
                chaoCallback->changeTempChao(false);
            }
        }
    }    
}


bool Player::changeCombatState(bool isCombatState)
{
    return playerState_->changeCombatState(isCombatState);
}


void Player::initializeCharacterTitle(ErrorCode errorCode, const CharacterTitleCodeSet& titleCodeSet)
{
    if (isSucceeded(errorCode)) {
        std::lock_guard<LockType> lock(getLock());

        characterTitleCodeSet_ = titleCodeSet;
    }
    getPlayerCharacterTitleController().onQueryCharacterTitles(errorCode, titleCodeSet);
}


ErrorCode Player::selectCharacterTitle(CharacterTitleCode titleCode)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (isValidCharacterTitleCode(titleCode)) {
            CharacterTitleCodeSet::iterator pos = characterTitleCodeSet_.find(titleCode);
            if (pos == characterTitleCodeSet_.end()) {
                return ecTitleHasNotCharacterTitle;
            }
        }
        characterInfo_->selectTitleCode_ = titleCode;
        UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
        MoreCharacterInfo& moreCharacterInfo = entityInfo.asCharacterInfo();
        moreCharacterInfo.selectTitleCode_ = titleCode;
    }

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncSaveSelectCharacterTitle(accountId_, getObjectId(), titleCode);

    return ecOk;
}

void Player::changeCharacterClass(CharacterClass characterClass)
{
    CharacterClass orgClass = characterInfo_->characterClass_;
    {
        std::lock_guard<LockType> lock(getLock());

        characterInfo_->characterClass_ = characterClass;
        UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
        entityInfo.asCharacterInfo().characterClass_ = characterClass;
    }
    getCreatureStatus().changeCharacterClass(orgClass);
    saveCharacterStats();
    ZONE_SERVICE->getCommunityServerProxy().z2m_changeCharacterClass(getAccountId(), getObjectId(), characterClass);
}


void Player::initializeAchievement(ErrorCode errorCode, const ProcessAchievementInfoMap& processInfoMap,
    const CompleteAchievementInfoMap& completeInfoMap)
{
    if (isSucceeded(errorCode)) {
        std::lock_guard<LockType> lock(getLock());

        achievementRepository_=
            std::make_unique<AchievementRepository>(*this, processInfoMap, completeInfoMap);
    }
    getPlayerAchievementController().onQueryAchievements(errorCode, processInfoMap, completeInfoMap);
}


void Player::updateAchievement(AchievementMissionType missionType, Entity* target)
{
    AchievementRepository* repository = nullptr;
    {
        std::lock_guard<LockType> lock(getLock());

        repository = achievementRepository_.get();
    }
    if (repository) {
        repository->update(missionType, target);
    }
    
}


void Player::updateAchievement(AchievementMissionType missionType, uint32_t param1, uint32_t param2)
{
    AchievementRepository* repository = nullptr;
    {
        std::lock_guard<LockType> lock(getLock());

        repository = achievementRepository_.get();
    }
    if (repository) {
        repository->update(missionType, param1, param2);
    }
}


void Player::loadProperties()
{
    DatabaseGuard db(SNE_DATABASE_MANAGER);

    sne::base::Future::Ref future = db->asyncLoadCharacterProperties(accountId_, getObjectId());
    if (future.get() != nullptr) {
        future->attach(
            new LoadCharacterPropertiesRequestFutureObserver(getControllerAs<gc::PlayerController>()));
        return;
    }
}


void Player::saveProperties(const std::string& config, const std::string& prefs)
{
    DatabaseGuard db(SNE_DATABASE_MANAGER);

    db->asyncSaveCharacterProperties(accountId_, getObjectId(), config, prefs);
}


bool Player::updateAttackTime(GameTime cooltime)
{
    const GameTime now = GAME_TIMER->msec();
    
    if (releaseLockPeaceTime_ > now) {
        return false;
    }

    combatExpireTime_ = GAME_TIMER->msec() + cooltime + peaceTime;
    return true;
}


void Player::updateLockPeaceTime(GameTime delayTime)
{
    releaseLockPeaceTime_ = GAME_TIMER->msec() + delayTime;
}


void Player::openOutsideInventory(InvenType invenType) 
{
    std::lock_guard<LockType> lock(getLock());

    outsideInvenOpenInfos_.insert(invenType);
}


void Player::closeOutsideInventory(InvenType invenType)
{
    std::lock_guard<LockType> lock(getLock());

    outsideInvenOpenInfos_.erase(invenType);
}


bool Player::isOpenOutsideInventory(InvenType invenType) const
{
    std::lock_guard<LockType> lock(getLock());

    return outsideInvenOpenInfos_.find(invenType) != outsideInvenOpenInfos_.end();
}


const BindRecallInfo* Player::getBindRecallInfo(ObjectId linkId) const
{
    std::lock_guard<LockType> lock(getLock());

    return characterInfo_->getBindRecallInfo(linkId);
}


ErrorCode Player::addBindRecall(const BindRecallInfo& info)
{
    std::lock_guard<LockType> lock(getLock());

    if (characterInfo_->maxBindRecallCont_ < characterInfo_->bindRecallInfos_.size()) {
        return ecTeleportMaxOverBindRecall;
    }
    characterInfo_->bindRecallInfos_.push_back(info);
    return ecOk;
}


void Player::removeBindRecall(ObjectId linkId)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->removeBindRecall(linkId);
}


void Player::setFieldDuel(FieldDualPtr ptr)
{
    std::lock_guard<LockType> lock(getLock());

    if (ptr.get()) {
        duelHitPoint_ = characterInfo_->currentPoints_.hp_;
        duelManaPoint_ = characterInfo_->currentPoints_.mp_;
    }
    else {
        updateChaobleTime_ = GAME_TIMER->msec() + 2000;
        getCreatureStatus().setHp(duelHitPoint_, true);
        getCreatureStatus().setMp(duelManaPoint_);
        duelHitPoint_ = hpMin;
        duelManaPoint_ = mpMin;
    }

    fieldDuelPtr_ = ptr;
}


FieldDualPtr Player::getFieldDualPtr()
{
    std::lock_guard<LockType> lock(getLock());

    return fieldDuelPtr_;
}


void Player::releaseBeginnerProtection(bool shouldNotify)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (! characterInfo_->isBeginnerProtection_) {
            return;
        }
        characterInfo_->isBeginnerProtection_ = false;
        UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
        MoreCharacterInfo& moreCharacterInfo = entityInfo.asCharacterInfo();
        moreCharacterInfo.isBeginnerProtection_ = false;
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncReleaseBeginnerProtection(accountId_, getObjectId());
    }

    if (shouldNotify) {
        getController().queryEntityStatusCallback()->begginerProtectionReleased();
    }
}


void Player::addVehicle(const VehicleInfo& info)
{
    bool result = false;
    {
        std::lock_guard<LockType> lock(getLock());

        result = characterInfo_->vehicleInventory_.add(info);
    }
    if (result) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncCreateVehicle(accountId_, getObjectId(), info);
    }
    else {
        SNE_LOG_ERROR("Player::addVehicle (C_ID:%" PRIu64 ", ITEM_ID:%" PRIu64 ", ITEM_CODE:%u)",
            getObjectId(), info.objectId_, info.vehicleCode_);
    }
}


void Player::addGlider(const GliderInfo& info)
{
    bool result = false;
    {
        std::lock_guard<LockType> lock(getLock());

        result = characterInfo_->gliderInventory_.add(info);
    }
    if (result) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncCreateGlider(accountId_, getObjectId(), info);
    }
    else {
        SNE_LOG_ERROR("Player::addGlider (C_ID:%" PRIu64 ", ITEM_ID:%" PRIu64 ", ITEM_CODE:%u)",
            getObjectId(), info.objectId_, info.gliderCode_);
    }
}


ErrorCode Player::selectVehicle(ObjectId selectId)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (!characterInfo_->vehicleInventory_.isExist(selectId)) {
            return ecVehicleNotFind;
        }
        characterInfo_->vehicleInventory_.selectId_ = selectId;
    }

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncSelectVehicle(accountId_, getObjectId(), selectId);

    return ecOk;
}


ErrorCode Player::selectGlider(ObjectId selectId)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (!characterInfo_->gliderInventory_.isExist(selectId)) {
            return ecVehicleNotFind;
        }
        characterInfo_->gliderInventory_.selectId_ = selectId;
    }

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncSelectGlider(accountId_, getObjectId(), selectId);

    return ecOk;
}


void Player::decreaseGliderDurability(uint32_t value)
{
    uint32_t durability = 0;
    ObjectId selectId = invalidObjectId;
    {
        std::lock_guard<LockType> lock(getLock());

        GliderInfo* gliderInfo = characterInfo_->gliderInventory_.getCurrentGliderInfo();
        if (! gliderInfo) {
            return;
        }
        gliderInfo->decreaseDurability(value);
        durability = gliderInfo->durability_;
        selectId = characterInfo_->gliderInventory_.selectId_;
    }

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUpdateGliderDurability(accountId_, getObjectId(), selectId, durability);
}


ErrorCode Player::repairGlider(ObjectId gliderId)
{
    uint32_t durability = 0;
    {
        std::lock_guard<LockType> lock(getLock());

        GliderInfo* gliderInfo = characterInfo_->gliderInventory_.getGliderInfo(gliderId);
        if (! gliderInfo) {
            return ecGliderNotFind;
        }
        const gdt::glider_t* glider = GLIDER_TABLE->getGlider(gliderInfo->gliderCode_);
        if (! glider) {
            return ecVehicleInvalidGlider;
        }
        float64_t leftPct = gliderInfo->durability_ / 
            (static_cast<float64_t>(glider->glide_durability_sec()) * float64_t(0.01));
        
        GameMoney payment = (100 - static_cast<uint32_t>(leftPct)) * glider->repair_payment();
        if (characterInfo_->gameMoney_ < payment) {
            return ecGliderNotEnoughGameMoney;
        }
        downGameMoney(payment);
        gliderInfo->durability_ = glider->glide_durability_sec();
        durability = gliderInfo->durability_;
    }

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncUpdateGliderDurability(accountId_, getObjectId(), gliderId, durability);
    return ecOk;
}


const BuyBackItemInfoMap Player::queryBuyBackItemInfoMap() const
{
    std::lock_guard<LockType> lock(getLock());

    return buyBackItemMap_;
}


void Player::reset()
{
    accountId_ = invalidAccountId;
    characterInfo_ = nullptr;

    if (session_ != nullptr) {
        sne::sgp::RpcingExtension* extension =
            session_->getImpl().getExtension<sne::sgp::RpcingExtension>();
        getControllerAs<gc::PlayerController>().unregisterRpc(*extension);
        getPlayerMoveController().unregisterRpc(*extension);
        getPlayerSkillController().unregisterRpc(*extension);
        getPlayerInventoryController().unregisterRpc(*extension);
        getPlayerItemController().unregisterRpc(*extension);
        getPlayerQuestController().unregisterRpc(*extension);
        getPlayerTradeController().unregisterRpc(*extension);
        getPlayerHarvestController().unregisterRpc(*extension);
        getPlayerTreasureController().unregisterRpc(*extension);
        getPlayerGraveStoneController().unregisterRpc(*extension);
        getPlayerPartyController().unregisterRpc(*extension);
        getPlayerAnchorController().unregisterRpc(*extension);
        getPlayerGuildController().unregisterRpc(*extension);
        getPlayerNpcController().unregisterRpc(*extension);
        getPlayerCastController().unregisterRpc(*extension);
        getPlayerEffectController().unregisterRpc(*extension);
        getPlayerMailController().unregisterRpc(*extension);
        getPlayerAuctionController().unregisterRpc(*extension);
        getPlayerArenaController().unregisterRpc(*extension);
        getPlayerBuildingController().unregisterRpc(*extension);
        getPlayerOutsideInventoryController().unregisterRpc(*extension);
        getPlayerDeviceController().unregisterRpc(*extension);
        getPlayerTeleportController().unregisterRpc(*extension);
        getPlayerDuelController().unregisterRpc(*extension);
        getPlayerWorldEventController().unregisterRpc(*extension);
        getPlayerAchievementController().unregisterRpc(*extension);
        getPlayerCharacterTitleController().unregisterRpc(*extension);
    }
}


void Player::initPlayerController()
{
    if (! tradeController_.get()) {
        tradeController_.reset(createPlayerTradeController().release());
    }
    tradeController_->initialize();

    if (! itemController_.get()) {
        itemController_.reset(createPlayerItemController().release());
    }
    itemController_->initialize();

    if (! questController_.get()) {
        questController_.reset(createPlayerQuestController().release());
    }
    questController_->initialize();

    if (! inventoryController_.get()) {
        inventoryController_.reset(createPlayerInventoryController().release());
    }
    inventoryController_->initialize();

    if (! harvestController_.get()) {
        harvestController_.reset(createPlayerHarvestController().release());
    }
    harvestController_->initialize();

    if (! treasureController_.get()) {
        treasureController_.reset(createPlayerTreasureController().release());
    }
    treasureController_->initialize();

    if (! graveStoneController_.get()) {
        graveStoneController_.reset(createPlayerGraveStoneController().release());
    }
    graveStoneController_->initialize();

    if (! partyController_.get()) {
        partyController_.reset(createPlayerPartyController().release());
    }
    partyController_->initialize();

    if (! anchorController_.get()) {
        anchorController_.reset(createPlayerAnchorController().release());
    }
    anchorController_->initialize();

    if (! buildingController_.get()) {
        buildingController_.reset(createPlayerBuildingController().release());
    }
    buildingController_->initialize();


    if (! guildController_.get()) {
        guildController_.reset(createPlayerGuildController().release());
    }
    guildController_->initialize();

    if (! npcController_.get()) {
        npcController_.reset(createPlayerNpcController().release());
    }
    npcController_->initialize();

    if (! castEventController_.get()) {
        castEventController_.reset(createPlayerCastController().release());
    }
    castEventController_->initialize();

    if (! mailController_.get()) {
        mailController_.reset(createPlayerMailController().release());
    }
    mailController_->initialize();

    if (! auctionController_.get()) {
        auctionController_.reset(createPlayerAuctionController().release());
    }
    auctionController_->initialize();

    if (! arenaController_.get()) {
        arenaController_.reset(createPlayerArenaController().release());
    }
    arenaController_->initialize();

    if (! outsideInventoryController_.get()) {
        outsideInventoryController_.reset(createPlayerOutsideInventoryController().release());
    }
    outsideInventoryController_->initialize();

    if (! deviceController_.get()) {
        deviceController_.reset(createPlayerDeviceController().release());
    }
    deviceController_->initialize();

    if (! teleportController_.get()) {
        teleportController_.reset(createPlayerTeleportController().release());
    }
    teleportController_->initialize();

    if (! duelController_.get()) {
        duelController_.reset(createPlayerDuelController().release());
    }
    duelController_->initialize();

    if (! worldEventController_.get()) {
        worldEventController_.reset(createPlayerWorldEventController().release());
    }
    worldEventController_->initialize();


    if (! achievementController_.get()) {
        achievementController_.reset(createPlayerAchievementController().release());
    }
    achievementController_->initialize(); 

    if (! characterTitleController_.get()) {
        characterTitleController_.reset(createPlayerCharacterTitleController().release());
    }
    characterTitleController_->initialize(); 
}


void Player::initInventory(MoreCharacterInfo& moreCharacterInfo)
{
    itemCastableAbility_= std::make_unique<ItemCastableAbility>(*this);
    inventory_ = std::make_unique<PlayerInventories>(*this, characterInfo_->inventory_,
        characterInfo_->equipments_, moreCharacterInfo.equipments_,
        characterInfo_->characterAccessories_);
    questInventory_= std::make_unique<QuestInventory>(*this, characterInfo_->questInventory_);
    questInventory_->initialize();
    vehicleInventory_= std::make_unique<Inventory>(*this,
        characterInfo_->vehicleInventory_.inventory_, itVehicle);
}


void Player::initBankAccount()
{
    bankAccount_= std::make_unique<BankAccount>(*this, characterInfo_->bankAccountInfo_);
}


void Player::initStatus(MoreCharacterInfo& moreCharacterInfo)
{
    CreatureStatusInfo& creatureStatus = getCreatureStatus().getCreatureStatusInfo();
    CHARACTER_STATUS_TABLE->applyCreatureStatusInfo(characterInfo_->characterClass_,
        creatureStatus.baseStatus_, characterInfo_->currentLevel_.level_,
        characterInfo_->currentLevel_.levelStep_);
    EQUIP_TABLE->applyCreatureStatusInfo(creatureStatus, characterInfo_->inventory_,
        characterInfo_->equipments_);
    ACCESSORY_TABLE->applyCreatureStatusInfo(creatureStatus, 
        characterInfo_->inventory_, characterInfo_->characterAccessories_);
    getCreatureStatus().updateAllStatus();
    const Points& creatureMaxPoints = getCurrentCreatureStatusInfo().points_;
    if (! moreCharacterInfo.stateInfo_.hasState(cstDead) && characterInfo_->currentPoints_.isMinPoints()) {
        characterInfo_->currentPoints_ = creatureMaxPoints;
    }
    characterInfo_->currentPoints_.clampPoints(creatureMaxPoints);
    moreCharacterInfo.currentPoints_ = characterInfo_->currentPoints_;

    if (playerState_.get() && characterInfo_->currentPoints_.isDied()) {
        playerState_->died();
    }
}


void Player::initQuestRepository()
{
    questRepository_= std::make_unique<QuestRepository>(*this, 
        characterInfo_->completedQuests_, characterInfo_->acceptedQuestInfos_,
        characterInfo_->acceptedRepeatQuestInfos_, characterInfo_->completedRepeatQuestInfos_);
}


bool Player::initSkill()
{
    initCharacterSkillManager();

    if (! skillCastableAbility_.get()) {
        skillCastableAbility_= std::make_unique<SkillCastableAbility>(*this);
    }

    if (! passiveSkillManager_.get()) {
        passiveSkillManager_= std::make_unique<PassiveSkillManager>(*this);
    }

    for (const CreatureSkills::value_type& value : characterInfo_->skills_) {
        const SkillCode skillCode = value;        
        if (sttPassivePlayer == getSkillTableType(skillCode)) {
            passiveSkillManager_->learn(skillCode);
        }
        else {
            skillCastableAbility_->learn(skillCode);
        }
    }

    return true;
}


void Player::initCharacterSkillManager()
{
    if (! characterSkillManager_.get()) {
        characterSkillManager_= std::make_unique<CharacterSkillManager>();
    }
    characterSkillManager_->initialize(characterInfo_->skills_);
}


void Player::initCooldown()
{
    getCoolDownTimer().setCooldown(characterInfo_->cooltimeInfos_);

    for (const RemainEffectInfo& remainEffect : characterInfo_->remainEffects_) {
        getEffectScriptApplier().applyRemainEffect(remainEffect);        
    }
}


void Player::downChaotic_i(Chaotic chaotic)
{
    UnionEntityInfo& unionInfo = getUnionEntityInfo_i();
    MoreCharacterInfo& moreCharacterInfo = unionInfo.asCharacterInfo();
    moreCharacterInfo.downChaotic(chaotic);
    characterInfo_->downChaotic(chaotic);
}


void Player::upChaotic_i(Chaotic chaotic)
{
    UnionEntityInfo& unionInfo = getUnionEntityInfo_i();
    MoreCharacterInfo& moreCharacterInfo = unionInfo.asCharacterInfo();
    moreCharacterInfo.upChaotic(chaotic);
    characterInfo_->upChaotic(chaotic);
    tempChaoInfo_.tempChaotic_ = minChaotic;
}


void Player::restoreSpeedAndScale()
{
    UnionEntityInfo& info = getUnionEntityInfo_i();
    info.asCreatureMoveSpeed() = originalSpeed_;
    info.asCreatureScale() = 1000.0f;

    resetSpeedRate_i();
}


std::unique_ptr<gc::PlayerTradeController> Player::createPlayerTradeController()
{
    return std::make_unique<gc::PlayerTradeController>(this);
}


std::unique_ptr<gc::PlayerItemController> Player::createPlayerItemController()
{
    return std::make_unique<gc::PlayerItemController>(this);
}


std::unique_ptr<gc::PlayerAnchorController> Player::createPlayerAnchorController()
{
    return std::make_unique<gc::PlayerAnchorController>(this);
}


std::unique_ptr<gc::PlayerBuildingController> Player::createPlayerBuildingController()
{
    return std::make_unique<gc::PlayerBuildingController>(this);
}


std::unique_ptr<gc::PlayerOutsideInventoryController> Player::createPlayerOutsideInventoryController()
{
    return std::make_unique<gc::PlayerOutsideInventoryController>(this);
}


std::unique_ptr<gc::PlayerNpcController> Player::createPlayerNpcController()
{
    return std::make_unique<gc::PlayerNpcController>(this);
}


std::unique_ptr<gc::PlayerCastController> Player::createPlayerCastController()
{
    return std::make_unique<gc::PlayerCastController>(this);
}


std::unique_ptr<gc::PlayerMailController> Player::createPlayerMailController()
{
    return std::make_unique<gc::PlayerMailController>(this);
}


std::unique_ptr<gc::PlayerAuctionController> Player::createPlayerAuctionController()
{
    return std::make_unique<gc::PlayerAuctionController>(this);
}

// = Creature overriding

void Player::initState(CreatureStateInfo& stateInfo)
{
    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    MoreCharacterInfo& moreCharacterInfo = entityInfo.asCharacterInfo();

    playerState_.reset(
        new PlayerState(*this, stateInfo, characterInfo_->stateInfo_,
        moreCharacterInfo.entityVehicleInfo_, moreCharacterInfo.gliderCode_));

    if (isChao()) {
        (void)playerState_->changeChaoState(true);
    }    
}


void Player::frenze(int32_t size, bool isRelease)
{
    std::lock_guard<LockType> lock(getLock());

    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    float32_t& creatureSize = entityInfo.asCreatureScale();

    if (isRelease) {
        creatureSize = 1000.0f;
    }
    else {
        creatureSize *= (size / 1000.0f);
    }
}


float32_t Player::getCurrentDefaultSpeed() const 
{
    std::lock_guard<LockType> lock(getLock());

    if (characterInfo_->stateInfo_.hasState(cstKnockBack)) {
        return GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"knockback_speed") / 100.0f;
    }
    else if (characterInfo_->stateInfo_.hasState(cstVehicleMount)) {
        VehicleInfo* info = characterInfo_->vehicleInventory_.getCurrentVehicleInfo();
        if (info) {
            const gdt::vehicle_t* vehicle = VEHICLE_TABLE->getVehicle(info->vehicleCode_);
            assert(vehicle);
            float32_t currentSpeed = info->getCurrentSpeed(vehicle->org_speed(), vehicle->apply_speed_value());
            permil_t addPer = getGuildEffectValue(estMountSpeedUp);
            if (0 < addPer) {
                currentSpeed += currentSpeed * (addPer * 0.001f);
            }
            return currentSpeed;
        }
    }
    else if (characterInfo_->stateInfo_.hasState(cstGliderMount)) {
        const gdt::glider_t* glider = GLIDER_TABLE->getGlider(getGliderCode());
        if (glider) {
            return glider->org_speed();
        }
    }
    return originalSpeed_;
}


const DebuffBuffEffectInfoSet Player::getDebuffBuffEffectInfoSet() const
{
    std::lock_guard<LockType> lock(getLock());

    DebuffBuffEffectInfoSet debuffBuffEffectSet;
    const DebuffBuffEffectInfoSet& activeInfo = static_cast<const CreatureEffectScriptApplier&>(getEffectScriptApplier()).getDebuffBuffEffectInfoSet();
    const DebuffBuffEffectInfoSet& passiveInfo = passiveSkillManager_->getDebuffBuffEffectInfoSet();
    debuffBuffEffectSet.insert(activeInfo.begin(), activeInfo.end());
    debuffBuffEffectSet.insert(passiveInfo.begin(), passiveInfo.end());
    return debuffBuffEffectSet;
}


bips_t Player::getMissChance(const Entity& target) const
{
    std::lock_guard<LockType> lock(getLock());

    const bips_t missChance = Parent::getMissChance(target);

    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    const bips_t hitChance = gideon::getHitChance(statusInfo.attackStatus_[atsiHitRate]);
    return clampBips(missChance - hitChance);
}


bips_t Player::getDodgeChance() const
{
    std::lock_guard<LockType> lock(getLock());

    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return gideon::getDodgeChance(getCreatureLevel(), statusInfo.defenceStatus_[dsiDodgeRate]);
}


bips_t Player::getParryChance() const
{
    if (! queryWeaponUseable()->isMeleeWeaponEquipped()) {
        return 0;
    }

    std::lock_guard<LockType> lock(getLock());

    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return gideon::getParryChance(getCreatureLevel(), statusInfo.defenceStatus_[dsiParryRate]);
}


bips_t Player::getBlockChance() const
{
    if (! queryWeaponUseable()->isShieldEquipped()) {
        return 0;
    }

    std::lock_guard<LockType> lock(getLock());

    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return gideon::getBlockChance(getCreatureLevel(), statusInfo.defenceStatus_[dsiBlockRate]);
}


bips_t Player::getPhysicalCriticalChance() const
{
    std::lock_guard<LockType> lock(getLock());

    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return gideon::getPhysicalCriticalChance(getCreatureLevel(), statusInfo.attackStatus_[atsiPhysicalCriticalRate]);
}


bips_t Player::getMagicCriticalChance() const
{
    std::lock_guard<LockType> lock(getLock());

    const CreatureStatusInfo& statusInfo = getCurrentCreatureStatusInfo();
    return gideon::getMagicCriticalChance(getCreatureLevel(), statusInfo.attackStatus_[atsiMagicCriticalRate]);
}

// = Entity overriding

ErrorCode Player::spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition)
{
    const ErrorCode errorCode = Parent::spawn(worldMap, spawnPosition);
    if (isSucceeded(errorCode)) {
        queryGliderState()->dismountGlider();
        queryVehicleState()->dismountVehicle();
        if (! arena_) {
            const GameTime activateTime = 7000;
            getEffectScriptApplier().applyEffectScript(estSpawnProtection, activateTime);
        }
    }
    return errorCode;
}


ErrorCode Player::respawn(WorldMap& /*worldMap*/)
{
    assert(false && "DON'T CALL ME!");
    return ecServerInternalError;
}


ErrorCode Player::despawn()
{
    return Parent::despawn();
}


ObjectPosition Player::getNextSpawnPosition() const
{
    assert(false && "DON'T CALL ME!");
    return ObjectPosition();
}


std::unique_ptr<EffectHelper> Player::createEffectHelper()
{
    return std::make_unique<PlayerEffectHelper>(*this);
}

// = Questable overriding

void Player::killed(go::Entity& victim)
{
    const GameObjectInfo& objectInfo = victim.getGameObjectInfo();
    if (! victim.isSame(victim.getGameObjectInfo())) {
        if (! isMyDueler(victim) && shouldApplyChao(victim)) {
            bool isPreChao = false;
            {
                std::lock_guard<LockType> lock(getLock());

                isPreChao = characterInfo_->chaotic_ > 0;
                upChaotic_i(killChaotic);            
            }
            gc::StatsCallback* statsCallback = getController().queryStatsCallback();
            if (statsCallback) {
                statsCallback->chaoticUpdated(getChaotic());
            }
            if (! isPreChao) {
                gc::ChaoCallback* callback = getController().queryChaoCallback();
                if (callback) {
                    callback->changeChao(true);
                }
            }
        }
    }

    Chaotic victimChaotic = minChaotic;
    Chaoable* victimChaoable = victim.queryChaoable();
    if (victimChaoable) {
        victimChaotic = victimChaoable->getChaotic();
    }

    questRepository_->killed(objectInfo.objectType_, victim.getEntityCode(), victimChaotic);
    if (victim.isNpcOrMonster()) {
        updateAchievement(acmtHunter, &victim);
    }
    else if (victim.isPlayer()) {
        updateAchievement(acmtPvPKill, &victim);
        victim.queryAchievementable()->updateAchievement(acmtPvPDeath, this);
    }
}


ErrorCode Player::canAddItem(QuestCode& questCode, QuestItemCode questItemCode) const
{
    return questRepository_->canAddItem(questCode, questItemCode);
}


std::unique_ptr<gc::PlayerQuestController> Player::createPlayerQuestController()
{
    return std::make_unique<gc::PlayerQuestController>(this);
}


std::unique_ptr<gc::PlayerTeleportController> Player::createPlayerTeleportController()
{
    return std::make_unique<gc::PlayerTeleportController>(this);
}


std::unique_ptr<gc::PlayerHarvestController> Player::createPlayerHarvestController()
{
    return std::make_unique<gc::PlayerHarvestController>(this);
}


std::unique_ptr<gc::PlayerTreasureController> Player::createPlayerTreasureController()
{
    return std::make_unique<gc::PlayerTreasureController>(this);
}


std::unique_ptr<gc::PlayerDeviceController> Player::createPlayerDeviceController()
{
    return std::make_unique<gc::PlayerDeviceController>(this);
}

// = Inventoryable overriding

std::unique_ptr<gc::PlayerInventoryController> Player::createPlayerInventoryController()
{
    return std::make_unique<gc::PlayerInventoryController>(this);
}

// = GraveStoneable overriding

std::unique_ptr<gc::PlayerGraveStoneController> Player::createPlayerGraveStoneController()
{
    return std::make_unique<gc::PlayerGraveStoneController>(this);
}

// = ArenaMatchable overriding

std::unique_ptr<gc::PlayerArenaController> Player::createPlayerArenaController()
{
    return std::make_unique<gc::PlayerArenaController>(this);
}


gc::PlayerMoveController& Player::getPlayerMoveController() 
{
    return static_cast<gc::PlayerMoveController&>(getMoveController());
}


gc::PlayerSkillController& Player::getPlayerSkillController()
{
    return static_cast<gc::PlayerSkillController&>(getSkillController());
}


const gc::PlayerMoveController& Player::getPlayerMoveController() const 
{
    return static_cast<const gc::PlayerMoveController&>(getMoveController());
}


gc::PlayerEffectController& Player::getPlayerEffectController() 
{
    return static_cast<gc::PlayerEffectController&>(getEffectController());
}


const gc::PlayerEffectController& Player::getPlayerEffectController() const 
{
    return static_cast<const gc::PlayerEffectController&>(getEffectController());
}

// = EntityStateAbility overriding

MoveState* Player::queryMoveState()
{
    return playerState_.get();
}


CreatureState* Player::queryCreatureState()
{
    return playerState_.get();
}


SkillCasterState* Player::querySkillCasterState()
{
    return playerState_.get();
}


TradeState* Player::queryTradeState()
{
    return playerState_.get();
}


ItemManageState* Player::queryItemManageState()
{
    return playerState_.get();
}


HarvestState* Player::queryHarvestState()
{
    return playerState_.get();
}


TreasureState* Player::queryTreasureState()
{
    return playerState_.get();
}


GliderState* Player::queryGliderState()
{
    return playerState_.get();
}


VehicleState* Player::queryVehicleState()
{
    return playerState_.get();
}


ChaoState* Player::queryChaoState()
{
    return playerState_.get();
}


CastState* Player::queryCastState()
{
    return playerState_.get();
}


CombatState* Player::queryCombatState()
{
    return playerState_.get();
}


FieldDuelState* Player::queryFieldDuelState()
{
    return playerState_.get();
}


const FieldDuelState* Player::queryFieldDuelState() const
{
    return playerState_.get();
}

// = Chaoable overriding

bool Player::isChao() const
{
    std::lock_guard<LockType> lock(getLock());

    if (characterInfo_) {
        return characterInfo_->chaotic_ > 0 ||
            tempChaoInfo_.isChao();
    }
    return false;
}


bool Player::isTempChao() const
{
    std::lock_guard<LockType> lock(getLock());

    if (characterInfo_) {
        return tempChaoInfo_.isChao();
    }
    return false;
}


bool Player::isRealChao() const
{
    std::lock_guard<LockType> lock(getLock());

    if (characterInfo_) {
        return characterInfo_->chaotic_ > 0;
    }
    return false;
}


void Player::hitToPlayer()
{
    bool preChaoPlayer = false;
    {
        std::lock_guard<LockType> lock(getLock());

        if (updateChaobleTime_ > GAME_TIMER->msec()) {
            return;
        }
        preChaoPlayer = tempChaoInfo_.isChao();
        tempChaoInfo_.hit();
    }
    if (! preChaoPlayer) {
        gc::ChaoCallback* chaoCallback = getController().queryChaoCallback();
        if (chaoCallback) {
            chaoCallback->changeTempChao(true);
        }
    }
}


void Player::standGraveStone()
{
    bool preChaoPlayer = false;
    {
        std::lock_guard<LockType> lock(getLock());

        preChaoPlayer = characterInfo_->chaotic_ > 0;
        upChaotic_i(graveStoneChaotic);
    }
    gc::StatsCallback* callback = getController().queryStatsCallback();
    if (callback) {
        callback->chaoticUpdated(characterInfo_->chaotic_);
    }
    if (! preChaoPlayer) {
        gc::ChaoCallback* chaoCallback = getController().queryChaoCallback();
        if (chaoCallback) {
            chaoCallback->changeChao(true);
        }
    }
}


// = Inventoryable overriding
bool Player::canExtendSlot(bool isCashSlot, uint8_t extendCount) const
{
    std::lock_guard<LockType> lock(getLock());

    return characterInfo_->inventory_.canExtendSlot(isCashSlot, extendCount);
}


uint8_t Player::extendSlot(bool isCashSlot, uint8_t extendCount)
{
    std::lock_guard<LockType> lock(getLock());

    characterInfo_->inventory_.extendSlot(isCashSlot, extendCount);
    return isCashSlot ? characterInfo_->inventory_.cashSlotCount_ : characterInfo_->inventory_.freeSlotCount_;
}

// = Partyable overriding

bool Player::isPartyMember() const
{
    std::lock_guard<LockType> lock(getLock());

    return party_.get() != nullptr;
}


bool Player::isSameParty(PartyId partyId) const
{
    std::lock_guard<LockType> lock(getLock());

    if (party_.get() == nullptr) {
        return false;
    }

    return party_->getPartyId() == partyId;
}


void Player::setParty(PartyPtr party)
{
    std::lock_guard<LockType> lock(getLock());

    party_ = party;
}


PartyPtr Player::getParty()
{
    std::lock_guard<LockType> lock(getLock());

    return party_;
}


const PartyPtr Player::getParty() const
{
    std::lock_guard<LockType> lock(getLock());

    return party_;
}


std::unique_ptr<gc::PlayerPartyController> Player::createPlayerPartyController()
{
    return std::make_unique<gc::PlayerPartyController>(this);
}


std::unique_ptr<gc::PlayerDuelController> Player::createPlayerDuelController()
{
    return std::make_unique<gc::PlayerDuelController>(this);
}


std::unique_ptr<gc::PlayerWorldEventController> Player::createPlayerWorldEventController()
{
    return std::make_unique<gc::PlayerWorldEventController>(this);
}


std::unique_ptr<gc::PlayerAchievementController> Player::createPlayerAchievementController()
{
    return std::make_unique<gc::PlayerAchievementController>(this);
}


std::unique_ptr<gc::PlayerCharacterTitleController> Player::createPlayerCharacterTitleController()
{
    return std::make_unique<gc::PlayerCharacterTitleController>(this);
}

// = Positionable overriding

void Player::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);

    characterInfo_->position_ = position;
}


void Player::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);

    characterInfo_->position_.heading_ = heading;
}

// = Moveable overriding

std::unique_ptr<gc::MoveController> Player::createMoveController()
{
    return std::make_unique<gc::PlayerMoveController>(this);
}


// = Liveable overriding

std::unique_ptr<CreatureStatus> Player::createCreatureStatus()
{
    return std::make_unique<PlayerStatus>(*this, getUnionEntityInfo_i(), *characterInfo_);
}

// = SkillCastable overriding

ErrorCode Player::castTo(const GameObjectInfo& targetInfo, SkillCode skillCode)
{
    return skillCastableAbility_->castTo(targetInfo, skillCode);
}


ErrorCode Player::castAt(const Position& targetPosition, SkillCode skillCode)
{
    return skillCastableAbility_->castAt(targetPosition, skillCode);
}


void Player::cancel(SkillCode skillCode)
{
    skillCastableAbility_->cancel(skillCode);
}


void Player::cancelConcentrationSkill(SkillCode skillCode)
{
    skillCastableAbility_->cancelConcentrationSkill(skillCode);
}


void Player::cancelAll()
{
    skillCastableAbility_->cancelAll();
}


void Player::consumePoints(const Points& points)
{   
    if (points.hp_ > hpMin) {
        getCreatureStatus().reduceHp(points.hp_, false);
    }

    if (points.mp_ > mpMin) {
        getCreatureStatus().reduceMp(points.mp_);
    }
}


void Player::consumeMaterialItem(const BaseItemInfo& itemInfo)
{
    inventory_->useItemsByBaseItemInfo(itemInfo);
}


ErrorCode Player::checkSkillCasting(SkillCode skillCode,
    const GameObjectInfo& targetInfo) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetInfo);
}


ErrorCode Player::checkSkillCasting(SkillCode skillCode,
    const Position& targetPosition) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetPosition);
}


void Player::notifyChangeCondition(PassiveCheckCondition condition)
{
    passiveSkillManager_->notifyChangeCondition(condition);
}


float32_t Player::getLongestSkillDistance() const
{
    return skillCastableAbility_->getLongestSkillDistance();
}


bool Player::isUsing(SkillCode skillCode) const
{
    return skillCastableAbility_->isUsing(skillCode);
}


bool Player::canCast(SkillCode skillCode) const
{
    return skillCastableAbility_->canCast(skillCode);
}


// = CastCheckable overriding
bool Player::checkCastableNeedSkill(SkillCode needSkillCode) const
{
    if (isValidSkillCode(needSkillCode)) {
        //SkillIndex needIndex = getSkillIndex(needSkillCode);

        SkillCode skillCode = invalidSkillCode;
        {
            std::lock_guard<LockType> lock(getLock());

            SkillIndex index = getSkillIndex(needSkillCode);
            SkillTableType tableType = getSkillTableType(needSkillCode);

            skillCode = characterSkillManager_->getCurrentSkillCode(tableType, index);
        }
        if (! isValidSkillCode(skillCode)) {
            return false;
        }

        const SkillLevel needLevel = getSkillLevel(needSkillCode);
        if (needLevel > getSkillLevel(skillCode)) {
            return false;
        }   
    }

    return true;
}


bool Player::checkCastableUsableState(SkillUseableState needUsableState) const
{
    return checkUseableState(needUsableState, playerState_->getCreatureStateType());    
}


bool Player::checkCastableEquip(EquipPart checekEquipPart, SkillCastableEquipType checkAllowedType) const
{
    std::lock_guard<LockType> lock(getLock());

    const EquipPartTypeMap& equipPartTypeMap = inventory_->getEquipPartTypeMap();
    return isCastableSkillFromEquip(checekEquipPart, checkAllowedType, equipPartTypeMap);    
}


bool Player::checkCastableNeedItem(const BaseItemInfo& needItem) const
{
    std::lock_guard<LockType> lock(getLock());

    return characterInfo_->inventory_.hasEnoughItem(needItem.itemCode_, needItem.count_);
}


bool Player::checkCastableEffectCategory(EffectStackCategory category) const
{
    return getEffectScriptApplier().hasEffectStackCategory(category);
}


ErrorCode Player::checkCastablePoints(PointType pt, bool isPercent,
    bool isCheckUp, uint32_t checkValue) const
{
    std::lock_guard<LockType> lock(getLock());

    if (gideon::isValid(pt)) {
        const CreatureStatusInfo& gameStats = getCreatureStatus().getCurrentCreatureStatusInfo();

        uint32_t currentValue = 0;         
        if (pt == ptHp) {
            currentValue = isPercent ? 
                getPercentValue(characterInfo_->currentPoints_.hp_, gameStats.points_.hp_) : 
            characterInfo_->currentPoints_.hp_;
        }
        else if (pt == ptMp) {
            currentValue = isPercent ? 
                getPercentValue(characterInfo_->currentPoints_.mp_, gameStats.points_.mp_) : 
                characterInfo_->currentPoints_.mp_;
        }
        else {
            assert(false);
        }

        if (isCheckUp) {
            if (currentValue <= checkValue) {
                return ecSkillNotEnoughStats;
            }
        }
        else {
            if (currentValue >= checkValue) {
                return ecSkillOverStats;
            }
        }
    }

    return ecOk;
}

// = Rewardable overriding

void Player::expReward(bool& isLevelUp, bool& isMajorLevelUp, ExpPoint rewardExp)
{
    CurrentLevel orgLevel;;
    {
        std::lock_guard<LockType> lock(getLock());

        if (getCreatureStatus().isDied()) {
            return;
        }

        permil_t bonusPer = guildSkillReposity_->getEffectValue(estExpBonus);
        if (0 < bonusPer) {
            rewardExp = toExpPoint(static_cast<uint32_t>(rewardExp + (rewardExp * (bonusPer * 0.001f))));
        }

        const GuildId guildId = getGuildId();
        if (isValidGuildId(guildId)) {
            GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
            if (guild.get() && guild->canSaveExp()) {
                getController().queryGuildCallback()->addGuildExp(static_cast<GuildExp>(rewardExp * 0.1f));
            }
        }

        if (characterInfo_->currentLevel_.isMaxLevel()) {
            return;
        }

        orgLevel = characterInfo_->currentLevel_;
        bool isDone = false;
        ExpPoint leftExp = toExpPoint(characterInfo_->exp_ + rewardExp);

        // 현재 레벨에 더한다.
        for (int level = characterInfo_->currentLevel_.level_; level < clCount; ++level) {
            const CreatureLevel creatureLevel = toCreatureLevel(level);
            const SkillPoint addSkillPoint = spLevelStepupRewardPoint;
            for (int levelStep = characterInfo_->currentLevel_.levelStep_;
                levelStep < lsCount; ++levelStep) {
                    LevelStep nextLevelStep = toLevelStep(levelStep + 1);
                    if (! gideon::isValid(nextLevelStep)) {
                        nextLevelStep = lsMinLevel;
                    }

                    if (characterInfo_->currentLevel_.isMaxLevel()) {
                        leftExp = ceMin;
                        isDone = true;
                        break;
                    }

                    const ExpPoint neededExp = EXP_TABLE->getExpPoint(creatureLevel,
                        characterInfo_->currentLevel_.levelStep_);
                    if (leftExp < neededExp) {
                        isDone = true;
                        break;
                    }
                    leftExp = toExpPoint(leftExp - neededExp);

                    characterInfo_->currentLevel_.levelStep_ = nextLevelStep;
                    characterInfo_->sp_ = toSkillPoint(characterInfo_->sp_ + addSkillPoint);
                    isLevelUp = true;
            }

            if (isDone) {
                break;
            }

            const CreatureLevel nextLevel = toCreatureLevel(creatureLevel + 1);
            if (gideon::isValidPlayerLevel(nextLevel)) {
                characterInfo_->currentLevel_.level_ = nextLevel;
                isMajorLevelUp = true;
            }
        }

        if (characterInfo_->isBeginnerProtection_ && 
            ! isBeginnerProtectionLevel(characterInfo_->currentLevel_.level_)) {
                releaseBeginnerProtection(true);
        }

        characterInfo_->exp_ = leftExp;
    }

    if (isLevelUp) {
        getCreatureStatus().levelUp(orgLevel);   
        getControllerAs<gc::PlayerController>().evPlayerLeveledUpInfo(
            characterInfo_->currentLevel_, characterInfo_->exp_, rewardExp, characterInfo_->sp_);
    }
    else {
        getControllerAs<gc::PlayerController>().evExpPointUpdated(characterInfo_->exp_);
    }	

    if (isMajorLevelUp) {
        updateAchievement(acmtLevel, nullptr);
        ZONE_SERVICE->getCommunityServerProxy().z2m_updateCreatureLevel(getAccountId(), getObjectId(), characterInfo_->currentLevel_.level_);
    }
    
}


ErrorCode Player::addRewardSkill(SkillCode skillCode)
{    
    const SkillIndex index = getSkillIndex(skillCode);
    const SkillTableType tableType = getSkillTableType(skillCode);
    if (sttActivePlayer == tableType) {
        if (PLAYER_ACTIVE_SKILL_TABLE->getPlayerSkill(skillCode) == nullptr) {
            return ecSkillInvalidSkill;
        }
    }
    else if (sttPassivePlayer == tableType) {
        if (PLAYER_PASSIVE_SKILL_TABLE->getPlayerSkill(skillCode) == nullptr) {
            return ecSkillInvalidSkill;
        }
    }
    else {
        return ecSkillInvalidSkill;
    }

    std::lock_guard<LockType> lock(getLock());

    const SkillCode currentSkillCode = 
        characterSkillManager_->getCurrentSkillCode(tableType, index);

    if (sttActivePlayer == tableType && isValidSkillCode(currentSkillCode)) {
        skillCastableAbility_->cancel(currentSkillCode);
        skillCastableAbility_->unlearn(skillCode);
    }

    if (sttPassivePlayer == tableType) {
        passiveSkillManager_->learn(skillCode);
    }
    else {
        skillCastableAbility_->learn(skillCode);
    }

    characterSkillManager_->updateSkill(skillCode);
    notifyChangeCondition(pccLearnSkill);

    getPlayerSkillController().skillLearned(currentSkillCode, skillCode);

    return ecOk;
}


ErrorCode Player::addCharacterTitle(CharacterTitleCode titleCode)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (characterTitleCodeSet_.find(titleCode) != characterTitleCodeSet_.end()) {
            return ecTitleAlreadyCharacterTitle;
        }
        characterTitleCodeSet_.insert(titleCode);
    }
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncAddCharacterTitle(accountId_, getObjectId(), titleCode); 
    }

    return ecOk;
}


// = ItemCastable overriding

ErrorCode Player::castTo(const GameObjectInfo& targetInfo, ObjectId objectId)
{
    return itemCastableAbility_->castTo(targetInfo, objectId);
}


ErrorCode Player::castAt(const Position& targetPosition, ObjectId objectId)
{
    return itemCastableAbility_->castAt(targetPosition, objectId);
}


void Player::insertCastItem(ObjectId itemId, DataCode dataCode)
{
    itemCastableAbility_->insertCastItem(itemId, dataCode);
}


void Player::removeCastItem(ObjectId itemId)
{
    itemCastableAbility_->removeCastItem(itemId);
}


void Player::cancel(ObjectId objectId)
{
    itemCastableAbility_->cancel(objectId);
}

ErrorCode Player::checkItemCasting(ObjectId itemId,
    const GameObjectInfo& targetInfo) const
{
    if (arena_) {
        DataCode itemCode = inventory_->getItemCode(itemId);
        if (isBindRecallItem(itemCode)) {
            return ecItemUseNotBindRecallItemInArena;
        }
    }
    return itemCastableAbility_->checkItemCasting(itemId, targetInfo);
}


ErrorCode Player::checkItemCasting(ObjectId itemId,
    const Position& targetPosition) const
{
    if (arena_) {
        DataCode itemCode = inventory_->getItemCode(itemId);
        if (isBindRecallItem(itemCode)) {
            return ecItemUseNotBindRecallItemInArena;
        }
    }

    return itemCastableAbility_->checkItemCasting(itemId, targetPosition);
}


bool Player::checkQuestItem(ObjectId objectId) const
{
    return questInventory_->hasUseableCount(objectId);
}


void Player::useQuestItem(ObjectId objectId)
{
    questInventory_->useItemCount(objectId);
}


bool Player::checkElementItem(ObjectId objectId) const
{
    ItemInfo itemInfo = inventory_->getItemInfo(objectId);
    if (! itemInfo.isValid()) {
        return false;
    }

    if (! isElementType(getCodeType(itemInfo.itemCode_))) {
        return false;
    }

    return itemInfo.count_ >= 1;
}


void Player::useElementItem(ObjectId itemId)
{
    inventory_->useItemsByItemId(itemId, 1);
}

// = Tickable overriding

void Player::tick(GameTime diff)
{
    {
        EffectScriptApplier& effectScriptApplier = getEffectScriptApplier();
        if (queryCreatureState()->isDied()) {
            effectScriptApplier.revert();
            passiveSkillManager_->revert();
        }

        effectScriptApplier.tick();
        passiveSkillManager_->tick();
    }

    if (queryMoveState()->isControlAbsorbed()) {
        getMovementManager().updateMovement(diff);
        getMoveController().update();
    }

    {
        oneSecondTracker_.update(diff);
        if (! oneSecondTracker_.isPassed()) {
            return;
        }
        oneSecondTracker_.reset(1000);

        handleExpiredTasks();
    }
}


std::unique_ptr<gc::SkillController> Player::createSkillController()
{
    return std::make_unique<gc::PlayerSkillController>(this);
}


std::unique_ptr<gc::CreatureEffectController> Player::createEffectController()
{
    return std::make_unique<gc::PlayerEffectController>(this);
}


std::unique_ptr<gc::PlayerGuildController> Player::createPlayerGuildController()
{
    return std::make_unique<gc::PlayerGuildController>(this);
}

// = TargetSelectable overriding

const Entity* Player::selectTarget(const GameObjectInfo& entityInfo)
{
    GameObjectInfo watchedEntityInfo;
    WorldMap* worldMap = nullptr;
    {
        std::lock_guard<LockType> lock(getLock());

        if (selectedTargetInfo_ == entityInfo) {
            return nullptr;
        }
        watchedEntityInfo = selectedTargetInfo_;
        selectedTargetInfo_ = entityInfo;
        worldMap = getCurrentWorldMap();
    }

    if (! worldMap) {
        return nullptr;
    }

    if (watchedEntityInfo.isValid()) {
        Entity* entity = worldMap->getEntity(watchedEntityInfo);
        if (entity != nullptr) {
            entity->unregisterObserver(getGameObjectInfo()); 
        }
    }

    if (! entityInfo.isValid()) {
        return nullptr;
    }

    Entity* target = queryKnowable()->getEntity(entityInfo);
    if (! target) {
        return nullptr;
    }

    target->registerObserver(getGameObjectInfo());

    EntityEvent::Ref targetChangedEvent(
        new TargetChangedEvent(getGameObjectInfo(), WORLD->getEntityStatusInfo(target)));
    notifyToObervers(targetChangedEvent, true);

    go::TargetSelectable* targetSelectable = target->queryTargetSelectable();
    if (! targetSelectable) {
        return nullptr;
    }

    const go::Entity* targetOfTarget = targetSelectable->getSelectedTarget();
    {
        std::lock_guard<LockType> lock(getLock());

        selectedTargetOfTargetInfo_ =
            (targetOfTarget != nullptr) ? targetOfTarget->getGameObjectInfo() : GameObjectInfo::null();
    }
    return targetOfTarget;
}


void Player::unselectTarget()
{
    GameObjectInfo prevTargetInfo;
    WorldMap* worldMap = nullptr;
    {
        std::lock_guard<LockType> lock(getLock());

        prevTargetInfo = selectedTargetInfo_;
        selectedTargetInfo_.reset();
        worldMap = getCurrentWorldMap();
    }

    if (! worldMap) {
        return;
    }

    if (! prevTargetInfo.isValid()) {
        return;
    }

    {
        std::lock_guard<LockType> lock(getLock());

        selectedTargetOfTargetInfo_.reset();
    }

    Entity* prevTarget = worldMap->getEntity(prevTargetInfo);
    if (prevTarget != nullptr) {
        prevTarget->unregisterObserver(getGameObjectInfo()); 
    }

    EntityEvent::Ref targetChangedEvent(
        new TargetChangedEvent(getGameObjectInfo(), EntityStatusInfo()));
    notifyToObervers(targetChangedEvent, true);
}


go::Entity* Player::getSelectedTarget()
{
    const GameObjectInfo& targetInfo = getSelectedTargetInfo();
    if (! targetInfo.isValid()) {
        return nullptr;
    }
    return queryKnowable()->getEntity(targetInfo);
}


const GameObjectInfo& Player::getSelectedTargetInfo() const
{
    std::lock_guard<LockType> lock(getLock());

    return selectedTargetInfo_;
}


// = Cheatable overriding

ErrorCode Player::updateCheatGameMoney(ValueChangetype type, GameMoney value)
{
    if (type == vctUp) {
        upGameMoney(value);
    }
    else if (type == vctDown) {
        downGameMoney(value);
    }
    else if (type == vctSet) {
        setGameMoney(value);
    }
    else {
        return ecCheatInvalidParam;
    }
    getControllerAs<gc::PlayerController>().evCheatValueTypeUpdated(cvtGameMoney, characterInfo_->gameMoney_);
    return ecOk;
}


ErrorCode Player::updateCheatArenaPoint(ValueChangetype type, ArenaPoint value)
{
    if (type == vctUp) {
        upArenaPoint(value);
    }
    else if (type == vctDown) {
        downArenaPoint(value);
    }
    else if (type == vctSet) {
        setArenaPoint(value);
    }
    else {
        return ecCheatInvalidParam;
    }

    getControllerAs<gc::PlayerController>().evCheatValueTypeUpdated(cvtArenaPoint, characterInfo_->arenaPoint_);
    return ecOk;
}


ErrorCode Player::updateCheatEventCoin(ValueChangetype type, EventCoin value)
{
    if (type == vctUp) {
        upEventCoin(value);
    }
    else if (type == vctDown) {
        downEventCoin(value);
    }
    else if (type == vctSet) {
        setEventCoin(value);
    }
    else {
        return ecCheatInvalidParam;
    }

    getControllerAs<gc::PlayerController>().evCheatValueTypeUpdated(cvtEventCoin, characterInfo_->arenaPoint_);
    return ecOk;
}


ErrorCode Player::updateCheatForgeCoin(ValueChangetype type, ForgeCoin value)
{
    if (type == vctUp) {
        upForgeCoin(value);
    }
    else if (type == vctDown) {
        downForgeCoin(value);
    }
    else if (type == vctSet) {
        setForgeCoin(value);
    }
    else {
        return ecCheatInvalidParam;
    }

    getControllerAs<gc::PlayerController>().evCheatValueTypeUpdated(cvtForgeCoin, characterInfo_->arenaPoint_);
    return ecOk;
}



ErrorCode Player::updateCheatExp(ValueChangetype type, ExpPoint value)
{
    if (type == vctUp) {
        gc::CheatCallback* cheatCallback = getController().queryCheatCallback();
        if (cheatCallback) {
            cheatCallback->cheatRewardExp(value);
        }
    }
    else {
        return ecCheatInvalidParam;
    }
    return ecOk;
}


ErrorCode Player::updateCheatSpeed(ValueChangetype type, float32_t value)
{
    float32_t orgSpeed = getSpeed();
    if (type == vctUp) {
        originalSpeed_ += value;
    }
    else if (type == vctDown) {
        if (originalSpeed_ > value) {
            originalSpeed_ -= value;
        }
        else {
            originalSpeed_ = 0.1f;
        }
    }
    else if (type == vctSet) {
        if (0.1f > 0) {
            originalSpeed_ = 0.1f;
        }
        originalSpeed_ = value;
    }
    else {
        return ecCheatInvalidParam;
    }
    
    gc::CreatureEffectCallback* callback = getController().queryCreatureEffectCallback();
    if (callback) {
        callback->changeMoveSpeed(orgSpeed);
    }

    return ecOk;
}


ErrorCode Player::updateCheatLevel(ValueChangetype type, bool isMajorLevel, uint8_t value)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (type == vctSet) {
            if (isMajorLevel) {
                if (toCreatureLevel(value) <= clMaxPlayerLevel) {
                    characterInfo_->currentLevel_.level_ = toCreatureLevel(value);
                }
                else {
                    characterInfo_->currentLevel_.level_ = clMaxPlayerLevel; 
                }
            }
            else {
                if (toLevelStep(value) < lsMaxLevel ) {
                    characterInfo_->currentLevel_.levelStep_ = toLevelStep(value);
                }
                else {
                    characterInfo_->currentLevel_.levelStep_ = toLevelStep(lsMaxLevel - 1); 
                }
            }
        }
        else {
            return ecCheatInvalidParam;
        }
    }
    getControllerAs<gc::PlayerController>().evCheatValueTypeUpdated(isMajorLevel ? cvtMajorLevel : cvtMinerLevel, characterInfo_->arenaPoint_);
    return ecOk;
}


ErrorCode Player::updateCheatSkillPoint(ValueChangetype type, SkillPoint value)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (type == vctUp) {
            SkillPoint skillPoint = toSkillPoint(characterInfo_->sp_ + value);
            if (skillPoint < spMax) {
                characterInfo_->sp_ = skillPoint;
            }
            else {
                characterInfo_->sp_ = spMax;
            }
        }
        else if (type == vctDown) {
            if (characterInfo_->sp_ < value) {
                characterInfo_->sp_ = spMin;
            }
            else {
                characterInfo_->sp_ = toSkillPoint(characterInfo_->sp_ - value);
            }
        }
        else if (type == vctSet) {
            if (gideon::isValid(value)) {
                characterInfo_->sp_ = value;
            }
        }
        else {
            return ecCheatInvalidParam;
        }
    }

    getControllerAs<gc::PlayerController>().evCheatValueTypeUpdated(cvtSkillPoint, characterInfo_->arenaPoint_);
    return ecOk;	
}


ErrorCode Player::updateCheatPoint(ValueChangetype type, PointType pointType, uint32_t value)
{
    if (pointType == ptHp) {
        if (type == vctUp) {
            getCreatureStatus().fillHp(toHitPoint(value));
        }
        else if (type == vctDown) {
            getCreatureStatus().reduceHp(toHitPoint(value), true);
        }
        else if (type == vctSet) {
            getCreatureStatus().setHp(toHitPoint(value), true);
        }
        if (getCreatureStatus().isDied()) {
            gc::LifeStatusCallback* lifeStatusCallback = 
                getController().queryLifeStatusCallback();
            if (lifeStatusCallback) {
                lifeStatusCallback->died(this);
            }
        }
    }
    else if (pointType == ptMp) {
        if (type == vctUp) {
            getCreatureStatus().fillMp(toManaPoint(value));
        }
        else if (type == vctDown) {
            getCreatureStatus().reduceMp(toManaPoint(value));
        }
        else if (type == vctSet) {
            getCreatureStatus().setMp(toManaPoint(value));
        }
    }

    return ecOk;
}


ErrorCode Player::updateChao(ValueChangetype type, bool isTempChao, Chaotic value)
{
    if (type == vctUp) {
        if (isTempChao) {
            tempChaoInfo_.upChaotic(value);
        }
        else {
            upChaotic(value);
        }
    }
    else if (type == vctDown) {
        if (isTempChao) {
            tempChaoInfo_.downChaotic(value);
        }
        else {
            downChaotic(value);
        }		
    }

    else if (type == vctSet) {
        if (isTempChao) {
            tempChaoInfo_.downChaotic(value);
        }
        else {
            characterInfo_->chaotic_ = toChaotic(value);
        }
    }
    else {
        return ecCheatInvalidParam;
    }

    if (isTempChao) {
        gc::ChaoCallback* chaoCallback = getController().queryChaoCallback();
        if (chaoCallback) {
            chaoCallback->changeTempChao(tempChaoInfo_.isChao());
        }             
    }
    else {
        gc::ChaoCallback* chaoCallback = getController().queryChaoCallback();
        if (chaoCallback) {
            chaoCallback->changeChao(characterInfo_->chaotic_ > 0);
        }               
    }
    
    getControllerAs<gc::PlayerController>().evCheatValueTypeUpdated(isTempChao ? cvtTempChao : cvtChao, 
        isTempChao ? tempChaoInfo_.tempChaotic_ : characterInfo_->chaotic_);
    return ecOk;	
}


ErrorCode Player::setPlayerStatus(CheatValueType valueType, int32_t value)
{
    PlayerStatus& playerStatus = static_cast<go::PlayerStatus&>(getCreatureStatus());
    switch (valueType) {
    case cvtStrength:
        playerStatus.setCurrentBaseStatus(bsiStrength, value);
        break;
    case cvtDexterity:
        playerStatus.setCurrentBaseStatus(bsiDexterity, value);
        break;
    case cvtIntellect:
        playerStatus.setCurrentBaseStatus(bsiIntellect, value);
        break;
    case cvtEnergy:
        playerStatus.setCurrentBaseStatus(bsiEnergy, value);
        break;
    case cvtMinAttack:
        playerStatus.setCurrentAttackPowers(apiMinAttack, value);
        break;
    case cvtMaxAttack:
        playerStatus.setCurrentAttackPowers(apiMaxAttack, value);
        break;
    case cvtMagic:
        playerStatus.setCurrentAttackPowers(apiMagic, value);
        break;
    case cvtPhysical:
        playerStatus.setCurrentAttackPowers(apiPhysical, value);
        break;
    case cvtHitRate:
        playerStatus.setCurrentAttackStatus(atsiHitRate, static_cast<int16_t>(value));
        break;
    case cvtPhysicalCriticalRate:
        playerStatus.setCurrentAttackStatus(atsiPhysicalCriticalRate, static_cast<int16_t>(value));
        break;
    case cvtMagicCriticalRate:
        playerStatus.setCurrentAttackStatus(atsiMagicCriticalRate, static_cast<int16_t>(value));
        break;
    case cvtParryRate:
        playerStatus.setCurrentDefenceStatus(dsiParryRate, static_cast<int16_t>(value));
        break;
    case cvtBlockRate:
        playerStatus.setCurrentDefenceStatus(dsiBlockRate, static_cast<int16_t>(value));
        break;
    case cvtDodgeRate:
        playerStatus.setCurrentDefenceStatus(dsiDodgeRate, static_cast<int16_t>(value));
        break;
    case cvtDefence:
        playerStatus.setCurrentDefenceStatus(dsiDefence, static_cast<int16_t>(value));
        break;
    case cvtLight:
        playerStatus.setCurrentAttribute(abiLight, static_cast<int16_t>(value), false);
        break;
    case cvtDark:
        playerStatus.setCurrentAttribute(abiDark, static_cast<int16_t>(value), false);
        break;
    case cvtFire:
        playerStatus.setCurrentAttribute(abiFire, static_cast<int16_t>(value), false);
        break;
    case cvtIce:
        playerStatus.setCurrentAttribute(abiIce, static_cast<int16_t>(value), false);
        break;
    case cvtDarkResist:
        playerStatus.setCurrentAttribute(abiDark, static_cast<int16_t>(value), true);
        break;
    case cvtLightResist:
        playerStatus.setCurrentAttribute(abiLight, static_cast<int16_t>(value), true);
        break;
    case cvtFireResist:
        playerStatus.setCurrentAttribute(abiFire, static_cast<int16_t>(value), true);
        break;
    case cvtIceResist:
        playerStatus.setCurrentAttribute(abiIce, static_cast<int16_t>(value), true);
        break;
    default:
        return ecCheatInvalidParam;
    }

    return ecOk;
}


ErrorCode Player::setZeroAllCurrentStatus()
{
    static_cast<go::PlayerStatus&>(getCreatureStatus()).setZeroAllCurrentStatus();
    return ecOk;
}


ErrorCode Player::restoreCurrentStatus()
{
    static_cast<go::PlayerStatus&>(getCreatureStatus()).restoreCurrentStatus();
    return ecOk;
}



ErrorCode Player::addCheatItem(DataCode itemCode, uint8_t itemCount)
{	
    return inventory_->addItem(createAddItemInfoByItemCode(itemCode, itemCount));
}


ErrorCode Player::addCheatSkill(SkillCode skillCode)
{
    return addRewardSkill(skillCode);
}


ErrorCode Player::addCheatQuest(QuestCode questCode)
{
    if (! questRepository_.get()) {
        return ecServerInternalError;
    }
    const ErrorCode errorCode = questRepository_->cheatAcceptQuest(questCode);
    if (isSucceeded(errorCode)) {
        gc::QuestCallback* callback = getController().queryQuestCallback();
        if (callback) {
            callback->accepted(questCode);
        }
    }
    getPlayerQuestController().onAcceptQuest(errorCode, questCode);
    return errorCode;
}


ErrorCode Player::addCheatCharacterTitle(CharacterTitleCode titleCode)
{
    const ErrorCode errorCode = addCharacterTitle(titleCode);
    if (isSucceeded(errorCode)) {
        getPlayerCharacterTitleController().evCharacterTitleAdded(titleCode);
    }
    return ecOk;
}


ErrorCode Player::removeCheatQuest(QuestCode questCode)
{
    if (! questRepository_.get()) {
        return ecServerInternalError;
    }
    const ErrorCode errorCode = questRepository_->cheatRemoveCompleteQuest(questCode);
    getControllerAs<gc::PlayerController>().evCheatCompleteQuestRemoved(questCode);
    return errorCode;
}


ErrorCode Player::resetCheatCoolTime()
{
    resetCooldowns();
    return ecOk;
}


ErrorCode Player::toCheatPlayer(Entity& target)
{
    WorldPosition position = target.getWorldPosition();
    return TELEPORT_SERVICE->teleportPosition(position, *this);	
}


ErrorCode Player::toCheatMe(Entity& target)
{
    WorldPosition callerPosition = getWorldPosition();
    return TELEPORT_SERVICE->teleportPosition(callerPosition, target);
}


ErrorCode Player::toCheatPosition(const Position& target)
{
    return TELEPORT_SERVICE->teleportPosition(WorldPosition(ObjectPosition(target, minHeading),
        getWorldPosition().mapCode_), *this);
}


ErrorCode Player::whos()
{
    getControllerAs<gc::PlayerController>().evZoneInUsers(WORLD->getZoneUserInfos());
    return ecOk;
}


ErrorCode Player::refillCheatPoints()
{
    if (! getCreatureStatus().isDied()) {
        getCreatureStatus().restorePoints();
    }

    return ecOk;
}


ErrorCode Player::clearAcceptQuest(QuestCode questCode)
{
    const ErrorCode errorCode = questRepository_->cheatClearAcceptQuest(questCode);
    if (isSucceeded(errorCode)) {
        getPlayerQuestController().onCompleteQuest(errorCode, questCode, invalidDataCode);
    }
    return errorCode;
}


bool Player::isMeleeWeaponEquipped() const
{
    return getEquipInventory().isMeleeWeaponEquipped();
}


bool Player::isShieldEquipped() const
{
    return getEquipInventory().isShieldEquipped();
}

}}} // namespace gideon { namespace zoneserver { namespace go {
