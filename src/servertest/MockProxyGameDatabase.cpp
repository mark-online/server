#include "ServerTestPCH.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <gideon/servertest/datatable/MockWorldMapTable.h>
#include <gideon/servertest/datatable/MockSkillTable.h>
#include <gideon/servertest/datatable/MockEquipTable.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/server/config/ConfigReader.h>
#include <sne/base/utility/Assert.h>
#include <sne/core/utility/CppUtils.h>

using namespace gideon::serverbase;

namespace gideon { namespace servertest {

namespace
{

const ShardId defaultShardId = 1;
const ZoneId defaultZoneId = 1;
const ZoneId secondZoneId = 2;

inline std::wstring cat(const wchar_t* prefix, AccountId accountId)
{
    return sne::core::formatString(L"%s%u", prefix, accountId);
}


inline void fillSkills(FullCharacterInfo& characterInfo)
{
    characterInfo;
 //   characterInfo.skills_.insert(defaultMeleeAttackSkillCode);
 //   characterInfo.skills_.insert(defaultRangeAttackSkillCode);
 //   characterInfo.skills_.insert(defaultMagicAttackSkillCode);
 //   characterInfo.skills_.insert(defaultAreaAndCastee2SkillCode);
 //   characterInfo.skills_.insert(defaultSelfAreaSkillCode);
 //   characterInfo.skills_.insert(defaultTargetAreaSkillCode);
 //   characterInfo.skills_.insert(defaultLongGlobalCooldownTimeSkillCode);
 //   characterInfo.skills_.insert(defaultLongCastTimeSkillCode);
 //   characterInfo.skills_.insert(combatUseTypeSkillCode);
 //   characterInfo.skills_.insert(changePeaceSkillCode);
 //   characterInfo.skills_.insert(peaceUseTypeSkillCode);
 //   characterInfo.skills_.insert(hpRecoverySkillCode);
 //   characterInfo.skills_.insert(mpRecoverySkillCode);
 //   characterInfo.skills_.insert(graveStoneSkillCode);
 //   characterInfo.skills_.insert(absolutenessDefence);
 //   characterInfo.skills_.insert(stunSkillCode);
    //characterInfo.skills_.insert(testObserverSkillCode);
}


inline void fillCurrentPoints(FullCharacterInfo& characterInfo)
{
    characterInfo.currentPoints_.hp_ = hpDefault;
    characterInfo.currentPoints_.mp_ = mpDefault;
}


inline void fillInventory(FullCharacterInfo& characterInfo,
    const CreateCharacterEquipments& createCharacterEquipments)
{
    assert(createCharacterEquipments.isValid());

    static ObjectId equipId = invalidObjectId;
    characterInfo.inventory_.defaultInvenCount_ = defultInvenSlotId;
    for (int i = epFirst; i < epCount; ++i) {
        const EquipPart part = static_cast<EquipPart>(i);
        const EquipCode ec = createCharacterEquipments[i].equipCode_;
        if (! isValidDataCode(ec)) {
            continue;
        }

        const ItemInfo item(ec, 1, ++equipId, equippedSlotId);
        //characterInfo.inventory_.addEquipItem(item, EquipItemInfo(2, 45));
        characterInfo.equipments_[part] = equipId;
        characterInfo.inventory_.addItem(item);
    }

    //const ItemInfo otherHelmet(otherHelmetEquipCode, 1, ++equipId, firstSlotId);
    //characterInfo.inventory_.addEquipItem(otherHelmet, EquipItemInfo());

    //const ItemInfo spear(lanceEquipCode, 1, ++equipId, firstSlotId + 1);
    //characterInfo.inventory_.addEquipItem(spear, EquipItemInfo());

    //const ItemInfo shoes(shoesEquipCode, 1, ++equipId, firstSlotId + 2);
    //characterInfo.inventory_.addEquipItem(shoes, EquipItemInfo());
}

} // namespace

// = MockProxyGameDatabase

MockProxyGameDatabase::MockProxyGameDatabase() :
    verifiedAccountId_(invalidAccountId),
    lastCreatedAccountId_(invalidAccountId),
    lastCreatedCharacterId_(invalidObjectId)
{
}


bool MockProxyGameDatabase::open(sne::server::ConfigReader& configReader)
{
    configReader;
    return true;
}


void MockProxyGameDatabase::close()
{
}


void MockProxyGameDatabase::setValidAccount(AccountId accountId,
    const UserId& userId, const std::string& password)
{
    verifiedAccountId_ = accountId;
    verifiedUserId_ = userId;
    verifiedPassword_ = password;
}


const FullUserInfo MockProxyGameDatabase::getFullUserInfo(AccountId accountId) const
{
    FullUserInfo userInfo;
    userInfo.accountId_ = accountId;
    if (lastCreatedCharacterInfo_.isValid()) {
        userInfo.characters_.insert(
            FullCharacterInfos::value_type(
                lastCreatedCharacterInfo_.objectId_, lastCreatedCharacterInfo_));
        userInfo.lastAccessCharacterId_ = lastCreatedCharacterInfo_.objectId_;
    }
    return userInfo;
}


void MockProxyGameDatabase::registGameMailCallback(serverbase::GameMailCallback& gameMailCallback)
{
    gameMailCallback_ = &gameMailCallback;
}


void MockProxyGameDatabase::registDBQueryPlayerCallback(DBQueryPlayerCallback& playerCallback)
{
    playerCallback_ = &playerCallback;
}


void MockProxyGameDatabase::registBuddyCallback(CommunityBuddyCallback& buddyCallback)
{
    buddyCallback_ = &buddyCallback;
}


sne::base::Future::Ref MockProxyGameDatabase::getMaxInventoryId(ZoneId zoneId)
{
    addCallCount("getMaxInventoryId");
    zoneId;
    GetMaxItemInventoryIdRequestFuture* future = new GetMaxItemInventoryIdRequestFuture;
    future->maxInventoryId_ = 12345678;
    future->completed();
    return sne::base::Future::Ref(future);
}


UserId MockProxyGameDatabase::makeUserId(AccountId accountId)
{
    return cat(L"test", accountId);
}


Nickname MockProxyGameDatabase::makeNickname(AccountId accountId)
{
    return cat(L"test", accountId);
}


sne::base::Future::Ref MockProxyGameDatabase::asyncAuthenticate(
    const UserId& userId, const UserPassword& password)
{
    addCallCount("authenticate");

    AuthenticateRequestFuture* future = new AuthenticateRequestFuture;

    if ((userId == verifiedUserId_) && (password == verifiedPassword_)) {
        future->accountId_ = verifiedAccountId_;
        future->errorCode_ = ecOk;
    }
    else {
        future->accountId_ = invalidAccountId;
        future->errorCode_ = ecAuthenticateFailed;
    }

    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::asyncGetFullUserInfo(
    const AccountInfo& accountInfo)
{
    addCallCount("getFullUserInfo");

    GetFullUserInfoRequestFuture* future = new GetFullUserInfoRequestFuture(accountInfo);

    if( accountInfo.accountId_ != verifiedAccountId_) {
        future->errorCode_ = ecAccountNotFound;
    }
    else {
        future->userInfo_.accountId_ = accountInfo.accountId_;
        if (accountInfo.accountId_ == lastCreatedAccountId_) {
            future->userInfo_.characters_.insert(
                FullCharacterInfos::value_type(
                    lastCreatedCharacterInfo_.objectId_, lastCreatedCharacterInfo_));
        }
        future->errorCode_ = ecOk;
    }

    return sne::base::Future::Ref(future);
}


bool MockProxyGameDatabase::asyncUpdateLoginAt(AccountId accountId)
{
    addCallCount("updateLoginAt");

    accountId;
    return true;
}


sne::base::Future::Ref MockProxyGameDatabase::asyncCreateCharacter(const CreateCharacterInfo& createCharacterInfo,
    const CreateCharacterEquipments& createCharacterEquipments, ZoneId zoneId, const ObjectPosition& position)
{
    addCallCount("createCharacter");

    CreateCharacterRequestFuture* future = new CreateCharacterRequestFuture;

    if (createCharacterInfo.accountId_ != verifiedAccountId_) {
        future->errorCode_ = ecAccountNotFound;
    }
    else {
        lastCreatedAccountId_ = createCharacterInfo.accountId_;
        lastCreatedCharacterInfo_.reset();
        lastCreatedCharacterInfo_.appearance_ = createCharacterInfo.appearance_;
        lastCreatedCharacterInfo_.characterClass_ = ccAny;
        lastCreatedCharacterInfo_.objectType_ = otPc;
        lastCreatedCharacterInfo_.objectId_ = ++lastCreatedCharacterId_;
        lastCreatedCharacterInfo_.nickname_ = createCharacterInfo.nickname_;
        lastCreatedCharacterInfo_.currentLevel_.level_ = clMinLevel;
        lastCreatedCharacterInfo_.currentLevel_.levelStep_= lsMinLevel;
        lastCreatedCharacterInfo_.zoneId_ = zoneId;
        lastCreatedCharacterInfo_.gameMoney_ = 10000;
        lastCreatedCharacterInfo_.position_ = static_cast<const ObjectPosition&>(position);
        lastCreatedCharacterInfo_.sexType_ = createCharacterInfo.sexType_;
        lastCreatedCharacterInfo_.inventory_.defaultInvenCount_ = defultInvenSlotId;
        //lastCreatedCharacterInfo_.inventory_.getTotalSlotCount() = maxDefultInvenSlotId;
        //lastCreatedCharacterInfo_.bankAccountInfo_.getTotalSlotCount() = maxBankSlotId;
        nicknameMap_.emplace(createCharacterInfo.nickname_, lastCreatedCharacterInfo_.objectId_);
        lastCreatedCharacterInfo_.selectEquipType_ = EquipType::etOneHandSword;

        fillSkills(lastCreatedCharacterInfo_);
        fillInventory(lastCreatedCharacterInfo_, createCharacterEquipments);
        fillCurrentPoints(lastCreatedCharacterInfo_);
        
        VehicleInfo vehicle(1, vehicleCode1);
        lastCreatedCharacterInfo_.vehicleInventory_.add(vehicle);
        lastCreatedCharacterInfo_.vehicleInventory_.selectId_ = 1;
        GliderInfo glider(1, gliderCode1);
        lastCreatedCharacterInfo_.gliderInventory_.selectId_ = 1;
        lastCreatedCharacterInfo_.gliderInventory_.add(glider);


        CreatureStatusInfo gameStats;
        /*EQUIP_TABLE->applyCreatureStatusInfo(gameStats, lastCreatedCharacterInfo_.currentPoints_,
            lastCreatedCharacterInfo_.inventory_, lastCreatedCharacterInfo_.equipments_);*/
        lastCreatedCharacterInfo_.currentPoints_ = gameStats.points_;
        future->characterInfo_ = lastCreatedCharacterInfo_;
        future->errorCode_ = ecOk;
    }

    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::asyncDeleteCharacter(
    AccountId accountId, ObjectId characterId)
{
    accountId;

    addCallCount("deleteCharacter");

    DeleteCharacterRequestFuture* future = new DeleteCharacterRequestFuture;

    future->characterId_ = characterId;
    if (characterId == lastCreatedCharacterId_) {
        future->errorCode_ = ecOk;
    }
    else {
        future->errorCode_ = ecCharacterNotFound;
    }

    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::asyncCheckDuplicateNickname(
    AccountId accountId, const Nickname& nickname)
{
    accountId;

    addCallCount("checkDuplicateNickname");

    CheckDuplicateNicknameRequestFuture* future = new CheckDuplicateNicknameRequestFuture;

    future->accountId_ = accountId;
    future->nickname_ = nickname;
    
    if (nicknameMap_.find(nickname) != nicknameMap_.end()) {
        future->errorCode_ = ecOk;
    }
    else {
        future->errorCode_ = ecCharacterDuplicatedNickname;
    }

    future->completed();
    return sne::base::Future::Ref(future);
}


void MockProxyGameDatabase::asyncSaveCharacterStats(AccountId accountId, ObjectId characterId,
    const DBCharacterStats& saveInfo)
{
    addCallCount("saveCharacterStats");

    // TODO:
    accountId, characterId, saveInfo;
}


void MockProxyGameDatabase::asyncSaveSelectCharacterTitle(AccountId accountId, ObjectId characterId,
    CharacterTitleCode titleCode)
{
    addCallCount("saveSelectCharacterTitle");

    // TODO:
    accountId, characterId, titleCode;
}


sne::base::Future::Ref MockProxyGameDatabase::asyncLoadCharacterProperties(AccountId accountId,
    ObjectId characterId)
{
    // TODO:
    accountId, characterId;

    LoadCharacterPropertiesRequestFuture* future = new LoadCharacterPropertiesRequestFuture;
    future->errorCode_ = ecOk;
    future->completed();
    return sne::base::Future::Ref(future);
}


void MockProxyGameDatabase::asyncSaveCharacterProperties(AccountId accountId, ObjectId characterId,
    const std::string& config, const std::string& prefs)
{
    // TODO:
    accountId, characterId, config, prefs;
}


void MockProxyGameDatabase::asyncMoveInventoryItem(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId, SlotId slotId)
{
    addCallCount("moveInventoryItem");

    invenType, accountId, characterId, itemId, slotId;
}


void MockProxyGameDatabase::asyncSwitchInventoryItem(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId1, ObjectId itemId2)
{
    addCallCount("switchInventoryItem");

    invenType, accountId, characterId, itemId1, itemId2;
}


void MockProxyGameDatabase::asyncAddInventoryItem(AccountId accountId, ObjectId characterId,
    InvenType invenType, const ItemInfo& itemInfo)
{
    addCallCount("addInventoryItem");

    invenType, accountId, characterId, itemInfo;
}


void MockProxyGameDatabase::asyncChangeEquipItemInfo(AccountId accountId, ObjectId characterId, 
    ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount)
{
    addCallCount("changeEquipItemInfo");

    accountId, characterId, itemId, newEquipCode, socketCount;
}


void MockProxyGameDatabase::asyncEnchantEquipItem(AccountId accountId, ObjectId characterId, 
    ObjectId itemId, SocketSlotId id, EquipSocketInfo& socketInfo)
{
    addCallCount("enchantEquipItem");

    accountId, characterId, itemId, id, socketInfo;
}


void MockProxyGameDatabase::asyncUnenchantEquipItem(AccountId accountId, ObjectId characterId, 
    ObjectId itemId, SocketSlotId id)
{
    addCallCount("unenchantEquipItem");

    accountId, characterId, itemId, id;
}


void MockProxyGameDatabase::asyncRemoveInventoryItem(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId)
{
    addCallCount("removeInventoryItem");

    invenType, accountId, characterId, itemId;
}


void MockProxyGameDatabase::asyncUpdateInventoryItemCount(AccountId accountId,
    ObjectId characterId, InvenType invenType, ObjectId itemId, uint8_t itemCount)
{
    addCallCount("updateInventoryItemCount");

    invenType, accountId, characterId, itemId, itemCount;
}


void MockProxyGameDatabase::asyncAddQuestItem(AccountId accountId, ObjectId characterId,
    const QuestItemInfo& questItemInfo)
{
    addCallCount("addQuestItem");

    accountId, characterId, questItemInfo;
}


void MockProxyGameDatabase::asyncRemoveQuestItem(AccountId accountId, ObjectId characterId,
    ObjectId questItemId)
{
    addCallCount("removeQuestItem");

    accountId, characterId, questItemId;
}


void MockProxyGameDatabase::asyncUpdateQuestItemUsableCount(AccountId accountId, ObjectId characterId,
    ObjectId questItemId, uint8_t usableCount)
{
    addCallCount("updateQuestItemUsableCount");

    accountId, characterId, questItemId, usableCount;
}


void MockProxyGameDatabase::asyncUpdateQuestItemCount(AccountId accountId, ObjectId characterId,
    ObjectId questItemId, uint8_t stackCount)
{
    addCallCount("updateQuestItemCount");

    accountId, characterId, questItemId, stackCount;
}


void MockProxyGameDatabase::asyncEquipItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, EquipPart equipPart)
{
    addCallCount("equipItem");

    accountId, characterId, itemId, equipPart;
}


void MockProxyGameDatabase::asyncUnequipItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, SlotId slotId, EquipPart unequipPart)
{
    addCallCount("unequipItem");

    accountId, characterId, itemId, slotId, unequipPart;
}


void MockProxyGameDatabase::asyncReplaceInventoryWithEquipItem(AccountId accountId,
    ObjectId characterId, ObjectId unequipItemId, EquipPart unequipPart,
    ObjectId equipItemId, EquipPart equipPart)
{
    addCallCount("replaceInventoryWithEquipItem");

    accountId, characterId, unequipItemId, unequipPart, equipItemId, equipPart;
}


void MockProxyGameDatabase::asyncEquipAccessoryItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, AccessoryIndex equipPart)
{
    addCallCount("equipAccessoryItem");

    accountId, characterId, itemId, equipPart;
}


void MockProxyGameDatabase::asyncUnequipAccessoryItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart)
{
    addCallCount("unequipAccessoryItem");

    accountId, characterId, itemId, slotId, unequipPart;
}

void MockProxyGameDatabase::asyncReplaceInventoryWithAccessoryItem(AccountId accountId,
    ObjectId characterId, ObjectId unequipItemId, AccessoryIndex unequipPart,
    ObjectId equipItemId, AccessoryIndex equipPart)
{
    addCallCount("replaceInventoryWithAccessoryItem");

    accountId, characterId, unequipItemId, unequipPart, equipItemId, equipPart;
}


void MockProxyGameDatabase::asyncChangeCharacterState(AccountId accountId,
    ObjectId characterId, CreatureStateType state)
{
    addCallCount("changeCharacterState");

    accountId, characterId, state;
}


void MockProxyGameDatabase::asyncSaveActionBar(AccountId accountId, ObjectId characterId, 
    ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code)
{
    addCallCount("saveActionBar");

    accountId, characterId, abiIndex, abpIndex, code;   
}


void MockProxyGameDatabase::asyncLearnSkill(AccountId accountId,
    ObjectId characterId, SkillCode currentSkillCode, SkillCode learnSkillCode)
{
    addCallCount("learnSkill");

    accountId, characterId, currentSkillCode, learnSkillCode;   
}


void MockProxyGameDatabase::asyncRemoveSkill(AccountId accountId, ObjectId characterId, 
    SkillCode skillCode)
{
    addCallCount("removeSkill");
    accountId, characterId, skillCode;
}


void MockProxyGameDatabase::asyncRemoveAllSkill(AccountId accountId, ObjectId characterId)
{
    addCallCount("removeAllSkill");
    accountId, characterId;
}


sne::base::Future::Ref MockProxyGameDatabase::getProperties()
{
    addCallCount("getProperties");

    auto future = std::make_shared<GetPropertiesRequestFuture>();
    future->errorCode_ = ecOk;
    future->properties_.emplace("c2s.high_concurrent_session_count", "100");
    future->properties_.emplace("c2s.max_concurrent_session_count", "100");
    future->properties_.emplace("certificate.effective_period", "100");
    future->properties_.emplace("zone_server.dont_check_safe_region", "1");
    future->completed();
    return future;
}


sne::base::Future::Ref MockProxyGameDatabase::getServerSpec(
    const std::string& serverName, const std::string& suffix)
{
    addCallCount("getServerSpec");

    serverName, suffix;

    GetServerSpecRequestFuture* future = new GetServerSpecRequestFuture;
    future->errorCode_ = ecOk;
    //future->serverSpec_ = ;
    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::getShardInfo(ShardId shardId)
{
    shardId;

    addCallCount("getShardInfo");

    GetShardInfoRequestFuture* future = new GetShardInfoRequestFuture;
    future->errorCode_ = ecOk;
    future->shardInfo_.shardId_ = shardId;
    future->shardInfo_.zoneServerInfoMap_.insert(
        ZoneServerInfoMap::value_type(defaultZoneId,
            ZoneServerInfo(servertest::globalWorldMapCode, "ip", 1)));
    future->shardInfo_.zoneServerInfoMap_.insert(
        ZoneServerInfoMap::value_type(secondZoneId,
            ZoneServerInfo(servertest::otherWorldMapCode, "ip", 1)));
    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::getZoneInfo(const std::string& name)
{
    addCallCount("getZoneInfo");

    GetZoneInfoRequestFuture* future = new GetZoneInfoRequestFuture;
    future->errorCode_ = ecOk;
    future->zoneInfo_.zoneId_ = defaultZoneId;
    future->zoneInfo_.name_ = name;
    future->zoneInfo_.globalMapInfo_.mapCode_ = servertest::globalWorldMapCode;
    future->zoneInfo_.shardId_ = defaultShardId;
    future->zoneInfo_.isFirstZone_ = true;
    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::getZoneInfoList(ShardId shardId)
{
    addCallCount("getZoneInfoList");

    ZoneInfo zoneInfo;
    zoneInfo.zoneId_ = defaultZoneId;
    zoneInfo.name_ = "test_zone";
    zoneInfo.shardId_ = shardId;
    zoneInfo.isFirstZone_ = true;

    GetZoneInfoListRequestFuture* future = new GetZoneInfoListRequestFuture;
    future->errorCode_ = ecOk;
    future->zoneInfos_.push_back(zoneInfo);
    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::getWorldTime(ZoneId /*zoneId*/)
{
    addCallCount("getWorldTime");

    GetWorldTimeRequestFuture* future = new GetWorldTimeRequestFuture;
    future->errorCode_ = ecOk;
    future->worldTime_ = 0;
    future->completed();
    return sne::base::Future::Ref(future);
}


bool MockProxyGameDatabase::asyncUpdateWorldTime(ZoneId zoneId, WorldTime worldTime)
{
    addCallCount("updateWorldTime");

    zoneId, worldTime;
    return true;
}


void MockProxyGameDatabase::asyncAcceptedQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode)
{
    addCallCount("acceptQuest");

    accountId, characterId, questCode;
}

void MockProxyGameDatabase::asyncAcceptRepeatQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, sec_t repeatAcceptTime)
{
    addCallCount("acceptRepeatQuest");

    accountId, characterId, questCode, repeatAcceptTime;
}

void MockProxyGameDatabase::asyncCancelQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, bool isRepeatQuest)
{
    addCallCount("cancelQuest");

    accountId, characterId, questCode, isRepeatQuest;
}


void MockProxyGameDatabase::asyncCompleteQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, bool isRepeatQuest)
{
    addCallCount("completeQuest");

    accountId, characterId, questCode, isRepeatQuest;
}



void MockProxyGameDatabase::asyncUpdateQuestMission(AccountId accountId, ObjectId characterId, 
    QuestCode questCode, QuestMissionCode missionCode, const QuestGoalInfo& goalInfo, bool isRepeatQuest)
{
    addCallCount("updateQuestMission");

    accountId, characterId, questCode, missionCode, goalInfo, isRepeatQuest;
}

void MockProxyGameDatabase::asyncRemoveCompleteQuest(AccountId accountId, ObjectId characterId, 
    QuestCode questCode)
{
    addCallCount("removeCompleteQuest");

    accountId, characterId, questCode;
}


sne::base::Future::Ref MockProxyGameDatabase::getMaxGuildId()
{
    addCallCount("getMaxGuildId");

    GetMaxGuildIdRequestFuture* future = new GetMaxGuildIdRequestFuture;
    future->maxGuildId_ = 10;
    future->completed();
    return sne::base::Future::Ref(future);
}


sne::base::Future::Ref MockProxyGameDatabase::getGuildInfos()
{
    addCallCount("getGuildInfos");

    GetGuildInfosRequestFuture* future = new GetGuildInfosRequestFuture;
    GuildInfo info;
    info.guildId_ = 1;
    info.guildName_ = L"testguild";
    info.guildMarkCode_ = 1;
    info.guildMemberInfos_.push_back(GuildMemberInfo(1, L"test1", gmpLord));   
    future->completed();
    return sne::base::Future::Ref(future);
}

void MockProxyGameDatabase::asyncCreateGuild(const BaseGuildInfo& guildInfo)
{
    addCallCount("createGuild");

    guildInfo;
}


void MockProxyGameDatabase::asyncAddGuildMember(AccountId accountId, GuildId guildId, const GuildMemberInfo& memberInfo)
{
    addCallCount("addGuildMember");

    accountId, guildId, memberInfo;
}


void MockProxyGameDatabase::asyncRemoveGuildMember(ObjectId characterId)
{
    addCallCount("removeGuildMember");

    characterId;
}


void MockProxyGameDatabase::asyncRemoveGuild(GuildId guildId)
{
    addCallCount("removeGuild");

    guildId;
}


void MockProxyGameDatabase::asyncAddGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId, GuildRelatioshipType type)
{
    addCallCount("addGuildRelationship");

    ownerGuildId, targetGuildId, type;
}


void MockProxyGameDatabase::asyncRemoveGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId)
{
    addCallCount("removeGuildRelationship");

    ownerGuildId, targetGuildId;
}


void MockProxyGameDatabase::asyncChangeGuildMemberPosition(ObjectId characterId, GuildMemberPosition position)
{
    addCallCount("changeGuildMemberPosition");

    characterId, position;
}


void MockProxyGameDatabase::asyncAddGuildApplicant(ObjectId characterId, GuildId guildId)
{
    addCallCount("addGuildApplicant");

    characterId, guildId;
}


void MockProxyGameDatabase::asyncRemoveGuildApplicant(ObjectId characterId)
{
    addCallCount("removeGuildApplicant");

    characterId;
}


void MockProxyGameDatabase::asyncModifyGuildIntroduction(GuildId guildId, const GuildIntroduction& introduction)
{
    addCallCount("modifyGuildIntroduction");

    guildId, introduction;
}


void MockProxyGameDatabase::asyncModifyGuildNotice(GuildId guildId, const GuildNotice& notice)
{
    addCallCount("modifyGuildNotice");

    guildId, notice;
}


void MockProxyGameDatabase::asyncAddGuildRank(GuildId guildId, const GuildRankInfo& rankInfo, uint8_t vaultCount)
{
    guildId, rankInfo, vaultCount;
}


void MockProxyGameDatabase::asyncAddGuildBankRights(GuildId guildId, GuildRankId guildRankId, VaultId vaultId, const GuildBankVaultRightInfo& bankRankInfo)
{
    guildId, guildRankId, vaultId, bankRankInfo;
}


void MockProxyGameDatabase::asyncDeleteGuildRank(GuildId guildId, GuildRankId rankId)
{
    guildId, rankId;
}


void MockProxyGameDatabase::asyncSwapGuildRank(GuildId guildId, GuildRankId rankId1, GuildRankId rankId2)
{
    guildId, rankId1, rankId2;
}


void MockProxyGameDatabase::asyncUpdateGuildRankName(GuildId guildId, GuildRankId rankId, const GuildRankName& rankName)
{
    guildId, rankId, rankName;
}


void MockProxyGameDatabase::asyncUpdateGuildRankRights(GuildId guildId, GuildRankId rankId, uint32_t rights, uint32_t goldWithdrawalPerDay)
{
    guildId, rankId, rights, goldWithdrawalPerDay;
}


void MockProxyGameDatabase::asyncUpdateGuildBankRights(GuildId guildId, GuildRankId rankId,
    VaultId vaultId, uint32_t rights, uint32_t itemWithdrawalPerDay)
{
    guildId, rankId, vaultId, rights, itemWithdrawalPerDay;
}


void MockProxyGameDatabase::asyncUpdateGuildRank(GuildId guildId, ObjectId playerId, GuildRankId rankId)
{
    guildId, playerId, rankId;
}


void MockProxyGameDatabase::asyncRemoveAllGuildSkills(GuildId guildId)
{
    guildId;
}


void MockProxyGameDatabase::asyncRemoveGuildSkills(GuildId guildId, const SkillCodes& skillCodes)
{
    guildId, skillCodes;
}


void MockProxyGameDatabase::asyncAddGuildSkill(GuildId guildId, SkillCode skillCode)
{
    guildId, skillCode;
}


void MockProxyGameDatabase::asyncUpdateGuildExpInfo(GuildId guildId, const GuildLevelInfo& guildLevelInfo) 
{
    guildId, guildLevelInfo;
}


void MockProxyGameDatabase::asyncMoveGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId, SlotId slotId)
{
    addCallCount("moveGuildInventoryItem");

    guildId, itemId, slotId, vaultId;
}


void MockProxyGameDatabase::asyncSwitchGuildInventoryItem(GuildId guildId, VaultId vaultId, ObjectId itemId1,
    SlotId slotId1, ObjectId itemId2, SlotId slotId2)
{
    addCallCount("switchGuildInventoryItem");

    guildId, vaultId, itemId1, slotId1, itemId2, slotId2;
}


void MockProxyGameDatabase::asyncAddGuildInventoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo)
{
    addCallCount("addGuildInventoryItem");

    guildId, itemInfo, vaultId;
}


void MockProxyGameDatabase::asyncRemoveGuildInventoryItem(GuildId guildId, ObjectId itemId)
{
    addCallCount("removeGuildInventoryItem");

    guildId, itemId;
}


void MockProxyGameDatabase::asyncUpdateGuildInventoryItemCount(GuildId guildId, ObjectId itemId, uint8_t itemCount)
{
    addCallCount("updateGuildInventoryItemCount");

    guildId, itemId, itemCount;
}


void MockProxyGameDatabase::asyncUpdateGuildGameMoney(GuildId guildId, GameMoney gameMoney)
{
    addCallCount("updateGuildGameMoney");

    guildId, gameMoney;
}


void MockProxyGameDatabase::asyncUpdateGuildMemberWithdraw(ObjectId playerId, GameMoney dayWithdraw, sec_t resetTime)
{
    addCallCount("updateGuildMemberWithdraw");

    playerId, dayWithdraw, resetTime;
}


void MockProxyGameDatabase::asyncUpdateGuildVaultName(GuildId guildId, VaultId vaultId, const VaultName& name)
{
    addCallCount("updateGuildVaultName");

    guildId, vaultId, name;
}


void MockProxyGameDatabase::asyncAddGuildVault(GuildId guildId, const VaultInfo& vaultInfo)
{
    addCallCount("addGuildVault");

    guildId, vaultInfo;
}


void MockProxyGameDatabase::asyncSyncMail(ObjectId characterId)
{
    addCallCount("syncMail");
    gameMailCallback_->onSyncMail(characterId, MailInfos());
}


void MockProxyGameDatabase::asyncSendMail(const Nickname& reveiverNickname, const MailInfo& maillInfo)
{
    addCallCount("sendMail");
    static MailId mailId = 0;
    ++mailId;
    reveiverNickname, maillInfo;
    MailInfo newMail = maillInfo;
    ObjectId receiverId = invalidObjectId;
    if (! isValidDataCode(newMail.titleInfo_.dataCode_)) {
        NicknameMap::const_iterator pos = nicknameMap_.find(reveiverNickname);
        if (pos != nicknameMap_.end()) {
            newMail.titleInfo_.objectId_ = (*pos).second;
            receiverId = (*pos).second;
        }
        else {
            receiverId = newMail.titleInfo_.objectId_;
            newMail.titleInfo_.mailType_ = mtReturnMail;
        }
    }
    
    gameMailCallback_->onReceive(receiverId, newMail);
}


void MockProxyGameDatabase::asyncSendMail(ObjectId receiverId, const MailInfo& maillInfo)
{
    addCallCount("sendMail");
    gameMailCallback_->onReceive(receiverId, maillInfo);
}


void MockProxyGameDatabase::asyncAquireItemsInMail(MailId mailId)
{
    addCallCount("aquireItemsInMail");
    mailId;
}


void MockProxyGameDatabase::asyncDeleteMail(MailId mailId)
{
    addCallCount("deleteMail");
    mailId;
}


void MockProxyGameDatabase::asyncReadMail(MailId mailId)
{
    addCallCount("readMail");
    mailId;
}


void MockProxyGameDatabase::asyncQueryHasNotMail(ObjectId characterId)
{
    characterId;    
    //gameMailCallback_->onNotReadMail(characterId);
}


sne::base::Future::Ref MockProxyGameDatabase::getAuctionInfos(ZoneId zoneId)
{
    addCallCount("getAuctionInfos");
    zoneId;
    ServerAuctionInfoMap serverInfoMap;
    /*FullAuctionInfoMap fullInfoMap;
    FullAuctionInfo info1;
    info1.auctionId_ = 1;
    info1.currentBidMoney_ = 100;
    info1.buyoutMoney_ = 200;
    info1.sellerId_ = 1;
    info1.itemInfo_ = ItemInfo(defaultOneHandSwordEquipCode, 1, 456789, 1);
    info1.equipItemInfo_ = EquipItemInfo();
    info1.expireTime_ = time_t(0) + 1000;
    
    FullAuctionInfo info2;
    info2.auctionId_ = 2;
    info2.currentBidMoney_ = 200;
    info2.buyoutMoney_ = 300;
    info2.sellerId_ = 1;
    info2.itemInfo_ = ItemInfo(defaultShieldEquipCode, 1, 456790, 1);
    info2.equipItemInfo_ = EquipItemInfo();
    info2.expireTime_ = time_t(0) + 1000;

    FullAuctionInfo info3;
    info3.auctionId_ = 3;
    info3.currentBidMoney_ = 10;
    info3.buyoutMoney_ = 50;
    info3.sellerId_ = 1;
    info3.itemInfo_ = ItemInfo(oneHandSwordFragmentCode, 1, 456791, 1);
    info3.expireTime_ = time_t(0) + 1000;


    fullInfoMap.emplace(info1.auctionId_, info1);
    fullInfoMap.emplace(info2.auctionId_, info2);
    fullInfoMap.emplace(info3.auctionId_, info3);
    serverInfoMap.emplace(storeNpcCode, fullInfoMap);*/

    GetAuctionInfosRequestFuture* future = new GetAuctionInfosRequestFuture;
    future->errorCode_ = ecOk;
    future->auctionMap_ = serverInfoMap;
    future->completed();
    return sne::base::Future::Ref(future);
}


void MockProxyGameDatabase::asyncCreateAuction(ZoneId zoneId, NpcCode npcCode, const FullAuctionInfo& auctionInfo)
{
    addCallCount("createAuction");
    zoneId, npcCode, auctionInfo;
}


void MockProxyGameDatabase::asyncDeleteAuction(ZoneId zoneId, AuctionId auctionId)
{
    addCallCount("deleteAuction");
    zoneId, auctionId;
}


void MockProxyGameDatabase::asyncUpdateBid(ZoneId zoneId, AuctionId auctionId, ObjectId buyerId, GameMoney currentBidMoney)
{
    addCallCount("updateBid");
    zoneId, auctionId, buyerId, currentBidMoney;
}


void MockProxyGameDatabase::asyncUpdateArenaRecord(AccountId accountId, ObjectId characterId, ArenaModeType arenaMode,
    uint32_t resultScore, ArenaResultType resultType)
{
    addCallCount("updateArenaRecord");
    accountId, characterId, arenaMode, resultScore, resultType;
}


void MockProxyGameDatabase::asyncUpdateArenaPoint(AccountId accountId, ObjectId characterId, ArenaPoint arenaPoint)
{
    addCallCount("updateArenaPoint");
    accountId, characterId, arenaPoint;
}


void MockProxyGameDatabase::asyncUpdateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime)
{
    addCallCount("updateDeserterExpireTime");
    characterId, deserterExpireTime;
}


sne::base::Future::Ref MockProxyGameDatabase::loadBuildingInfos(ZoneId zoneId)
{
    addCallCount("loadBuildings");
    ObjectId buildingId = getMinZoneObjectId(zoneId);
    //BuildingInfo info;
    //info.objectType_ = otBuilding;
    //info.objectId_ = buildingId;
    //info.buildingCode_ = castleWall;
    //info.startBuildTime_ = getTime() - 1000;
    //info.state_ = bstBuilding;

    BuildingInfoMap buildingInfoMap;
//    buildingInfoMap.emplace(info.objectId_, info);

    LoadBuildingRequestFuture* future = new LoadBuildingRequestFuture;
    future->errorCode_ = ecOk;
    future->maxBuildingId_ = ++buildingId;
    future->buildingInfoMap_ = buildingInfoMap;
    future->completed();
    return sne::base::Future::Ref(future); 
}


void MockProxyGameDatabase::asyncMoveBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, ObjectId itemId, SlotId slotId)
{
    addCallCount("moveBuildingInventoryItem");
    buildingId, invenType, itemId, slotId;
}


void MockProxyGameDatabase::asyncSwitchBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, ObjectId itemId1, ObjectId itemId2,
    SlotId slotId1, SlotId slotId2)
{
    addCallCount("switchBuildingInventoryItem");
    buildingId, invenType, itemId1, itemId2, slotId1, slotId2;
}


void MockProxyGameDatabase::asyncAddBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, const ItemInfo& itemInfo)
{
    addCallCount("addBuildingInventoryItem");
    buildingId, invenType, itemInfo;
}


void MockProxyGameDatabase::asyncAddBuildingInventoryEquipItem(ObjectId buildingId,
    InvenType invenType, const ItemInfo& itemInfo, const EquipItemInfo& equipItemInfo)
{
    addCallCount("addBuildingInventoryEquipItem");
    buildingId, invenType, itemInfo, equipItemInfo;
}


void MockProxyGameDatabase::asyncRemoveBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType, ObjectId itemId)
{
    addCallCount("removeBuildingInventoryItem");
    buildingId, invenType, itemId;
}


void MockProxyGameDatabase::asyncUpdateBuildingInventoryItemCount(ObjectId buildingId,
    InvenType invenType, ObjectId itemId, uint8_t itemCount)
{
    addCallCount("updateBuildingInventoryItemCount");
    buildingId, invenType, itemId, itemCount;
}


void MockProxyGameDatabase::asyncRemoveAllBuildingInventoryItem(ObjectId buildingId,
    InvenType invenType)
{
    addCallCount("removeAllBuildingInventoryItem");
    buildingId, invenType;
}


void MockProxyGameDatabase::asyncAddGuildEventLog(GuildId guildId, const GuildEventLogInfo& guildEventLog)
{
    addCallCount("addGuildEventLog");
    guildId, guildEventLog;
}


void MockProxyGameDatabase::asyncAddGuildBankEventLog(GuildId guildId, const GuildBankEventLogInfo& guildBankEventLog)
{
    addCallCount("addGuildBankEventLog");
    guildId, guildBankEventLog;
}


void MockProxyGameDatabase::asyncAddGuildGameMoneyEventLog(GuildId guildId, const GuildGameMoneyEventLogInfo& guildGameMoneyEventLog)
{
    addCallCount("addGuildGameMoneyEventLog");
    guildId, guildGameMoneyEventLog;
}


void MockProxyGameDatabase::asyncAddBuildingGuard(ObjectId buildingId, const BuildingGuardInfo& guardInfo)
{
    addCallCount("addBuildingGuard");
    buildingId, guardInfo;
}


void MockProxyGameDatabase::asyncRemoveBuildingGuard(ObjectId guardId)
{
    addCallCount("removeBuildingGuard");
    guardId;
}


void MockProxyGameDatabase::asyncCreateBuilding(const BuildingInfo& buildingInfo)
{
    addCallCount("createBuilding");
    buildingInfo;
}


void MockProxyGameDatabase::asyncDeleteBuilding(ObjectId buildingId)
{
    addCallCount("deleteBuilding");
    buildingId;
}


void MockProxyGameDatabase::asyncUpdateBuildingState(ObjectId buildingId, BuildingStateType state,
    sec_t startBuildTime, sec_t expireTime, HitPoint currentHitPoint)
{
    addCallCount("updateBuildingState");
    buildingId, state, startBuildTime, expireTime, currentHitPoint;
}


void MockProxyGameDatabase::asyncUpdateBuildingOwner(ObjectId buildingId, 
    BuildingOwnerType ownerType, ObjectId characterId, GuildId guildId)
{
    addCallCount("updateBuildingOwner");
    buildingId, ownerType, characterId, guildId;
}


void MockProxyGameDatabase::asyncAddBindRecall(AccountId accountId, ObjectId characterId, 
    const BindRecallInfo& bindRecallInfo)
{
    addCallCount("addBindRecall");

    accountId, characterId, bindRecallInfo;
}


void MockProxyGameDatabase::asyncRemoveBindRecall(AccountId accountId, ObjectId characterId, 
    ObjectId linkId)
{
    addCallCount("removeBindRecall");

    accountId, characterId, linkId;
}


void MockProxyGameDatabase::asyncAddSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
    uint8_t count, sec_t completeTime)
{
    addCallCount("addSelectRecipeProduction");

    buildingId, recipeCode, count, completeTime;
}


void MockProxyGameDatabase::asyncRemoveSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode)
{
    addCallCount("removeSelectRecipeProduction");

    buildingId, recipeCode;
}


void MockProxyGameDatabase::asyncUpdateSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode,
    uint8_t count, sec_t completeTime)
{
    addCallCount("updateSelectRecipeProduction");

    buildingId, recipeCode, count, completeTime;
}


void MockProxyGameDatabase::asyncReleaseBeginnerProtection(AccountId accountId, ObjectId characterId)
{
    addCallCount("releaseBeginnerProtection");

    accountId, characterId;
}


void MockProxyGameDatabase::asyncAddCooldownInfos(AccountId accountId, ObjectId characterId, const CooltimeInfos& infos)
{
    addCallCount("addCooldownInfos");

    accountId, characterId, infos;
}


void MockProxyGameDatabase::asyncAddRemainEffects(AccountId accountId, ObjectId characterId, const RemainEffectInfos& infos)
{
    addCallCount("removeCooldownEffects");

    accountId, characterId, infos;
}


void MockProxyGameDatabase::asyncUpdateCharacterInventoryInfo(AccountId accountId, ObjectId characterId, InvenType invenType,
    bool isCashSlot, uint8_t count)
{
    addCallCount("updateCharacterInventoryInfo");

    accountId, characterId, invenType, isCashSlot, count;
}


void MockProxyGameDatabase::asyncCreateVehicle(AccountId accountId, ObjectId characterId, const VehicleInfo& info)
{
    addCallCount("createVehicle");

    accountId, characterId, info;
}


void MockProxyGameDatabase::asyncCreateGlider(AccountId accountId, ObjectId characterId, const GliderInfo& info)
{
    addCallCount("createGlider");

    accountId, characterId, info;
}


void MockProxyGameDatabase::asyncDeleteVehicle(AccountId accountId, ObjectId characterId, ObjectId id)
{
    addCallCount("deleteVehicle");

    accountId, characterId, id;
}


void MockProxyGameDatabase::asyncDeleteGlider(AccountId accountId, ObjectId characterId, ObjectId id)
{
    addCallCount("deleteGlider");

    accountId, characterId, id;
}


void MockProxyGameDatabase::asyncSelectVehicle(AccountId accountId, ObjectId characterId, ObjectId id)
{
    addCallCount("selectVehicle");

    accountId, characterId, id;
}


void MockProxyGameDatabase::asyncSelectGlider(AccountId accountId, ObjectId characterId, ObjectId id)
{
    addCallCount("selectGlider");

    accountId, characterId, id;
}


void MockProxyGameDatabase::asyncUpdateGliderDurability(AccountId accountId, ObjectId characterId, ObjectId id, uint32_t currentValue)
{
    addCallCount("updateGliderDurability");

    accountId, characterId, id, currentValue;
}


void MockProxyGameDatabase::asyncGetBuddies(AccountId accountId, ObjectId characterId)
{
    addCallCount("getBuddies");

    accountId, characterId;
}


void MockProxyGameDatabase::asyncAddBuddy(ObjectId characterId1, ObjectId characterId2)
{
    addCallCount("addBuddy");

    characterId1, characterId2;
}


void MockProxyGameDatabase::asyncRemoveBuddy(ObjectId characterId1, ObjectId characterId2)
{
    addCallCount("removeAddBuddy");

    characterId1, characterId2;
}


void MockProxyGameDatabase::asyncAddBlock(ObjectId ownerId, ObjectId characterId)
{
    addCallCount("addBlock");

    ownerId, characterId;
}


void MockProxyGameDatabase::asyncRemoveBlock(ObjectId ownerId, ObjectId characterId)
{
    addCallCount("removeAddBlock");

    ownerId, characterId;
}


void MockProxyGameDatabase::asyncQueryAchievements(AccountId accountId, ObjectId characterId)
{
    addCallCount("queryAchievements");

    accountId, characterId;
}


void MockProxyGameDatabase::asyncUpdateProcessAchievement(AccountId accountId, ObjectId characterId, 
    AchievementCode code, const ProcessAchievementInfo& missionInfo)
{
    addCallCount("updateProcessAchievement");

    accountId, characterId, code, missionInfo;
}


void MockProxyGameDatabase::asyncCompleteAchievement(AccountId accountId, ObjectId characterId, AchievementCode code, AchievementPoint point)
{
    addCallCount("completeAchievement");

    accountId, characterId, code, point;
}


void MockProxyGameDatabase::asyncQueryCharacterTitles(AccountId accountId, ObjectId characterId)
{
    addCallCount("queryCharacterTitles");

    accountId, characterId;
}

void MockProxyGameDatabase::asyncAddCharacterTitle(AccountId accountId, ObjectId characterId, CharacterTitleCode titleCode)
{
    addCallCount("addCharacterTitle");

    accountId, characterId, titleCode;
}


}} // namespace gideon { namespace servertest {
