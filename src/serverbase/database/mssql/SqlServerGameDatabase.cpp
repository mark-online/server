#include "ServerBasePCH.h"
#include "SqlServerGameDatabase.h"
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/HarnessTable.h>
#include <sne/database/ado/AdoDatabaseUtils.h>
#include <sne/server/config/ConfigReader.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Assert.h>

namespace gideon { namespace serverbase {

namespace
{

AcceptedQuestInfo* getAcceptedQuestInfo(AcceptedQuestInfos& infos, QuestCode questCode)
{
    for (AcceptedQuestInfo& info : infos) {
        if (info.questCode_ == questCode) {
            return &info;
        }
    }
    return nullptr;
}


AcceptedRepeatQuestInfo* getAcceptedRepeatQuestInfo(AcceptedRepeatQuestInfos& infos, QuestCode questCode)
{
    for (AcceptedRepeatQuestInfo& info : infos) {
        if (info.questCode_ == questCode) {
            return &info;
        }
    }
    return nullptr;
}

// SELECT * FROM sys.messages WHERE message_id = @ec
inline ErrorCode toErrorCodeFromCreateCharacterError(int ec)
{
    switch (ec) {
    case 0:
        return ecOk;
    case 547: // The %ls statement conflicted with the %ls constraint "%.*ls".
        return ecAccountNotFound;
    case 2601: // Cannot insert duplicate key row in object '%.*ls' with unique index '%.*ls'.
        return ecCharacterDuplicatedNickname;
    }
    return ecDatabaseInternalError;
}

template <class T>
void setOptionValue(T& commander, const ItemScriptInfos& options)
{
    commander.paramOptionScript1_.setValue(options.size() > 0 ? options[0].scriptType_ : 0);
    commander.paramOptionValue1_.setValue(options.size() > 0 ? options[0].value_ : 0);
    commander.paramOptionScript2_.setValue(options.size() > 1 ? options[1].scriptType_ : 0);
    commander.paramOptionValue2_.setValue(options.size() > 1 ? options[1].value_ : 0);
    commander.paramOptionScript3_.setValue(options.size() > 2 ? options[2].scriptType_ : 0);
    commander.paramOptionValue3_.setValue(options.size() > 2 ? options[2].value_ : 0);
}


void fetchCharacterInfo(FullCharacterInfo& characterInfo,
    sne::database::AdoRecordSet& rs, short i)
{
    rs.getFieldValue(i++, characterInfo.nickname_.ref());
    rs.getFieldEnumValue(i++, characterInfo.selectEquipType_);
    rs.getFieldValue(i++, characterInfo.selectTitleCode_);
    rs.getFieldValue(i++, characterInfo.characterClass_);
    rs.getFieldValue(i++, characterInfo.zoneId_);
    rs.getFieldValue(i++, characterInfo.position_.x_);
    rs.getFieldValue(i++, characterInfo.position_.y_);
    rs.getFieldValue(i++, characterInfo.position_.z_);
    rs.getFieldValue(i++, characterInfo.position_.heading_);
    rs.getFieldValue(i++, characterInfo.sexType_);
    rs.getFieldValue(i++, characterInfo.currentLevel_.level_);
    rs.getFieldValue(i++, characterInfo.currentLevel_.levelStep_);
    rs.getFieldValue(i++, characterInfo.exp_);
    rs.getFieldValue(i++, characterInfo.sp_);
    rs.getFieldValue(i++, characterInfo.currentPoints_.hp_);
    rs.getFieldValue(i++, characterInfo.currentPoints_.mp_);
    rs.getFieldValue(i++, characterInfo.stateInfo_.states_);
    rs.getFieldValue(i++, characterInfo.chaotic_);
    rs.getFieldValue(i++, characterInfo.achievementPoint_);
    rs.getFieldValue(i++, characterInfo.actionBars_.isActionBarLock_);
    rs.getFieldValue(i++, characterInfo.guildId_);
    rs.getFieldValue(i++, characterInfo.arenaPoint_);
    rs.getFieldValue(i++, characterInfo.deserterExpireTime_);
    rs.getFieldValue(i++, characterInfo.isBeginnerProtection_);
    rs.getFieldValue(i++, characterInfo.maxBindRecallCont_);
    rs.getFieldValue(i++, characterInfo.vehicleInventory_.selectId_);
    rs.getFieldValue(i++, characterInfo.gliderInventory_.selectId_);
    rs.getFieldValue(i++, characterInfo.vehicleInventory_.commonHarnessInfo_.harnessCode_);
    rs.getFieldValue(i++, characterInfo.vehicleInventory_.commonHarnessInfo_.expireTime_);

    if (HARNESS_TABLE) {
        const gdt::harness_t* harness =
            HARNESS_TABLE->getHarness(characterInfo.vehicleInventory_.commonHarnessInfo_.harnessCode_);
        if (harness) {
            characterInfo.vehicleInventory_.inventory_.defaultInvenCount_ = harness->inven_count();
        }
    }
    

    if (characterInfo.deserterExpireTime_ < getTime()) {
        characterInfo.deserterExpireTime_ = 0;
    }
}

void fetchActionBar(ActionBar& actionBar,
    sne::database::AdoRecordSet& rs, short i)
{
    rs.getFieldValue(i++, actionBar[0]);
    rs.getFieldValue(i++, actionBar[1]);
    rs.getFieldValue(i++, actionBar[2]);
    rs.getFieldValue(i++, actionBar[3]);
    rs.getFieldValue(i++, actionBar[4]);
    rs.getFieldValue(i++, actionBar[5]);
    rs.getFieldValue(i++, actionBar[6]);
    rs.getFieldValue(i++, actionBar[7]);
    rs.getFieldValue(i++, actionBar[8]);
    rs.getFieldValue(i++, actionBar[9]);
    rs.getFieldValue(i++, actionBar[10]);
    rs.getFieldValue(i++, actionBar[11]);
}


void fetchBindRecall(BindRecallInfo& bindRecallInfo, sne::database::AdoRecordSet& rs, short i)
{
    rs.getFieldValue(i++, bindRecallInfo.linkId_);
    rs.getFieldValue(i++, bindRecallInfo.dataCode_);
    rs.getFieldValue(i++, bindRecallInfo.wordlPosition_.mapCode_);
    rs.getFieldValue(i++, bindRecallInfo.wordlPosition_.x_);
    rs.getFieldValue(i++, bindRecallInfo.wordlPosition_.y_);
    rs.getFieldValue(i++, bindRecallInfo.wordlPosition_.z_);
}


void fetchItemScriptInfo(ItemScriptInfos& options,
    sne::database::AdoRecordSet& rs, short i)
{
    ItemScriptInfo info;
    rs.getFieldValue(i++, info.scriptType_);
    rs.getFieldValue(i++, info.value_);
    if (info.isValid()) {
        options.push_back(info);
    }
    
    rs.getFieldValue(i++, info.scriptType_);
    rs.getFieldValue(i++, info.value_);
    if (info.isValid()) {
        options.push_back(info);
    }
    rs.getFieldValue(i++, info.scriptType_);
    rs.getFieldValue(i++, info.value_);
    if (info.isValid()) {
        options.push_back(info);
    }
}


void fetchSocketInfo(EquipSocketInfo& socketInfo, sne::database::AdoRecordSet& rs, short i)
{
    rs.getFieldValue(i++, socketInfo.gemCode_);
    fetchItemScriptInfo(socketInfo.scriptInfos_, rs, i);
}


void fetchApplyEffectInfo(RemainEffectInfo& effectInfo,
    sne::database::AdoRecordSet& rs, short i)
{
    rs.getFieldValue(i++, effectInfo.dataCode_);
    rs.getFieldValue(i++, effectInfo.isCaster_);
    rs.getFieldValue(i++, effectInfo.useTargetStatusType_);
    rs.getFieldValue(i++, effectInfo.effects_[0].effectValue_);
    rs.getFieldValue(i++, effectInfo.effects_[0].expireActivateTime_);
    rs.getFieldValue(i++, effectInfo.effects_[1].effectValue_);
    rs.getFieldValue(i++, effectInfo.effects_[1].expireActivateTime_);
}


void fetchVehicleInventory(VehicleInventory& inventory,
    sne::database::AdoRecordSet& rs, short i)
{
    VehicleInfo info;
    rs.getFieldValue(i++, info.objectId_);
    rs.getFieldValue(i++, info.vehicleCode_);
    rs.getFieldValue(i++, info.birthDay_);
    rs.getFieldValue(i++, info.peakAge_);
    rs.getFieldValue(i++, info.harnessInfo_.harnessCode_);
    rs.getFieldValue(i++, info.harnessInfo_.expireTime_);
    if (inventory.selectId_ == info.objectId_) {
        const gdt::harness_t* harness = HARNESS_TABLE->getHarness(info.harnessInfo_.harnessCode_);
        if (harness) {
            int8_t invenCount = static_cast<int8_t>(harness->inven_count());
            if (invenCount != 0) {
                if (invenCount < 0 && inventory.inventory_.defaultInvenCount_ < invenCount) {
                    inventory.inventory_.defaultInvenCount_ = 0;
                }
                else {
                    inventory.inventory_.defaultInvenCount_ += invenCount;
                }
            }
        }
    }
    inventory.add(info);
}


} // namespace

// = SqlServerGameDatabase

SqlServerGameDatabase::SqlServerGameDatabase() :
    getPropertiesCommand_(getAdoDatabase()),
    getServerSpecCommand_(getAdoDatabase()),
    getShardInfoCommand_(getAdoDatabase()),
    getZoneInfoCommand_(getAdoDatabase()),
    getZoneListCommand_(getAdoDatabase()),
    getWorldTimeCommand_(getAdoDatabase()),
    updateWorldTimeCommand_(getAdoDatabase()),
    getMaxInventoryIdCommand_(getAdoDatabase()),
    getRecentLoginUserListCommand_(getAdoDatabase()),
    getFullUserInfoCommand_(getAdoDatabase()),
    checkDuplicateNicknameCommand_(getAdoDatabase()),
    createCharacterCommand_(getAdoDatabase()),
    deleteCharacterCommand_(getAdoDatabase()),
    saveCharacterStatsCommand_(getAdoDatabase()),
    loadCharacterPropertiesCommand_(getAdoDatabase()),
    saveCharacterPropertiesCommand_(getAdoDatabase()),
    getCharacterInfoCommand_(getAdoDatabase()),
    updateCharacterPointsCommand_(getAdoDatabase()),
    saveSelectCharacterTitleCommand_(getAdoDatabase()),
    moveItemCommand_(getAdoDatabase()),
    switchItemCommand_(getAdoDatabase()),
    addItemCommand_(getAdoDatabase()),
    addEquipItemCommand_(getAdoDatabase()),
    addEquipSocketOptionCommand_(getAdoDatabase()),
    removeEquipSocketOptionCommand_(getAdoDatabase()),
    changeEquipItemInfoCommand_(getAdoDatabase()),
    removeItemCommand_(getAdoDatabase()),
    updateItemCountCommand_(getAdoDatabase()),
    addQuestItemCommand_(getAdoDatabase()),
    removeQuestItemCommand_(getAdoDatabase()),
    updateQuestItemUsableCountCommand_(getAdoDatabase()),
    updateQuestItemCountCommand_(getAdoDatabase()),
    equipItemCommand_(getAdoDatabase()),
    unequipItemCommand_(getAdoDatabase()),
    replaceInvenWithEquipItemCommand_(getAdoDatabase()),
    equipAccessoryItemCommand_(getAdoDatabase()),
    unequipAccessoryItemCommand_(getAdoDatabase()),
    replaceInventoryWithAccessoryItemCommand_(getAdoDatabase()),
    changeCharacterStateCommand_(getAdoDatabase()),
    saveActionBarCommand_(getAdoDatabase()),
    learnSkillCommand_(getAdoDatabase()),
    removeSkillCommand_(getAdoDatabase()),
    removeAllSkillCommand_(getAdoDatabase()),
    getQuestsCommand_(getAdoDatabase()),
    getRepeatQuestsCommand_(getAdoDatabase()),
    acceptQuestCommand_(getAdoDatabase()),
    acceptRepeatQuestCommand_(getAdoDatabase()),
    cancelQuestCommand_(getAdoDatabase()),
    cancelRepeatQuestCommand_(getAdoDatabase()),
    completeQuestCommand_(getAdoDatabase()),
    completeRepeatQuestCommand_(getAdoDatabase()),
    updateCompleteRepeatQuestCommand_(getAdoDatabase()),
    insertQuestMissionCommand_(getAdoDatabase()),
    insertRepeatQuestMissionCommand_(getAdoDatabase()),
    updateQuestMissionCommand_(getAdoDatabase()),
    updateRepeatQuestMissionCommand_(getAdoDatabase()),
    removeCompleteRepeatQuest_(getAdoDatabase()),
    removeCompleteQuestCommand_(getAdoDatabase()),
    createGuildCommand_(getAdoDatabase()),
    addGuildMemberCommand_(getAdoDatabase()),
    getGuildInfosCommand_(getAdoDatabase()),
    getGuildBankRightsCommand_(getAdoDatabase()),
    getGuildRanksCommand_(getAdoDatabase()),
    getGuildSkillsCommand_(getAdoDatabase()),
    getGuildMemberInfosCommand_(getAdoDatabase()),
    getGuildRelationshipsCommand_(getAdoDatabase()),
    guildApplicantsCommand_(getAdoDatabase()),
    getMaxGuildIdCommand_(getAdoDatabase()),
    getGuildVaultsCommand_(getAdoDatabase()),
    getGuildInventory_(getAdoDatabase()),
    getGuildEventLogCommand_(getAdoDatabase()),
    getGuildBankEventLogCommand_(getAdoDatabase()),
    getGuildGameMoneyEventLogCommand_(getAdoDatabase()),
    addGuildRankCommand_(getAdoDatabase()),
    addGuildBankRightsCommand_(getAdoDatabase()),
    deleteGuildRankCommand_(getAdoDatabase()),
    swapGuildRankCommand_(getAdoDatabase()),
    updateGuildRankNameCommand_(getAdoDatabase()),
    updateGuildRankRightsCommand_(getAdoDatabase()),
    updateGuildBankRightsCommand_(getAdoDatabase()),
    updateGuildRankCommand_(getAdoDatabase()),
    updateGuildExpCommand_(getAdoDatabase()),
    addGuildSkillCommand_(getAdoDatabase()),
    removeAllGuildSkillsCommand_(getAdoDatabase()),
    removeGuildSkillCommand_(getAdoDatabase()),
    removeGuildMemberCommand_(getAdoDatabase()),
    removeGuildCommand_(getAdoDatabase()),
    addGuildRelationshipCommand_(getAdoDatabase()),
    removeGuildRelationshipCommand_(getAdoDatabase()),
    changeGuildMemberPositionCommand_(getAdoDatabase()),
    addGuildApplicantCommand_(getAdoDatabase()),
    removeGuildApplicantCommand_(getAdoDatabase()),
    modifyGuildIntroductionCommand_(getAdoDatabase()),
    modifyGuildNoticeCommand_(getAdoDatabase()),
    updateGuildGameMoney_(getAdoDatabase()),
    updateGuildMemberWithdrawCommand_(getAdoDatabase()),
    updateGuildVaultNameCommand_(getAdoDatabase()),
    addGuildVaultCommand_(getAdoDatabase()),
    addGuildItemCommand_(getAdoDatabase()),
    addGuildEquipItemCommand_(getAdoDatabase()),
    addGuildEquipSocketOptionCommand_(getAdoDatabase()),
    removeGuildItemCommand_(getAdoDatabase()),
    updateGuildItemCountCommand_(getAdoDatabase()),
    moveGuildItemCommand_(getAdoDatabase()),
    switchGuildItemCommand_(getAdoDatabase()),
    getPlayerIdCommand_(getAdoDatabase()),
    getMailsCommand_(getAdoDatabase()),
    insertMailCommand_(getAdoDatabase()),
    addMailItemCommand_(getAdoDatabase()),
    addMailEquipItemCommand_(getAdoDatabase()),
    addMailEquipSocketOptionCommand_(getAdoDatabase()),
    deleteMailItemCommand_(getAdoDatabase()),
    deleteMailCommand_(getAdoDatabase()),
    readMailCommand_(getAdoDatabase()),
    hasNotReadMailCommand_(getAdoDatabase()),
    getAuctionsCommand_(getAdoDatabase()),
    insertAuctionCommand_(getAdoDatabase()),
    addAuctionEquipInfoCommand_(getAdoDatabase()),
    addAuctionEquipSocketOptionCommand_(getAdoDatabase()),
    updateBidCommand_(getAdoDatabase()),
    deleteAuctionCommand_(getAdoDatabase()),
    getPlayerArenaRecordsCommand_(getAdoDatabase()),
    updateArenaRecordCommand_(getAdoDatabase()),
    updateArenaPointCommand_(getAdoDatabase()),
    updateDeserterExpireTimeCommand_(getAdoDatabase()),
    createBuildingCommand_(getAdoDatabase()),
    getBuildingsCommand_(getAdoDatabase()),
    removeBuildingCommand_(getAdoDatabase()),
    updateBuildingStateCommand_(getAdoDatabase()),
    updateBuildingOwnerCommand_(getAdoDatabase()),
    moveBuildingItemCommand_(getAdoDatabase()),
    switchBuildingItemCommand_(getAdoDatabase()),
    addBuildingItemCommand_(getAdoDatabase()),
    addBuildingEquipItemCommand_(getAdoDatabase()),
    removeBuildingItemCommand_(getAdoDatabase()),
    addBuildingGuardCommand_(getAdoDatabase()),
    removeBuildingGuardCommand_(getAdoDatabase()),
    addSelectRecipeProductionCommand_(getAdoDatabase()),
    removeSelectRecipeProductionCommand_(getAdoDatabase()),
    updateSelectRecipeProductionCommand_(getAdoDatabase()),
    updateBuildingItemCountCommand_(getAdoDatabase()),
    removeAllBuildingItemCommand_(getAdoDatabase()),
    addBindRecallInfoCommand_(getAdoDatabase()),
    removeBindRecallCommand_(getAdoDatabase()),
    releaseBeginnerProtectionCommand_(getAdoDatabase()),
    addCooldownCommand_(getAdoDatabase()),
    addRemainEffectCommand_(getAdoDatabase()),
    removeAllCooldownCommand_(getAdoDatabase()),
    removeAllRemainEffectCommand_(getAdoDatabase()),
    updateCharacterInventoryInfoCommand_(getAdoDatabase()),
    createVehicleCommand_(getAdoDatabase()),
    createGliderCommand_(getAdoDatabase()),
    deleteVehicleCommand_(getAdoDatabase()),
    deleteGliderCommand_(getAdoDatabase()),
    selectVehicleCommand_(getAdoDatabase()),
    selectGliderCommand_(getAdoDatabase()),
    updateGliderDurabilityCommand_(getAdoDatabase()),
    getBuddiesCommand_(getAdoDatabase()),
    addBuddyCommand_(getAdoDatabase()),
    removeBuddyCommand_(getAdoDatabase()),
    addBlockCommand_(getAdoDatabase()),
    removeBlockCommand_(getAdoDatabase()),
    addAccessoryItemCommand_(getAdoDatabase()),
    addGuildAccessoryItemCommand_(getAdoDatabase()),
    addMailAccessoryItemCommand_(getAdoDatabase()),
    addAuctionAccessoryInfoCommand_(getAdoDatabase()),
    addBuildingEquipSocketOptionCommand_(getAdoDatabase()),
    addBuildingAccessoryItemCommand_(getAdoDatabase()),
    getCharacterTitlesCommand_(getAdoDatabase()),
    getCompleteAchievementsCommand_(getAdoDatabase()),
    getProcessAchievementsCommand_(getAdoDatabase()),
    addCharacterTitleCommand_(getAdoDatabase()),
    addProcessAchievementCommand_(getAdoDatabase()),
    updateProcessAchievementCommand_(getAdoDatabase()),
    completeAchievementCommand_(getAdoDatabase())
{
}


bool SqlServerGameDatabase::open(sne::server::ConfigReader& configReader)
{
    const std::string connectionString =
        configReader.getString("database", "connection-string");
    const uint8_t connectionTimeout = static_cast<uint8_t>(
        configReader.getNumeric<size_t>("database", "connection-timeout"));
    if (! connection_.open(connectionString, connectionTimeout)) {
        SNE_LOG_ERROR(__FUNCTION__ " Failed(%s, %d)",
            connectionString.c_str(), connectionTimeout);
        return false;
    }
    return true;
}


void SqlServerGameDatabase::close()
{
    connection_.close();
}



void SqlServerGameDatabase::beginTransaction()
{
    try {
        getAdoDatabase().execute("BEGIN TRANSACTION");
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
    }
}


void SqlServerGameDatabase::commitTransaction()
{
    try {
        getAdoDatabase().execute("COMMIT");
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
    }
}


void SqlServerGameDatabase::rollbackTransaction()
{
    try {
        getAdoDatabase().execute("ROLLBACK");
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
    }
}


bool SqlServerGameDatabase::getProperties(sne::server::Properties& properties)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getPropertiesCommand_.execute();

        std::string key;
        std::string value;
        sne::database::AdoRecordSet& rs = getPropertiesCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, key);
            rs.getFieldValue(1, value);

            properties.emplace(key, value);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getServerSpec(sne::server::ServerSpec& spec,
    const std::string& serverName, const std::string& suffix)
{
    sne::server::Profiler profiler(__FUNCTION__);

    std::string name = serverName;
    if (! suffix.empty()) {
        name += "." + suffix;
    }

    try {
        getServerSpecCommand_.paramName_.setValue(name);

        getServerSpecCommand_.execute();

        std::string address;
        getServerSpecCommand_.paramListeningAddress_.getValue(address);
        spec.listeningAddresses_.push_back(address);
        getServerSpecCommand_.paramListeningPort_.getValue(
            spec.listeningPort_);
        getServerSpecCommand_.paramMonitorPort_.getValue(
            spec.monitorPort_);
        getServerSpecCommand_.paramWorkerThreadCount_.getValue(
            spec.workerThreadCount_);
        getServerSpecCommand_.paramSessionPoolSize_.getValue(
            spec.sessionPoolSize_);
        getServerSpecCommand_.paramMaxUserCount_.getValue(
            spec.maxUserCount_);
        getServerSpecCommand_.paramPacketCipher_.getValue(
            spec.packetCipher_);
        getServerSpecCommand_.paramCipherKeyTimeLimit_.getValue(
            spec.cipherKeyTimeLimit_);
        getServerSpecCommand_.paramAuthenticationTimeout_.getValue(
            spec.authenticationTimeout_);
        getServerSpecCommand_.paramHeartbeatTimeout_.getValue(
            spec.heartbeatTimeout_);
        getServerSpecCommand_.paramMaxBytesPerSecond_.getValue(
            spec.sessionCapacity_.maxBytesPerSecond_);
        getServerSpecCommand_.paramSecondsForThrottling_.getValue(
            spec.sessionCapacity_.secondsForThrottling_);
        getServerSpecCommand_.paramMaxPendablePacketCount_.getValue(
            spec.sessionCapacity_.maxPendablePacketCount_);
        getServerSpecCommand_.paramCertificate_.getValue(
            spec.certificate_);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    spec.name_ = name;
    return true;
}


bool SqlServerGameDatabase::getShardInfo(FullShardInfo& shardInfo, ShardId shardId)
{
    shardInfo.shardId_ = shardId;
    try {
        getShardInfoCommand_.paramShardId_.setValue(shardId);

        getShardInfoCommand_.execute();

        sne::database::AdoRecordSet& rs = getShardInfoCommand_.adoRecordSet_;

        ZoneServerInfo zoneInfo;

        for (; ! rs.isEof(); rs.moveNext()) {
            ZoneId zoneId = invalidZoneId;
            rs.getFieldValue(0, zoneId);
            rs.getFieldValue(1, zoneInfo.mapCode_);
            rs.getFieldValue(2, zoneInfo.isFirstZone_);
            rs.getFieldValue(3, zoneInfo.isEnabled_);
            rs.getFieldValue(4, zoneInfo.zoneServerIp_);
            rs.getFieldValue(5, zoneInfo.zoneServerPort_);
            shardInfo.zoneServerInfoMap_.emplace(zoneId, zoneInfo);
        }

    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getZoneInfo(ZoneInfo& zoneInfo, const std::string& name)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getZoneInfoCommand_.paramName_.setValue(name);

        getZoneInfoCommand_.execute();

        getZoneInfoCommand_.paramId_.getValue(zoneInfo.zoneId_);
        getZoneInfoCommand_.paramGlobalWorldMapCode_.getValue(
            zoneInfo.globalMapInfo_.mapCode_);
        getZoneInfoCommand_.paramShardId_.getValue(zoneInfo.shardId_);
        getZoneInfoCommand_.paramIsFirstZone_.getValue(zoneInfo.isFirstZone_);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    zoneInfo.name_ = name;
    return true;
}


bool SqlServerGameDatabase::getZoneInfoList(ZoneInfos& zoneInfos, ShardId shardId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getZoneListCommand_.paramShardId_.setValue(shardId);

        getZoneListCommand_.execute();

        ZoneInfo zoneInfo;
        sne::database::AdoRecordSet& rs = getZoneListCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, zoneInfo.zoneId_);
            rs.getFieldValue(1, zoneInfo.name_);
            rs.getFieldValue(2, zoneInfo.globalMapInfo_.mapCode_);
            rs.getFieldValue(3, zoneInfo.isFirstZone_);
            zoneInfo.shardId_ = shardId;

            zoneInfos.push_back(zoneInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getWorldTime(WorldTime& worldTime, ZoneId zoneId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    worldTime = 0;
    try {
        getWorldTimeCommand_.paramZoneId_.setValue(zoneId);

        getWorldTimeCommand_.execute();

        getWorldTimeCommand_.paramWorldTime_.getValue(worldTime);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getMaxInventoryId(ObjectId& itemId, ZoneId zoneId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    itemId = invalidObjectId;
    const ObjectId minId = getMinZoneObjectId(zoneId);
    const ObjectId maxId = getMaxZoneObjectId(zoneId);
    
    try {
        getMaxInventoryIdCommand_.paramMinId_.setValue(minId);
        getMaxInventoryIdCommand_.paramMaxId_.setValue(maxId);
        getMaxInventoryIdCommand_.execute();

        getMaxInventoryIdCommand_.paramItemId_.getValue(itemId);
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getRecentLoginUserList(AccountIds& userList, uint16_t maxCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getRecentLoginUserListCommand_.paramMaxCount_.setValue(maxCount);
        getRecentLoginUserListCommand_.execute();

        userList.reserve(maxCount);
        sne::database::AdoRecordSet& rs =
            getRecentLoginUserListCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            AccountId accountId = invalidAccountId;
            rs.getFieldValue(0, accountId);

            userList.push_back(accountId);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}



bool SqlServerGameDatabase::getFullUserInfo(FullUserInfo& userInfo,
    AccountId accountId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    userInfo.accountId_ = accountId;
    if (! isValidAccountId(accountId)) {
        return false;
    }

    try {
        getFullUserInfoCommand_.paramAccountId_.setValue(accountId);

        getFullUserInfoCommand_.execute();

        sne::database::AdoRecordSet& rs = getFullUserInfoCommand_.adoRecordSet_;

        FullCharacterInfo characterInfo;				
        for (; ! rs.isEof(); rs.moveNext()) {
            // TODO: inven
            // characterInfo.inventory_.getTotalSlotCount() = maxDefultInvenSlotId;
            characterInfo.bankAccountInfo_.defaultInvenCount_ = maxBankSlotId;

            rs.getFieldValue(0, characterInfo.objectId_);
            fetchCharacterInfo(characterInfo, rs, 1);            
            userInfo.characters_.emplace(characterInfo.objectId_, characterInfo);
        }


        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, userInfo.lastAccessCharacterId_);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId = invalidObjectId;
            GliderInfo info;
            rs.getFieldValue(0, characterId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }
            rs.getFieldValue(1, character->appearance_.hair_);
            rs.getFieldValue(2, character->appearance_.face_);
            rs.getFieldValue(3, character->appearance_.eyes_);			
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId = invalidObjectId;
            VehicleInfo info;
            short i = 0;
            rs.getFieldValue(i++, characterId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }
            fetchVehicleInventory(character->vehicleInventory_, rs, i);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId = invalidObjectId;
            GliderInfo info;
            rs.getFieldValue(0, characterId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }
            rs.getFieldValue(1, info.objectId_);
            rs.getFieldValue(2, info.gliderCode_);
            rs.getFieldValue(3, info.durability_);
            character->gliderInventory_.add(info);
        }

        rs.nextRecordset();
        
        characterInfo.bankAccountInfo_.defaultInvenCount_ = maxBankSlotId;

        for (; ! rs.isEof(); rs.moveNext()) {
            // 추후에 다른 인벤도 추가할수도 있음
            ObjectId characterId;
            InvenType invenType;
            rs.getFieldValue(0, characterId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }
            rs.getFieldValue(1, invenType);
            assert(invenType == itPc);
            rs.getFieldValue(2, character->inventory_.defaultInvenCount_);
            rs.getFieldValue(3, character->inventory_.freeSlotCount_);
            rs.getFieldValue(4, character->inventory_.cashSlotCount_);
        }

        rs.nextRecordset();

        QuestItemInfo questItemInfo;
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, questItemInfo.itemId_);
            rs.getFieldValue(1, characterId);
            rs.getFieldValue(2, questItemInfo.questItemCode_);
            rs.getFieldValue(3, questItemInfo.usableCount_);
            rs.getFieldValue(4, questItemInfo.stackCount_);
            rs.getFieldValue(5, questItemInfo.questCode_);

            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }
            character->questInventory_.addItem(questItemInfo);
        }

        rs.nextRecordset();

        // Inventory
        ItemInfo item;
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            InvenType invenType;
            rs.getFieldValue(0, item.itemId_);
            rs.getFieldValue(1, characterId);
            rs.getFieldValue(2, invenType);
            rs.getFieldValue(3, item.itemCode_);
            rs.getFieldValue(4, item.slotId_);
            rs.getFieldValue(5, item.count_);
            rs.getFieldValue(6, item.expireTime_);

            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            if (! character->addItem(invenType, item)) {
                assert(false);
            }
        }

        rs.nextRecordset();

        // equip info
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            ObjectId itemId;
            uint16_t index = 0;
            uint8_t socketCount = 0;
            rs.getFieldValue(index++, characterId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, socketCount);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            ItemInfo* itemInfo = character->getItemInfo(itemId);

            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(C%" PRIu64 ")",
                    characterId, itemId);
                return false;
            }
            itemInfo->equipItemInfo_.socketCount_ = socketCount;
            fetchItemScriptInfo(itemInfo->equipItemInfo_.addOptions_, rs, index);
        }

        rs.nextRecordset();

        // equip socket
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            ObjectId itemId;
            SocketSlotId slotId = invalidSocketSlotId;
            uint16_t index = 0;
            rs.getFieldValue(index++, characterId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, slotId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            ItemInfo* itemInfo = character->getItemInfo(itemId);
            
            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(C%" PRIu64 ")",
                    characterId, itemId);
                return false;
            }
            
            EquipSocketInfo socketInfo;
            fetchSocketInfo(socketInfo, rs, index);
            itemInfo->equipItemInfo_.addGemItem(slotId, socketInfo);
        }

        rs.nextRecordset();

        // accessory socket
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            ObjectId itemId;
            uint16_t index = 0;
            rs.getFieldValue(index++, characterId);
            rs.getFieldValue(index++, itemId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            ItemInfo* itemInfo = character->getItemInfo(itemId);
            
            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(C%" PRIu64 ")",
                    characterId, itemId);
                return false;
            }

            fetchItemScriptInfo(itemInfo->accessoryItemInfo_.addOptions_, rs, index);
        }

        rs.nextRecordset();

        // Equipments
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, characterId);

            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            EquipPart part = epInvalid;
            rs.getFieldValue(1, part);
            ObjectId itemId = invalidObjectId;
            rs.getFieldValue(2, itemId);
            character->equipments_[part] = itemId;
        }

        rs.nextRecordset();

        // Accessries
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, characterId);

            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            AccessoryIndex part = aiInvalid;
            rs.getFieldValue(1, part);
            ObjectId itemId = invalidObjectId;
            rs.getFieldValue(2, itemId);
            if (isValid(part)) {
                character->characterAccessories_[part] = itemId;
            }
        }

        rs.nextRecordset();

        // Skills
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, characterId);

            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            SkillCode skillCode = invalidSkillCode;
            rs.getFieldValue(1, skillCode);
            character->skills_.insert(skillCode);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, characterId);

            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            ActionBar actionBar;
            fetchActionBar(actionBar, rs, 1);
            character->actionBars_.add(actionBar);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, characterId);

            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            rs.getFieldValue(1, character->gameMoney_);
            rs.getFieldValue(2, character->bankAccountInfo_.gameMoney_);
            rs.getFieldValue(3, character->eventCoin_);
            rs.getFieldValue(4, character->forgeCoin_);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, characterId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            BindRecallInfo bindRecallInfo;
            fetchBindRecall(bindRecallInfo, rs, 1);
            character->bindRecallInfos_.push_back(bindRecallInfo);
        }

        rs.nextRecordset();
        
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            rs.getFieldValue(0, characterId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            CooltimeInfo cooldownInfo;
            rs.getFieldValue(1, cooldownInfo.dataCode_);
            rs.getFieldValue(2, cooldownInfo.endTime_);

            character->cooltimeInfos_.push_back(cooldownInfo);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId characterId;
            short i = 0;
            rs.getFieldValue(i++, characterId);
            FullCharacterInfo* character = userInfo.getCharacter(characterId);
            if (! character) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find character(C%" PRIu64 ")",
                    characterId);
                return false;
            }

            RemainEffectInfo effectInfo;
            fetchApplyEffectInfo(effectInfo, rs, i);

            character->remainEffects_.push_back(effectInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    if (! userInfo.isValid()) {
        SNE_LOG_INFO("Failed to get User(A%" PRIu64 ") - Character info is not valid.",
            accountId);
        return false;
    }

    return true;
}


ErrorCode SqlServerGameDatabase::createCharacter(ObjectId& characterId,
    AccountId accountId, const Nickname& nickname, EquipType selectEquipType,
    CharacterClass characterClass, SexType sexType, const CreateCharacterEquipments& createCharacterEquipments,
    const CharacterAppearance& appearance, ZoneId zoneId, const ObjectPosition& position)
{
    assert(createCharacterEquipments.isValid());

    sne::server::Profiler profiler(__FUNCTION__);

    int32_t ec = 0;
    try {
        createCharacterCommand_.paramAccountId_.setValue(accountId);
        createCharacterCommand_.paramNickname_.setValue(nickname);
        createCharacterCommand_.paramEquipType_.setValue(selectEquipType);
        createCharacterCommand_.paramCharacterClass_.setValue(characterClass);
        createCharacterCommand_.paramSex_.setValue(sexType);

        createCharacterCommand_.paramHair_.setValue(appearance.hair_);
        createCharacterCommand_.paramFace_.setValue(appearance.face_);
        createCharacterCommand_.paramEyes_.setValue(appearance.eyes_);

        createCharacterCommand_.paramHelmet_.setValue(createCharacterEquipments[epHelmet].equipCode_);
        createCharacterCommand_.paramShoulder_.setValue(createCharacterEquipments[epShoulder].equipCode_);
        createCharacterCommand_.paramArmor_.setValue(createCharacterEquipments[epArmor].equipCode_);
        createCharacterCommand_.paramWaist_.setValue(createCharacterEquipments[epWaist].equipCode_);
        createCharacterCommand_.paramTrousers_.setValue(createCharacterEquipments[epTrousers].equipCode_);
        createCharacterCommand_.paramGaiters_.setValue(createCharacterEquipments[epGaiters].equipCode_);
        createCharacterCommand_.paramGloves_.setValue(createCharacterEquipments[epGloves].equipCode_);
        createCharacterCommand_.paramShoes_.setValue(createCharacterEquipments[epShoes].equipCode_);
        createCharacterCommand_.paramRightHand_.setValue(createCharacterEquipments[epRightHand].equipCode_);
        createCharacterCommand_.paramLeftHand_.setValue(createCharacterEquipments[epLeftHand].equipCode_);
        createCharacterCommand_.paramBothHands_.setValue(createCharacterEquipments[epTwoHands].equipCode_);

        createCharacterCommand_.paramHelmetId_.setValue(createCharacterEquipments[epHelmet].equipId_);
        createCharacterCommand_.paramShoulderId_.setValue(createCharacterEquipments[epShoulder].equipId_);
        createCharacterCommand_.paramArmorId_.setValue(createCharacterEquipments[epArmor].equipId_);
        createCharacterCommand_.paramWaistId_.setValue(createCharacterEquipments[epWaist].equipId_);
        createCharacterCommand_.paramTrousersId_.setValue(createCharacterEquipments[epTrousers].equipId_);
        createCharacterCommand_.paramGaitersId_.setValue(createCharacterEquipments[epGaiters].equipId_);
        createCharacterCommand_.paramGlovesId_.setValue(createCharacterEquipments[epGloves].equipId_);
        createCharacterCommand_.paramShoesId_.setValue(createCharacterEquipments[epShoes].equipId_);
        createCharacterCommand_.paramRightHandId_.setValue(createCharacterEquipments[epRightHand].equipId_);
        createCharacterCommand_.paramLeftHandId_.setValue(createCharacterEquipments[epLeftHand].equipId_);
        createCharacterCommand_.paramBothHandsId_.setValue(createCharacterEquipments[epTwoHands].equipId_);

        createCharacterCommand_.paramZoneId_.setValue(zoneId);
        createCharacterCommand_.paramPosX_.setValue(position.x_);
        createCharacterCommand_.paramPosY_.setValue(position.y_);
        createCharacterCommand_.paramPosZ_.setValue(position.z_);
        createCharacterCommand_.paramPosHeading_.setValue(position.heading_);

        createCharacterCommand_.paramInventoryCount_.setValue(defultInvenSlotId);

        createCharacterCommand_.execute();

        createCharacterCommand_.paramErrorCode_.getValue(ec);
        createCharacterCommand_.paramCharacterId_.getValue(characterId);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return ecDatabaseInternalError;
    }

    const ErrorCode errorCode = toErrorCodeFromCreateCharacterError(ec);
    return errorCode;
}


ErrorCode SqlServerGameDatabase::deleteCharacter(GuildId& guildId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    int32_t ec = 0;
    try {
        deleteCharacterCommand_.paramCharacterId_.setValue(characterId);

        deleteCharacterCommand_.execute();

        deleteCharacterCommand_.paramErrorCode_.getValue(ec);
        deleteCharacterCommand_.paramGuildId_.getValue(guildId);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return ecDatabaseInternalError;
    }

    return ec == 0 ? ecOk : ecDatabaseInternalError;
}


ErrorCode SqlServerGameDatabase::checkDuplicateNickname(const Nickname& nickname)
{
    sne::server::Profiler profiler(__FUNCTION__);

    bool isExistNickname = true;
    try {
        checkDuplicateNicknameCommand_.paramNickname_.setValue(nickname);

        checkDuplicateNicknameCommand_.execute();

        checkDuplicateNicknameCommand_.paramExistNickname_.getValue(isExistNickname);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return ecDatabaseInternalError;
    }

    return isExistNickname ? ecCharacterDuplicatedNickname : ecOk;
}


bool SqlServerGameDatabase::moveInventoryItem(ObjectId itemId, SlotId slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        moveItemCommand_.paramItemId_.setValue(itemId);
        moveItemCommand_.paramSlotId_.setValue(uint8_t(slotId));

        moveItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::saveCharacterStats(AccountId accountId, ObjectId characterId,
    const DBCharacterStats& saveInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        saveCharacterStatsCommand_.paramAccountId_.setValue(accountId);
        saveCharacterStatsCommand_.paramCharacterId_.setValue(characterId);
        saveCharacterStatsCommand_.paramLevel_.setValue(saveInfo.level_);
        saveCharacterStatsCommand_.paramLevelStep_.setValue(saveInfo.levelStep_);
        saveCharacterStatsCommand_.paramExp_.setValue(saveInfo.exp_);
        saveCharacterStatsCommand_.paramCharacterState_.setValue(saveInfo.state_);
        saveCharacterStatsCommand_.paramSp_.setValue(saveInfo.sp_);
        saveCharacterStatsCommand_.paramHp_.setValue(saveInfo.currentPoints_.hp_);
        saveCharacterStatsCommand_.paramMp_.setValue(saveInfo.currentPoints_.mp_);
        saveCharacterStatsCommand_.paramPosX_.setValue(saveInfo.position_.x_);
        saveCharacterStatsCommand_.paramPosY_.setValue(saveInfo.position_.y_);
        saveCharacterStatsCommand_.paramPosZ_.setValue(saveInfo.position_.z_);
        saveCharacterStatsCommand_.paramPosHeading_.setValue(uint8_t(saveInfo.position_.heading_));
        saveCharacterStatsCommand_.paramZoneId_.setValue(saveInfo.zoneId_);
        saveCharacterStatsCommand_.paramGameMoney_.setValue(saveInfo.gameMoney_);
        saveCharacterStatsCommand_.paramBankGameMoney_.setValue(saveInfo.bankGameMoney_);
        saveCharacterStatsCommand_.paramChaotic_.setValue(saveInfo.chaotic_);
        saveCharacterStatsCommand_.paramIsActionBarLock_.setValue(saveInfo.isActionBarLocked_);
        saveCharacterStatsCommand_.paramCharacterClass_.setValue(saveInfo.cc_);
        saveCharacterStatsCommand_.paramArenaPoint_.setValue(saveInfo.arenaPoint_);
        saveCharacterStatsCommand_.paramEventCoin_.setValue(saveInfo.eventCoin_);
        saveCharacterStatsCommand_.paramForgeCoin_.setValue(saveInfo.forgeCoin_);

        saveCharacterStatsCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::loadCharacterProperties(std::string& config, std::string& prefs, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        loadCharacterPropertiesCommand_.paramCharacterId_.setValue(characterId);

        loadCharacterPropertiesCommand_.execute();

        loadCharacterPropertiesCommand_.paramConfig_.getValue(config);
        loadCharacterPropertiesCommand_.paramPrefs_.getValue(prefs);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::saveCharacterProperties(AccountId accountId, ObjectId characterId,
    const std::string& config, const std::string& prefs)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        saveCharacterPropertiesCommand_.paramAccountId_.setValue(accountId);
        saveCharacterPropertiesCommand_.paramCharacterId_.setValue(characterId);
        saveCharacterPropertiesCommand_.paramConfig_.setValue(config);
        saveCharacterPropertiesCommand_.paramPrefs_.setValue(prefs);

        saveCharacterPropertiesCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::switchInventoryItem(
    ObjectId itemId1, SlotId slotId1, ObjectId itemId2, SlotId slotId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        switchItemCommand_.paramItemId1_.setValue(itemId1);
        switchItemCommand_.paramSlotId1_.setValue(slotId1);
        switchItemCommand_.paramItemId2_.setValue(itemId2);
        switchItemCommand_.paramSlotId2_.setValue(slotId2);

        switchItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addInventoryItem(AccountId accountId, ObjectId characterId,
    const ItemInfo& itemInfo, InvenType invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addItemCommand_.paramAccountId_.setValue(accountId);
        addItemCommand_.paramCharacterId_.setValue(characterId);
        addItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addItemCommand_.paramSlotId1_.setValue(uint8_t(itemInfo.slotId_));
        addItemCommand_.paramItemCount_.setValue(uint8_t(itemInfo.count_));
        addItemCommand_.paramInvenType_.setValue(invenType);
        addItemCommand_.expireAt_.setValue(uint64_t(itemInfo.expireTime_));
        addItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addInventoryEquipItem(AccountId accountId, ObjectId characterId,
    const ItemInfo& itemInfo, InvenType invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addEquipItemCommand_.paramAccountId_.setValue(accountId);
        addEquipItemCommand_.paramCharacterId_.setValue(characterId);
        addEquipItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addEquipItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addEquipItemCommand_.paramSlotId_.setValue(itemInfo.slotId_);
        addEquipItemCommand_.paramItemCount_.setValue(itemInfo.count_);
        addEquipItemCommand_.expireAt_.setValue(itemInfo.expireTime_);

        setOptionValue(addEquipItemCommand_, itemInfo.equipItemInfo_.addOptions_);
        addEquipItemCommand_.paramSocketCount_.setValue(itemInfo.equipItemInfo_.socketCount_);
        addEquipItemCommand_.paramInvenType_.setValue(invenType);

        addEquipItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}

bool SqlServerGameDatabase::addEquipSocketOption(AccountId accountId, ObjectId characterId, 
    ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addEquipSocketOptionCommand_.paramAccountId_.setValue(accountId);
        addEquipSocketOptionCommand_.paramCharacterId_.setValue(characterId);
        addEquipSocketOptionCommand_.paramItemId_.setValue(itemId);
        addEquipSocketOptionCommand_.paramSlotId_.setValue(slotId);
        addEquipSocketOptionCommand_.paramGemCode_.setValue(socketInfo.gemCode_);
        setOptionValue(addEquipSocketOptionCommand_, socketInfo.scriptInfos_);
        
        addEquipSocketOptionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::removeEquipSocketOption(ObjectId itemId, SocketSlotId slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeEquipSocketOptionCommand_.paramItemId_.setValue(itemId);
        removeEquipSocketOptionCommand_.paramSlotId_.setValue(slotId);
        removeEquipSocketOptionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::addInventoryAccessoryItem(AccountId accountId, ObjectId characterId, const ItemInfo& itemInfo,
    InvenType invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addAccessoryItemCommand_.paramAccountId_.setValue(accountId);
        addAccessoryItemCommand_.paramCharacterId_.setValue(characterId);
        addAccessoryItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addAccessoryItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addAccessoryItemCommand_.paramSlotId1_.setValue(itemInfo.slotId_);
        addAccessoryItemCommand_.paramItemCount_.setValue(itemInfo.count_);
        addAccessoryItemCommand_.expireAt_.setValue(itemInfo.expireTime_);

        setOptionValue(addAccessoryItemCommand_, itemInfo.accessoryItemInfo_.addOptions_);

        addAccessoryItemCommand_.paramInvenType_.setValue(invenType);

        addAccessoryItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::changeEquipItemInfo(ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        changeEquipItemInfoCommand_.paramItemId_.setValue(itemId);
        changeEquipItemInfoCommand_.paramNewItemCode_.setValue(newEquipCode);
        changeEquipItemInfoCommand_.paramSocketCount_.setValue(socketCount);
        changeEquipItemInfoCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeInventoryItem(ObjectId itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeItemCommand_.paramItemId_.setValue(itemId);

        removeItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateInventoryItemCount(ObjectId itemId, uint8_t itemCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateItemCountCommand_.paramItemId_.setValue(itemId);
        updateItemCountCommand_.paramItemCount_.setValue(itemCount);

        updateItemCountCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addQuestItem(AccountId accountId, ObjectId characterId,
    const QuestItemInfo& questItemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addQuestItemCommand_.paramAccountId_.setValue(accountId);
        addQuestItemCommand_.paramCharacterId_.setValue(characterId);
        addQuestItemCommand_.paramItemId_.setValue(questItemInfo.itemId_);
        addQuestItemCommand_.paramQuestItemCode_.setValue(questItemInfo.questItemCode_);
        addQuestItemCommand_.paramUsableCount_.setValue(uint8_t(questItemInfo.usableCount_));
        addQuestItemCommand_.paramStackCount_.setValue(uint8_t(questItemInfo.stackCount_));
        addQuestItemCommand_.paramQuestCode_.setValue(questItemInfo.questCode_);

        addQuestItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeQuestItem(ObjectId itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeQuestItemCommand_.paramItemId_.setValue(itemId);

        removeQuestItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateQuestItemUsableCount(ObjectId itemId, uint8_t usableCount)
{
    sne::server::Profiler profiler(__FUNCTION__);
    try {
        updateQuestItemUsableCountCommand_.paramItemId_.setValue(itemId);
        updateQuestItemUsableCountCommand_.paramUsableCount_.setValue(usableCount);

        updateQuestItemUsableCountCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateQuestItemCount(ObjectId itemId, uint8_t stackCount)
{
    sne::server::Profiler profiler(__FUNCTION__);
    try {
        updateQuestItemCountCommand_.paramItemId_.setValue(itemId);
        updateQuestItemCountCommand_.paramStackCount_.setValue(stackCount);

        updateQuestItemCountCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::equipItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, EquipPart equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        equipItemCommand_.paramAccountId_.setValue(accountId);
        equipItemCommand_.paramCharacterId_.setValue(characterId);
        equipItemCommand_.paramItemId_.setValue(itemId);
        equipItemCommand_.paramEquipPart_.setValue(uint8_t(equipPart));

        equipItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::unequipItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, SlotId slotId, EquipPart unequipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        unequipItemCommand_.paramAccountId_.setValue(accountId);
        unequipItemCommand_.paramCharacterId_.setValue(characterId);
        unequipItemCommand_.paramItemId_.setValue(itemId);
        unequipItemCommand_.paramSlotId_.setValue(uint8_t(slotId));
        unequipItemCommand_.paramEquipPart_.setValue(uint8_t(unequipPart));

        unequipItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::replaceInventoryWithEquipItem(AccountId accountId,
    ObjectId characterId, ObjectId invenItemId, EquipPart unequipPart, SlotId slotId,
    ObjectId equipItemId, EquipPart equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        replaceInvenWithEquipItemCommand_.paramAccountId_.setValue(accountId);
        replaceInvenWithEquipItemCommand_.paramCharacterId_.setValue(characterId);
        replaceInvenWithEquipItemCommand_.paramInvenItemId_.setValue(invenItemId);
        replaceInvenWithEquipItemCommand_.paramEquipItemId_.setValue(equipItemId);
        replaceInvenWithEquipItemCommand_.paramSlotId_.setValue(uint8_t(slotId));
        replaceInvenWithEquipItemCommand_.paramEquipPart_.setValue(uint8_t(equipPart));
        replaceInvenWithEquipItemCommand_.paramUnequipPart_.setValue(uint8_t(unequipPart));

        replaceInvenWithEquipItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::equipAccessoryItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, AccessoryIndex equipIndex)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        equipAccessoryItemCommand_.paramAccountId_.setValue(accountId);
        equipAccessoryItemCommand_.paramCharacterId_.setValue(characterId);
        equipAccessoryItemCommand_.paramItemId_.setValue(itemId);
        equipAccessoryItemCommand_.paramAccessoryPart_.setValue(uint8_t(equipIndex));

        equipAccessoryItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::unequipAccessoryItem(AccountId accountId, ObjectId characterId,
    ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        unequipAccessoryItemCommand_.paramAccountId_.setValue(accountId);
        unequipAccessoryItemCommand_.paramCharacterId_.setValue(characterId);
        unequipAccessoryItemCommand_.paramItemId_.setValue(itemId);
        unequipAccessoryItemCommand_.paramSlotId_.setValue(uint8_t(slotId));
        unequipAccessoryItemCommand_.paramAccessoryPart_.setValue(uint8_t(unequipPart));

        unequipAccessoryItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::replaceInventoryWithAccessoryItem(AccountId accountId,
    ObjectId characterId, ObjectId invenItemId, AccessoryIndex unequipPart, SlotId slotId,
    ObjectId equipItemId, AccessoryIndex equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        replaceInventoryWithAccessoryItemCommand_.paramAccountId_.setValue(accountId);
        replaceInventoryWithAccessoryItemCommand_.paramCharacterId_.setValue(characterId);
        replaceInventoryWithAccessoryItemCommand_.paramInvenItemId_.setValue(invenItemId);
        replaceInventoryWithAccessoryItemCommand_.paramAccessoryItemId_.setValue(equipItemId);
        replaceInventoryWithAccessoryItemCommand_.paramSlotId_.setValue(uint8_t(slotId));
        replaceInventoryWithAccessoryItemCommand_.paramAccessoryPart_.setValue(uint8_t(equipPart));
        replaceInventoryWithAccessoryItemCommand_.paramUnequipPart_.setValue(uint8_t(unequipPart));

        replaceInventoryWithAccessoryItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}



bool SqlServerGameDatabase::changeCharacterState(ObjectId characterId, CreatureStateType state)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        changeCharacterStateCommand_.paramCharacterId_.setValue(characterId);
        changeCharacterStateCommand_.paramCharacterState_.setValue(state);
       
        changeCharacterStateCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::saveActionBar(ObjectId characterId, 
    ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        saveActionBarCommand_.paramCharacterId_.setValue(characterId);
        saveActionBarCommand_.paramActionBarIndex_.setValue(abiIndex);
        saveActionBarCommand_.paramActionBarPosition_.setValue(abpIndex);
        saveActionBarCommand_.paramCode_.setValue(code);

        saveActionBarCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::learnSkill(AccountId accountId, ObjectId characterId,
    SkillCode currentSkillCode, SkillCode learnSkillCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        learnSkillCommand_.paramAccountId_.setValue(accountId);
        learnSkillCommand_.paramCharacterId_.setValue(characterId);
        learnSkillCommand_.paramCurrentSkillCode_.setValue(currentSkillCode);
        learnSkillCommand_.paramLearnSkillCode_.setValue(learnSkillCode);

        learnSkillCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;    
}


bool SqlServerGameDatabase::removeSkill(ObjectId characterId, SkillCode skillCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeSkillCommand_.paramCharacterId_.setValue(characterId);
        removeSkillCommand_.paramSkillCode_.setValue(skillCode);

        removeSkillCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;   
}


bool SqlServerGameDatabase::removeAllSkill(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeAllSkillCommand_.paramCharacterId_.setValue(characterId);

        removeAllSkillCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;   
}


bool SqlServerGameDatabase::getCharacterInfo(FullCharacterInfo& characterInfo,
    ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    characterInfo.objectId_ = characterId;
    if (! isValidObjectId(characterId)) {
        return false;
    }

    try {
        getCharacterInfoCommand_.paramCharacterId_.setValue(characterId);

        getCharacterInfoCommand_.execute();

        sne::database::AdoRecordSet& rs = getCharacterInfoCommand_.adoRecordSet_;
        
        fetchCharacterInfo(characterInfo, rs, 0);
 
        rs.nextRecordset();

        rs.getFieldValue(0, characterInfo.appearance_.hair_);
        rs.getFieldValue(1, characterInfo.appearance_.face_);
        rs.getFieldValue(2, characterInfo.appearance_.eyes_);

        rs.nextRecordset();
        
        characterInfo.bankAccountInfo_.defaultInvenCount_ = maxBankSlotId;

        for (; ! rs.isEof(); rs.moveNext()) {
            fetchVehicleInventory(characterInfo.vehicleInventory_, rs, 0);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            GliderInfo info;
            rs.getFieldValue(0, info.objectId_);
            rs.getFieldValue(1, info.gliderCode_);
            rs.getFieldValue(2, info.durability_);
            characterInfo.gliderInventory_.add(info);
        }


        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            InvenType invenType;
            rs.getFieldValue(0, invenType);
            assert(invenType == itPc);
            rs.getFieldValue(1, characterInfo.inventory_.defaultInvenCount_);
            rs.getFieldValue(2, characterInfo.inventory_.freeSlotCount_);
            rs.getFieldValue(3, characterInfo.inventory_.cashSlotCount_);
        }

        rs.nextRecordset();


        QuestItemInfo questItemInfo;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, questItemInfo.itemId_);            
            rs.getFieldValue(1, questItemInfo.questItemCode_);
            rs.getFieldValue(2, questItemInfo.usableCount_);
            rs.getFieldValue(3, questItemInfo.stackCount_);
            rs.getFieldValue(4, questItemInfo.questCode_);

            characterInfo.questInventory_.addItem(questItemInfo);
        }

        rs.nextRecordset();

        // Inventory
        ItemInfo item;
        for (; ! rs.isEof(); rs.moveNext()) {
            InvenType invenType;
            rs.getFieldValue(0, invenType);
            rs.getFieldValue(1, item.itemId_);
            rs.getFieldValue(2, item.itemCode_);
            rs.getFieldValue(3, item.slotId_);
            rs.getFieldValue(4, item.count_);
            rs.getFieldValue(5, item.expireTime_);

            if (! characterInfo.addItem(invenType, item)) {
                assert(false);
            }		
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId itemId;
            uint16_t index = 0;
            uint8_t socketCount = 0;
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, socketCount);

            ItemInfo* itemInfo = characterInfo.inventory_.getItemInfo(itemId);
                        
            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(C%" PRIu64 ")",
                    characterId, itemId);
                return false;
            }
            itemInfo->equipItemInfo_.socketCount_ = socketCount;
            fetchItemScriptInfo(itemInfo->equipItemInfo_.addOptions_, rs, index);
        }

        rs.nextRecordset();

        // equip socketInfo
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId itemId;

            uint16_t index = 0;
            SocketSlotId slotId = invalidSocketSlotId;
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, slotId);

            ItemInfo* itemInfo = characterInfo.getItemInfo(itemId);
            
            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(C%" PRIu64 ")",
                    characterId, itemId);
                return false;
            }

            EquipSocketInfo socketInfo;
            fetchSocketInfo(socketInfo, rs, index);
            itemInfo->equipItemInfo_.addGemItem(slotId, socketInfo);
        }

        rs.nextRecordset();
        
        // accessory
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId itemId;
            uint16_t index = 0;
            rs.getFieldValue(index++, itemId);

            ItemInfo* itemInfo = characterInfo.inventory_.getItemInfo(itemId);
            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(C%" PRIu64 ")",
                    characterId, itemId);
                return false;
            }

            fetchItemScriptInfo(itemInfo->accessoryItemInfo_.addOptions_, rs, index);
        }


        rs.nextRecordset();

        // Equipments
        for (; ! rs.isEof(); rs.moveNext()) {
            EquipPart part = epInvalid;
            rs.getFieldValue(0, part);
            ObjectId itemId = invalidObjectId;
            rs.getFieldValue(1, itemId);
            characterInfo.equipments_[part] = itemId;
        }

        rs.nextRecordset();


        // Accessories
        for (; ! rs.isEof(); rs.moveNext()) {
            AccessoryIndex part = aiInvalid;
            rs.getFieldValue(0, part);
            ObjectId itemId = invalidObjectId;
            rs.getFieldValue(1, itemId);
            if (isValid(part)) {
                characterInfo.characterAccessories_[part] = itemId;
            }
        }

        rs.nextRecordset();

        // Skills
        for (; ! rs.isEof(); rs.moveNext()) {
            SkillCode skillCode = invalidSkillCode;
            rs.getFieldValue(0, skillCode);
            characterInfo.skills_.insert(skillCode);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ActionBar actionBar;
            fetchActionBar(actionBar, rs, 0);
            characterInfo.actionBars_.add(actionBar);
        }

        rs.nextRecordset();

        rs.getFieldValue(0, characterInfo.gameMoney_);
        rs.getFieldValue(1, characterInfo.bankAccountInfo_.gameMoney_);
        rs.getFieldValue(2, characterInfo.eventCoin_);
        rs.getFieldValue(3, characterInfo.forgeCoin_);

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            BindRecallInfo bindRecallInfo;
            fetchBindRecall(bindRecallInfo, rs, 0);
            characterInfo.bindRecallInfos_.push_back(bindRecallInfo);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            CooltimeInfo cooldownInfo;
            rs.getFieldValue(0, cooldownInfo.dataCode_);
            rs.getFieldValue(1, cooldownInfo.endTime_);

            characterInfo.cooltimeInfos_.push_back(cooldownInfo);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            RemainEffectInfo effectInfo;
            fetchApplyEffectInfo(effectInfo, rs, 0);
            characterInfo.remainEffects_.push_back(effectInfo);
        }

    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    if (! characterInfo.isValid()) {
        SNE_LOG_INFO("Failed to get Character(C%" PRIu64 ") - Character info is not valid.",
            characterId);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateCharacterPoints(ObjectId characterId, const Points& points)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateCharacterPointsCommand_.paramCharacterId_.setValue(characterId);
        updateCharacterPointsCommand_.hp_.setValue(points.hp_);
        updateCharacterPointsCommand_.mp_.setValue(points.mp_);

        updateCharacterPointsCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::saveSelectCharacterTitle(ObjectId characterId, const CharacterTitleCode titleCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        saveSelectCharacterTitleCommand_.paramCharacterId_.setValue(characterId);
        saveSelectCharacterTitleCommand_.paramTitleCode_.setValue(titleCode);
        
        saveSelectCharacterTitleCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}

bool SqlServerGameDatabase::getQuests(ObjectId characterId, QuestCodes& questCodes,
    AcceptedQuestInfos& acceptQuestInfos)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getQuestsCommand_.paramCharacterId_.setValue(characterId);
        getQuestsCommand_.execute();
        sne::database::AdoRecordSet& rs = getQuestsCommand_.adoRecordSet_;

        for (; ! rs.isEof(); rs.moveNext()) {
            QuestCode questCode;
            rs.getFieldValue(0, questCode);
            questCodes.insert(questCode);
        }

        rs.nextRecordset();		
        AcceptedQuestInfo questInfo;
        for (; ! rs.isEof(); rs.moveNext()) {
            QuestCode questCode;
            rs.getFieldValue(0, questCode);
            questInfo.questCode_ = questCode;
            acceptQuestInfos.push_back(questInfo);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            QuestCode questCode = invalidQuestCode;

            rs.getFieldValue(0, questCode);
            AcceptedQuestInfo* info = getAcceptedQuestInfo(acceptQuestInfos, questCode);
            if (! info) {                          
                SNE_LOG_ERROR("getQuests find failed AcceptedQuestInfo(c%u, q:%d)",
                    characterId, questCode);
                continue;
            }

            QuestMissionCode missionCode = invalidQuestMissionCode;
            QuestGoalInfo goalInfo;

            rs.getFieldValue(1, missionCode);
            rs.getFieldValue(2, goalInfo.goalCode_);
            rs.getFieldValue(3, goalInfo.goalValue_);

            QuestGoalInfosMap::iterator pos = info->questGoalInfosMap_.find(missionCode);
            if (pos == info->questGoalInfosMap_.end()) {
                QuestGoalInfos infos;
                infos.push_back(goalInfo);
                info->questGoalInfosMap_.emplace(missionCode, infos);
            }
            else {
                QuestGoalInfos& infos = (*pos).second;
                infos.push_back(goalInfo);
            }
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getRepeatQuests(ObjectId characterId, 
    CompletedRepeatQuestInfos& completeRepeatQuestInfos,
    AcceptedRepeatQuestInfos& acceptQuestInfos, QuestCodes& removeQuestCodes)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getRepeatQuestsCommand_.paramCharacterId_.setValue(characterId);
        getRepeatQuestsCommand_.execute();
        sne::database::AdoRecordSet& rs = getRepeatQuestsCommand_.adoRecordSet_;

        for (; ! rs.isEof(); rs.moveNext()) {
            QuestCode questCode;
            sec_t acceptTime;
            rs.getFieldValue(0, questCode);
            rs.getFieldValue(1, acceptTime);
            
            const sec_t now = getTime();
            struct tm* nowInfo = _localtime64(&now);
            struct tm* acceptInfo = _localtime64(&acceptTime);
            if (! nowInfo || ! acceptInfo) {
                // TODO 로그
                continue;
            }
            
            if (nowInfo->tm_yday != acceptInfo->tm_yday) {
                removeQuestCodes.insert(questCode);
            }
            else {
                completeRepeatQuestInfos.insert(CompletedRepeatQuestInfo(questCode, acceptTime));
            }
        }

        rs.nextRecordset();		
        AcceptedRepeatQuestInfo questInfo;
        for (; ! rs.isEof(); rs.moveNext()) {
            QuestCode questCode;
            sec_t acceptTime;
            rs.getFieldValue(0, questCode);
            rs.getFieldValue(1, acceptTime);
            questInfo.questCode_ = questCode;
            questInfo.acceptTime_ = acceptTime;
            acceptQuestInfos.push_back(questInfo);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            QuestCode questCode = invalidQuestCode;

            rs.getFieldValue(0, questCode);
            AcceptedRepeatQuestInfo* info = getAcceptedRepeatQuestInfo(acceptQuestInfos, questCode);
            if (! info) {                          
                SNE_LOG_DEBUG("getRepeatQuests find failed AcceptedQuestInfo(c%u, q:%d)",
                    characterId, questCode);
                continue;
            }

            QuestMissionCode missionCode = invalidQuestMissionCode;
            QuestGoalInfo goalInfo;

            rs.getFieldValue(1, missionCode);
            rs.getFieldValue(2, goalInfo.goalCode_);
            rs.getFieldValue(3, goalInfo.goalValue_);

            QuestGoalInfosMap::iterator pos = info->questGoalInfosMap_.find(missionCode);
            if (pos == info->questGoalInfosMap_.end()) {
                QuestGoalInfos infos;
                infos.push_back(goalInfo);
                info->questGoalInfosMap_.emplace(missionCode, infos);
            }
            else {
                QuestGoalInfos& infos = (*pos).second;
                infos.push_back(goalInfo);
            }
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::acceptQuest(ObjectId characterId, QuestCode questCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        acceptQuestCommand_.paramCharacterId_.setValue(characterId);
        acceptQuestCommand_.paramQuestCode_.setValue(questCode);

        acceptQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::acceptRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t gameTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        acceptRepeatQuestCommand_.paramCharacterId_.setValue(characterId);
        acceptRepeatQuestCommand_.paramQuestCode_.setValue(questCode);
        acceptRepeatQuestCommand_.paramAcceptTime_.setValue(gameTime);

        acceptRepeatQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::cancelQuest(ObjectId characterId, QuestCode questCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        cancelQuestCommand_.paramCharacterId_.setValue(characterId);
        cancelQuestCommand_.paramQuestCode_.setValue(questCode);

        cancelQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::cancelRepeatQuest(ObjectId characterId, QuestCode questCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        cancelRepeatQuestCommand_.paramCharacterId_.setValue(characterId);
        cancelRepeatQuestCommand_.paramQuestCode_.setValue(questCode);

        cancelRepeatQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::completeQuest(ObjectId characterId, QuestCode questCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        completeQuestCommand_.paramCharacterId_.setValue(characterId);
        completeQuestCommand_.paramQuestCode_.setValue(questCode);

        completeQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::completeRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        completeRepeatQuestCommand_.paramCharacterId_.setValue(characterId);
        completeRepeatQuestCommand_.paramQuestCode_.setValue(questCode);
        completeRepeatQuestCommand_.paramAcceptTime_.setValue(acceptTime);

        completeRepeatQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateCompleteRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateCompleteRepeatQuestCommand_.paramCharacterId_.setValue(characterId);
        updateCompleteRepeatQuestCommand_.paramQuestCode_.setValue(questCode);
        updateCompleteRepeatQuestCommand_.paramAcceptTime_.setValue(acceptTime);

        updateCompleteRepeatQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::insertQuestMission(ObjectId characterId, QuestCode questCode,
    QuestMissionCode missionCode, const QuestGoalInfo& goalInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        insertQuestMissionCommand_.paramCharacterId_.setValue(characterId);
        insertQuestMissionCommand_.paramQuestCode_.setValue(questCode);
        insertQuestMissionCommand_.paramQuestMissionCode_.setValue(missionCode);
        insertQuestMissionCommand_.paramQuestGoalCode_.setValue(goalInfo.goalCode_);
        insertQuestMissionCommand_.paramQuestGoalValue_.setValue(goalInfo.goalValue_);

        insertQuestMissionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::insertRepeatQuestMission(ObjectId characterId, QuestCode questCode,
    QuestMissionCode missionCode, const QuestGoalInfo& goalInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        insertRepeatQuestMissionCommand_.paramCharacterId_.setValue(characterId);
        insertRepeatQuestMissionCommand_.paramQuestCode_.setValue(questCode);
        insertRepeatQuestMissionCommand_.paramQuestMissionCode_.setValue(missionCode);
        insertRepeatQuestMissionCommand_.paramQuestGoalCode_.setValue(goalInfo.goalCode_);
        insertRepeatQuestMissionCommand_.paramQuestGoalValue_.setValue(goalInfo.goalValue_);

        insertRepeatQuestMissionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateQuestMission(ObjectId characterId, QuestCode questCode,
    QuestMissionCode missionCode, const QuestGoalInfo& goalInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateQuestMissionCommand_.paramCharacterId_.setValue(characterId);
        updateQuestMissionCommand_.paramQuestCode_.setValue(questCode);
        updateQuestMissionCommand_.paramQuestMissionCode_.setValue(missionCode);
        updateQuestMissionCommand_.paramQuestGoalCode_.setValue(goalInfo.goalCode_);
        updateQuestMissionCommand_.paramQuestGoalValue_.setValue(goalInfo.goalValue_);

        updateQuestMissionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateRepeatQuestMission(ObjectId characterId, QuestCode questCode,
    QuestMissionCode missionCode, const QuestGoalInfo& goalInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateRepeatQuestMissionCommand_.paramCharacterId_.setValue(characterId);
        updateRepeatQuestMissionCommand_.paramQuestCode_.setValue(questCode);
        updateRepeatQuestMissionCommand_.paramQuestMissionCode_.setValue(missionCode);
        updateRepeatQuestMissionCommand_.paramQuestGoalCode_.setValue(goalInfo.goalCode_);
        updateRepeatQuestMissionCommand_.paramQuestGoalValue_.setValue(goalInfo.goalValue_);

        updateRepeatQuestMissionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeCompleteRepeatQuest(ObjectId characterId, QuestCode questCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeCompleteRepeatQuest_.paramCharacterId_.setValue(characterId);
        removeCompleteRepeatQuest_.paramQuestCode_.setValue(questCode);

        removeCompleteRepeatQuest_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeCompleteQuest(ObjectId characterId, QuestCode questCode)
{
        sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeCompleteQuestCommand_.paramCharacterId_.setValue(characterId);
        removeCompleteQuestCommand_.paramQuestCode_.setValue(questCode);


        removeCompleteQuestCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getGuildInfos(GuildInfos& guildInfos)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildInfosCommand_.execute();

        GuildInfo guildInfo;
        sne::database::AdoRecordSet& rs = getGuildInfosCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, guildInfo.guildId_);
            rs.getFieldValue(1, guildInfo.guildName_.ref());
            rs.getFieldValue(2, guildInfo.guildMarkCode_);
            rs.getFieldValue(3, guildInfo.introduction_.ref());
            rs.getFieldValue(4, guildInfo.notice_.ref());
            rs.getFieldValue(5, guildInfo.gameMoney_);
            rs.getFieldValue(6, guildInfo.levelInfo_.guildExp_);
            rs.getFieldValue(7, guildInfo.levelInfo_.dayGuildAddExp_);
            rs.getFieldValue(8, guildInfo.levelInfo_.lastUpdateExpTime_);
            rs.getFieldValue(9, guildInfo.levelInfo_.skillPoint_);

            guildInfos.push_back(guildInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::getGuildRanks(GuildInfo& guildInfo, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildRanksCommand_.paramGuildId_.setValue(guildId);

        getGuildRanksCommand_.execute();

        GuildRankInfo info;
        sne::database::AdoRecordSet& rs = getGuildRanksCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, info.rankId_);
            rs.getFieldValue(1, info.name_.ref());
            rs.getFieldValue(2, info.rights_);
            rs.getFieldValue(3, info.goldWithdrawalPerDay_);
            guildInfo.rankInfos_.push_back(info);
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getGuildBankRights(GuildInfo& guildInfo, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildBankRightsCommand_.paramGuildId_.setValue(guildId);

        getGuildBankRightsCommand_.execute();

        sne::database::AdoRecordSet& rs = getGuildBankRightsCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            GuildRankId rankId = invalidGuildRankId;
            rs.getFieldValue(0, rankId);
            GuildRankInfo& rankInfo = guildInfo.rankInfos_[rankId];
            GuildBankVaultRightInfo rightInfo;
            VaultId vaultId = invalidVaultId;
            rs.getFieldValue(1, vaultId);             
            rs.getFieldValue(2, rightInfo.rights_);
            rs.getFieldValue(3, rightInfo.itemWithdrawalPerDay_);
            rankInfo.vaultRights_.push_back(rightInfo);
            if (rankInfo.vaultRights_.size() != vaultId + 1) {
                assert(false);
            }
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}



bool SqlServerGameDatabase::getGuildSkills(GuildInfo& guildInfo, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildSkillsCommand_.paramGuildId_.setValue(guildId);

        getGuildSkillsCommand_.execute();

        sne::database::AdoRecordSet& rs = getGuildSkillsCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            SkillCode skillCode = invalidSkillCode;
            rs.getFieldValue(0, skillCode);
            guildInfo.guildSkillCodes_.push_back(skillCode);
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}

bool SqlServerGameDatabase::getGuildMemberInfos(GuildId guildId, GuildMemberInfos& guildMemberInfos)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildMemberInfosCommand_.paramGuildId_.setValue(guildId);

        getGuildMemberInfosCommand_.execute();

        GuildMemberInfo guildMemberInfo;
        sne::database::AdoRecordSet& rs = getGuildMemberInfosCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, guildMemberInfo.playerId_);
            rs.getFieldValue(1, guildMemberInfo.playerNickname_.ref());
            rs.getFieldValue(2, guildMemberInfo.characterClass_);
            rs.getFieldValue(3, guildMemberInfo.position_);
            rs.getFieldValue(4, guildMemberInfo.rankId_);
            rs.getFieldValue(5, guildMemberInfo.contributiveness_);
            rs.getFieldValue(6, guildMemberInfo.resetDayWithdrawTime_);
            rs.getFieldValue(7, guildMemberInfo.dayWithdraw_);
            rs.getFieldValue(8, guildMemberInfo.level_);
            guildMemberInfos.push_back(guildMemberInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::getGuildRelationships(GuildId guildId, GuildRelationshipInfos& relationInfos)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildRelationshipsCommand_.paramGuildId_.setValue(guildId);

        getGuildRelationshipsCommand_.execute();

        GuildRelationshipInfo relationship;
        sne::database::AdoRecordSet& rs = getGuildRelationshipsCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, relationship.targetGuildId_);
            rs.getFieldValue(1, relationship.guildName_.ref());
            rs.getFieldValue(2, relationship.guildMarkCode_);
            rs.getFieldValue(3, relationship.relationship_);
            rs.getFieldValue(4, relationship.relationshipTime_);

            relationInfos.push_back(relationship);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::getGuildApplicants(GuildId guildId, GuildApplicantInfoMap& guildApplicantInfoMap)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        guildApplicantsCommand_.paramGuildId_.setValue(guildId);

        guildApplicantsCommand_.execute();

        GuildApplicantInfo applicantInfo;
        sne::database::AdoRecordSet& rs = guildApplicantsCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId playerId = invalidObjectId;
            rs.getFieldValue(0, playerId);
            rs.getFieldValue(1, applicantInfo.nickname_.ref());
            rs.getFieldValue(2, applicantInfo.characterClass_);
            guildApplicantInfoMap.emplace(playerId, applicantInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::createGuild(const BaseGuildInfo& guildInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    beginTransaction();

    bool isSucceeded = true;
    try {
        createGuildCommand_.paramId_.setValue(guildInfo.guildId_);
        createGuildCommand_.paramGuildName_.setValue(guildInfo.guildName_.ref());
        createGuildCommand_.paramGuildMarkCode_.setValue(guildInfo.guildMarkCode_);

        createGuildCommand_.execute();

        GuildRankInfos infos;
        for (const GuildRankInfo& info : fillDefaultGuildRanks(infos)) {
            if (! addGuildRank(guildInfo.guildId_, info, 0)) {
                isSucceeded = false;
                break;
            }
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        isSucceeded = false;
    }

    if (isSucceeded) {
        commitTransaction();
    }
    else {
        rollbackTransaction();
    }
    return isSucceeded;
}


bool SqlServerGameDatabase::addGuildMember(GuildId guildId, const GuildMemberInfo& guildMemberInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildMemberCommand_.paramCharacterId_.setValue(guildMemberInfo.playerId_);
        addGuildMemberCommand_.paramGuildId_.setValue(guildId);
        addGuildMemberCommand_.paramPosition_.setValue(guildMemberInfo.position_);
        addGuildMemberCommand_.paramRankId_.setValue(guildMemberInfo.rankId_);
        addGuildMemberCommand_.paramContributiveness_.setValue(guildMemberInfo.contributiveness_);

        addGuildMemberCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeGuildMember(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeGuildMemberCommand_.paramCharacterId_.setValue(characterId);

        removeGuildMemberCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeGuild(GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeGuildCommand_.paramGuildId_.setValue(guildId);

        removeGuildCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId, GuildRelatioshipType type)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildRelationshipCommand_.paramOwnerGuildId_.setValue(ownerGuildId);
        addGuildRelationshipCommand_.paramTargetGuildId_.setValue(targetGuildId);
        addGuildRelationshipCommand_.paramRelationshipType_.setValue(type);
        addGuildRelationshipCommand_.paramRelationshipAt_.setValue(getTime());

        addGuildRelationshipCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeGuildRelationship(GuildId ownerGuildId, GuildId targetGuildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeGuildRelationshipCommand_.paramOwnerGuildId_.setValue(ownerGuildId);
        removeGuildRelationshipCommand_.paramTargetGuildId_.setValue(targetGuildId);

        removeGuildRelationshipCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::changeGuildMemberPosition(ObjectId characterId, GuildMemberPosition position)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        changeGuildMemberPositionCommand_.paramCharacterId_.setValue(characterId);
        changeGuildMemberPositionCommand_.paramPosition_.setValue(position);

        changeGuildMemberPositionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildApplicant(ObjectId characterId, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildApplicantCommand_.paramCharacterId_.setValue(characterId);
        addGuildApplicantCommand_.paramGuildId_.setValue(guildId);

        addGuildApplicantCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeGuildApplicant(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeGuildApplicantCommand_.paramCharacterId_.setValue(characterId);
        
        removeGuildApplicantCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::modifyGuildIntroduction(GuildId guildId, const GuildIntroduction& introduction)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        modifyGuildIntroductionCommand_.paramGuildId_.setValue(guildId);
        modifyGuildIntroductionCommand_.paramIntroduction_.setValue(introduction);
        
        modifyGuildIntroductionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::modifyGuildNotice(GuildId guildId, const GuildNotice& notice)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        modifyGuildNoticeCommand_.paramGuildId_.setValue(guildId);
        modifyGuildNoticeCommand_.paramNotice_.setValue(notice);

        modifyGuildNoticeCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildRank(GuildId guildId, const GuildRankInfo& rankInfo, uint8_t vaultCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    beginTransaction();

    bool isSucceeded = true;

    try {
        addGuildRankCommand_.paramGuildId_.setValue(guildId);
        addGuildRankCommand_.paramRankId_.setValue(rankInfo.rankId_);
        addGuildRankCommand_.paramName_.setValue(rankInfo.name_);
        addGuildRankCommand_.paramRights_.setValue(rankInfo.rights_);
        addGuildRankCommand_.paramGoldWithdrawalPerDay_.setValue(rankInfo.goldWithdrawalPerDay_);

        addGuildRankCommand_.execute();

        if (0 < vaultCount) {
            for (VaultId id = 0; id < vaultCount; ++id) {
                if (! addGuildBankRights(guildId, rankInfo.rankId_, id, GuildBankVaultRightInfo())) {
                    isSucceeded = false;
                    break;;
                }
            }
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        isSucceeded = false;        
    }

    if (isSucceeded) {
        commitTransaction();
    }
    else {
        rollbackTransaction();
    }

    return isSucceeded;
}


bool SqlServerGameDatabase::addGuildBankRights(GuildId guildId, GuildRankId rankId, VaultId vaultId, const GuildBankVaultRightInfo& bankRankInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildBankRightsCommand_.paramGuildId_.setValue(guildId);
        addGuildBankRightsCommand_.paramRankId_.setValue(rankId);
        addGuildBankRightsCommand_.paramVaultId_.setValue(vaultId);
        addGuildBankRightsCommand_.paramRights_.setValue(bankRankInfo.rights_);
        addGuildBankRightsCommand_.paramGoldWithdrawalPerDay_.setValue(bankRankInfo.itemWithdrawalPerDay_);

        addGuildBankRightsCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::deleteGuildRank(GuildId guildId, GuildRankId rankId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        deleteGuildRankCommand_.paramGuildId_.setValue(guildId);
        deleteGuildRankCommand_.paramRankId_.setValue(rankId);

        deleteGuildRankCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::swapGuildRank(GuildId guildId, GuildRankId rankId1, GuildRankId rankId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        swapGuildRankCommand_.paramGuildId_.setValue(guildId);
        swapGuildRankCommand_.paramRankId1_.setValue(rankId1);
        swapGuildRankCommand_.paramRankId2_.setValue(rankId2);

        swapGuildRankCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildRankName(GuildId guildId,
    GuildRankId rankId, const GuildRankName& rankName)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildRankNameCommand_.paramGuildId_.setValue(guildId);
        updateGuildRankNameCommand_.paramRankId_.setValue(rankId);
        updateGuildRankNameCommand_.paramRankName_.setValue(rankName);

        updateGuildRankNameCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildRankRights(GuildId guildId, GuildRankId rankId,
    uint32_t rights, uint32_t goldWithdrawalPerDay)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildRankRightsCommand_.paramGuildId_.setValue(guildId);
        updateGuildRankRightsCommand_.paramRankId_.setValue(rankId);
        updateGuildRankRightsCommand_.paramRights_.setValue(rights);
        updateGuildRankRightsCommand_.paramGoldWithdrawalPerDay_.setValue(goldWithdrawalPerDay);

        updateGuildRankRightsCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildBankRights(GuildId guildId, GuildRankId rankId, VaultId vaultId, uint32_t rights, uint32_t itemWithdrawalPerDay)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildBankRightsCommand_.paramGuildId_.setValue(guildId);
        updateGuildBankRightsCommand_.paramRankId_.setValue(rankId);
        updateGuildBankRightsCommand_.paramVaultId_.setValue(vaultId);
        updateGuildBankRightsCommand_.paramRights_.setValue(rights);
        updateGuildBankRightsCommand_.paramGoldWithdrawalPerDay_.setValue(itemWithdrawalPerDay);

        updateGuildBankRightsCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildRank(ObjectId playerId, GuildRankId rankId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildRankCommand_.paramPlayerId_.setValue(playerId);
        updateGuildRankCommand_.paramRankId_.setValue(rankId);

        updateGuildRankCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildExp(GuildId guildId, GuildExp exp, GuildSkillPoint currentSkillPoint, 
    GuildExp dayAddExp, sec_t lastUpdateTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildExpCommand_.paramGuildId_.setValue(guildId);
        updateGuildExpCommand_.paramExp_.setValue(exp);
        updateGuildExpCommand_.paramDayAddExp_.setValue(dayAddExp);
        updateGuildExpCommand_.paramSp_.setValue(currentSkillPoint);
        updateGuildExpCommand_.paramLastUpdateTime_.setValue(lastUpdateTime);

        updateGuildExpCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildSkill(GuildId guildId, SkillCode guildSkillCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildSkillCommand_.paramGuildId_.setValue(guildId);
        addGuildSkillCommand_.paramSkillCode_.setValue(guildSkillCode);

        addGuildSkillCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeAllGuildSkills(GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeAllGuildSkillsCommand_.paramGuildId_.setValue(guildId);

        removeAllGuildSkillsCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeGuildSkill(GuildId guildId, SkillCode guildSkillCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeGuildSkillCommand_.paramGuildId_.setValue(guildId);
        removeGuildSkillCommand_.paramSkillCode_.setValue(guildSkillCode);

        removeGuildSkillCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::moveGuildInventoryItem(ObjectId itemId, VaultId vaultId, SlotId slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        moveGuildItemCommand_.paramItemId_.setValue(itemId);
        moveGuildItemCommand_.paramVaultId_.setValue(vaultId);
        moveGuildItemCommand_.paramSlotId_.setValue(slotId);

        moveGuildItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::switchGuildInventoryItem(ObjectId itemId1, VaultId vaultId1, SlotId slotId1,
    ObjectId itemId2, VaultId vaultId2, SlotId slotId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        switchGuildItemCommand_.paramItemId1_.setValue(itemId1);
        switchGuildItemCommand_.paramVaultId1_.setValue(vaultId1);
        switchGuildItemCommand_.paramSlotId1_.setValue(slotId1);
        switchGuildItemCommand_.paramItemId2_.setValue(itemId2);
        switchGuildItemCommand_.paramVaultId2_.setValue(vaultId2);
        switchGuildItemCommand_.paramSlotId2_.setValue(slotId2);

        switchGuildItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildInventoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addGuildItemCommand_.paramGuildId_.setValue(guildId);
        addGuildItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addGuildItemCommand_.paramVaultId_.setValue(vaultId);
        addGuildItemCommand_.paramSlotId1_.setValue(itemInfo.slotId_);
        addGuildItemCommand_.paramItemCount_.setValue(uint8_t(itemInfo.count_));
        addGuildItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildInventoryEquipItem(GuildId guildId, 
    VaultId vaultId, const ItemInfo& itemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildEquipItemCommand_.paramGuildId_.setValue(guildId);
        addGuildEquipItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addGuildEquipItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addGuildEquipItemCommand_.paramSlotId_.setValue(itemInfo.slotId_);
        addGuildEquipItemCommand_.paramItemCount_.setValue(uint8_t(itemInfo.count_));
        addGuildEquipItemCommand_.paramVaultId_.setValue(vaultId);

        setOptionValue(addGuildEquipItemCommand_, itemInfo.equipItemInfo_.addOptions_);
        addGuildEquipItemCommand_.paramSocketCount_.setValue(itemInfo.equipItemInfo_.socketCount_);
        addGuildEquipItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildEquipSocketOption(GuildId guildId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildEquipSocketOptionCommand_.paramGuildId_.setValue(guildId);
        addGuildEquipSocketOptionCommand_.paramItemId_.setValue(itemId);
        addGuildEquipSocketOptionCommand_.paramSlotId_.setValue(slotId);
        addGuildEquipSocketOptionCommand_.paramGemCode_.setValue(socketInfo.gemCode_);
        setOptionValue(addGuildEquipSocketOptionCommand_, socketInfo.scriptInfos_);
        addGuildEquipSocketOptionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}

bool SqlServerGameDatabase::addGuildInventoryAccessoryItem(GuildId guildId, VaultId vaultId, const ItemInfo& itemInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildAccessoryItemCommand_.paramGuildId_.setValue(guildId);
        addGuildAccessoryItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addGuildAccessoryItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addGuildAccessoryItemCommand_.paramSlotId_.setValue(itemInfo.slotId_);
        addGuildAccessoryItemCommand_.paramItemCount_.setValue(uint8_t(itemInfo.count_));
        addGuildAccessoryItemCommand_.paramVaultId_.setValue(vaultId);

        setOptionValue(addGuildAccessoryItemCommand_, itemInfo.accessoryItemInfo_.addOptions_);

        addGuildAccessoryItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildInventoryItemCount(ObjectId itemId,
    uint8_t itemCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildItemCountCommand_.paramItemId_.setValue(itemId);
        updateGuildItemCountCommand_.paramItemCount_.setValue(itemCount);

        updateGuildItemCountCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildGameMoney(GuildId guildId, GameMoney gameMoney)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildGameMoney_.paramGuildId_.setValue(guildId);
        updateGuildGameMoney_.paramGameMoney_.setValue(gameMoney);

        updateGuildGameMoney_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildMemberWithdraw(ObjectId playerId, GameMoney dayWithdraw, sec_t resetTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildMemberWithdrawCommand_.paramPlayerId_.setValue(playerId);
        updateGuildMemberWithdrawCommand_.paramWithdraw_.setValue(dayWithdraw);
        updateGuildMemberWithdrawCommand_.paramResetWithdrawTime_.setValue(resetTime);
        updateGuildMemberWithdrawCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGuildVaultName(GuildId guildId, VaultId vaultId, const VaultName& name)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGuildVaultNameCommand_.paramId_.setValue(vaultId);
        updateGuildVaultNameCommand_.paramGuildId_.setValue(guildId);
        updateGuildVaultNameCommand_.paramName_.setValue(name.ref());
        updateGuildVaultNameCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addGuildVault(GuildId guildId, const VaultInfo& vaultInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addGuildVaultCommand_.paramId_.setValue(vaultInfo.id_);
        addGuildVaultCommand_.paramGuildId_.setValue(guildId);
        addGuildVaultCommand_.paramName_.setValue(vaultInfo.name_.ref());
        addGuildVaultCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeGuildInventoryItem(ObjectId itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeGuildItemCommand_.paramItemId_.setValue(itemId);
        
        removeGuildItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getGuildVaults(GuildInfo& guildInfo, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildVaultsCommand_.paramGuildId_.setValue(guildId);

        getGuildVaultsCommand_.execute();
        
        sne::database::AdoRecordSet& rs = getGuildVaultsCommand_.adoRecordSet_;
        uint8_t vaultCount = 1;
        rs.getFieldValue(0, vaultCount);

        rs.nextRecordset();

        BaseVaultInfo vaultInfo;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, vaultInfo.id_);
            rs.getFieldValue(1, vaultInfo.name_.ref());
            assert(vaultInfo.id_ == guildInfo.inventoryInfo_.getNextVaultId());
            guildInfo.inventoryInfo_.addVault(vaultInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getGuildInventory(GuildInventoryInfo& inventoryInfo, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildInventory_.paramGuildId_.setValue(guildId);

        getGuildInventory_.execute();

        sne::database::AdoRecordSet& rs = getGuildInventory_.adoRecordSet_;

        for (; ! rs.isEof(); rs.moveNext()) {
            VaultId vaultId = 0;
            rs.getFieldValue(0, vaultId);
            ItemInfo itemInfo;
            rs.getFieldValue(1, itemInfo.itemId_);
            rs.getFieldValue(2, itemInfo.itemCode_);
            rs.getFieldValue(3, itemInfo.count_);
            rs.getFieldValue(4, itemInfo.slotId_);

            inventoryInfo.addItem(vaultId, itemInfo);                 
         }

        rs.nextRecordset();
        // equip
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId itemId = invalidObjectId;
            VaultId vaultId = 0;
            uint16_t index = 0;
            uint8_t socketCount = 0;
            rs.getFieldValue(index++, vaultId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, socketCount);

            ItemInfo* itemInfo = inventoryInfo.getItemInfo(vaultId, itemId);
            if (! itemInfo) {
                return false;
            }

            itemInfo->equipItemInfo_.socketCount_ = socketCount;
            fetchItemScriptInfo(itemInfo->equipItemInfo_.addOptions_, rs, index);
        }
        // socket
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId itemId = invalidObjectId;
            VaultId vaultId = 0;
            uint16_t index = 0;
            SocketSlotId slotId = invalidSocketSlotId;
            rs.getFieldValue(index++, vaultId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, slotId);
            ItemInfo* itemInfo = inventoryInfo.getItemInfo(vaultId, itemId);
            if (! itemInfo) {
                return false;
            }
            EquipSocketInfo socketInfo;
            rs.getFieldValue(index++, socketInfo.gemCode_);
            fetchItemScriptInfo(socketInfo.scriptInfos_, rs, index);
            itemInfo->equipItemInfo_.addGemItem(slotId, socketInfo);
        }

        // accessory
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId itemId = invalidObjectId;
            VaultId vaultId = 0;
            uint16_t index = 0;
            rs.getFieldValue(index++, vaultId);
            rs.getFieldValue(index++, itemId);
            ItemInfo* itemInfo = inventoryInfo.getItemInfo(vaultId, itemId);
            if (! itemInfo) {
                return false;
            }

            fetchItemScriptInfo(itemInfo->accessoryItemInfo_.addOptions_, rs, index);
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getGuildEventLog(GuildEventLogManager& logManager, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildEventLogCommand_.paramGuildId_.setValue(guildId);
        getGuildEventLogCommand_.execute();
        sne::database::AdoRecordSet& rs = getGuildEventLogCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            GuildEventLogInfo logInfo;
            rs.getFieldValue(0, logInfo.id_);
            rs.getFieldValue(1, logInfo.logType_);
            rs.getFieldValue(2, logInfo.playerId1_);
            rs.getFieldValue(3, logInfo.nickname1_.ref());
            rs.getFieldValue(4, logInfo.playerId2_);
            rs.getFieldValue(5, logInfo.nickname2_.ref());
            rs.getFieldValue(6, logInfo.param1_);
            rs.getFieldValue(7, logInfo.logTime_); 
            logManager.addLog(logInfo);
            logManager.logId_ = logInfo.id_;
        }
    }

    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}

bool SqlServerGameDatabase::getGuildGameMoneyEventLog(GuildGameMoneyEventLogManager& logManager, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildGameMoneyEventLogCommand_.paramGuildId_.setValue(guildId);
        getGuildGameMoneyEventLogCommand_.execute();
        sne::database::AdoRecordSet& rs = getGuildGameMoneyEventLogCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            GuildGameMoneyEventLogInfo logInfo;
            rs.getFieldValue(0, logInfo.id_);
            rs.getFieldValue(1, logInfo.logType_);
            rs.getFieldValue(2, logInfo.playerId_);
            rs.getFieldValue(3, logInfo.nickname_.ref());
            rs.getFieldValue(4, logInfo.gameMoney_);
            rs.getFieldValue(5, logInfo.logTime_); 

            logManager.addLog(logInfo);
            logManager.logId_ = logInfo.id_;
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getGuildBankEventLog(GuildBankEventLogManager& logManager, GuildId guildId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getGuildBankEventLogCommand_.paramGuildId_.setValue(guildId);
        getGuildBankEventLogCommand_.execute();
        sne::database::AdoRecordSet& rs = getGuildBankEventLogCommand_.adoRecordSet_;
                
        for (; ! rs.isEof(); rs.moveNext()) {
            GuildBankEventLogInfo logInfo;
            rs.getFieldValue(0, logInfo.id_);
            rs.getFieldValue(1, logInfo.vaultId_);
            rs.getFieldValue(2, logInfo.logType_);
            rs.getFieldValue(3, logInfo.playerId_);
            rs.getFieldValue(4, logInfo.nickname_.ref());
            rs.getFieldValue(5, logInfo.param1_);
            rs.getFieldValue(6, logInfo.param2_);
            rs.getFieldValue(7, logInfo.logTime_); 
            logManager.logId_ = logInfo.id_;
            logManager.addLog(logInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getMaxGuildId(GuildId& itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    itemId = invalidObjectId;

    try {
        getMaxGuildIdCommand_.execute();

        getMaxGuildIdCommand_.paramItemId_.getValue(itemId);
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


ObjectId SqlServerGameDatabase::getPlayerId(const Nickname& nickname)
{
    ObjectId playerId = invalidObjectId;

    try {
        getPlayerIdCommand_.paramNickname_.setValue(nickname);
        getPlayerIdCommand_.execute();

        getPlayerIdCommand_.paramPlayerId_.getValue(playerId);
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return playerId;
}


bool SqlServerGameDatabase::getMails(MailInfos& mails, ObjectId playerId)
{
    mails, playerId;
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(playerId)) {
        return false;
    }

    try {
        getMailsCommand_.paramCharacterId_.setValue(playerId);

        getMailsCommand_.execute();

        sne::database::AdoRecordSet& rs = getMailsCommand_.adoRecordSet_;

        FullCharacterInfo characterInfo;				
        for (; ! rs.isEof(); rs.moveNext()) {
            MailInfo mailInfo;
            rs.getFieldValue(0, mailInfo.titleInfo_.mailId_);
            rs.getFieldValue(1, mailInfo.titleInfo_.nickname_.ref());
            rs.getFieldValue(2, mailInfo.titleInfo_.objectId_);
            rs.getFieldValue(3, mailInfo.titleInfo_.dataCode_);
            rs.getFieldValue(4, mailInfo.titleInfo_.title_.ref());
            rs.getFieldValue(5, mailInfo.contentInfo_.mailBody_.ref());
            rs.getFieldValue(6, mailInfo.titleInfo_.time_);
            rs.getFieldValue(7, mailInfo.titleInfo_.isRead_);
            rs.getFieldValue(8, mailInfo.contentInfo_.gameMoney_);
            rs.getFieldValue(9, mailInfo.titleInfo_.mailType_);
            mailInfo.titleInfo_.hasAttachment_ = mailInfo.contentInfo_.gameMoney_ > 0;
            mails.emplace(mailInfo.titleInfo_.mailId_, mailInfo);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ItemInfo itemInfo;
            MailId mailId = invalidMailId;
            rs.getFieldValue(0, mailId);
            rs.getFieldValue(1, itemInfo.itemId_);
            rs.getFieldValue(2, itemInfo.itemCode_);
            rs.getFieldValue(3, itemInfo.count_);

            MailInfos::iterator pos = mails.find(mailId);
            if (pos != mails.end()) {
                MailInfo& mailInfo = (*pos).second;
                mailInfo.titleInfo_.hasAttachment_ = true;
                mailInfo.contentInfo_.mailInventory_.addItem(itemInfo);                
            }
        }

        rs.nextRecordset();

        // equip
        for (; ! rs.isEof(); rs.moveNext()) {
            MailId mailId = invalidMailId;
            ObjectId itemId = invalidObjectId;
            uint16_t index = 0;
            uint8_t socketCount = 0;
        
            rs.getFieldValue(index++, mailId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, socketCount);

            MailInfos::iterator pos = mails.find(mailId);
            if (pos == mails.end()) {
                assert(false);
                continue;
            }
            MailInfo& mailInfo = (*pos).second;                
            ItemInfo* itemInfo = mailInfo.contentInfo_.mailInventory_.getItemInfo(itemId);     
           
            if (! itemInfo) {
                assert(false);
                continue;
            }
            itemInfo->equipItemInfo_.socketCount_ = socketCount;
            fetchItemScriptInfo(itemInfo->equipItemInfo_.addOptions_, rs, index);    
        }

        rs.nextRecordset();

        // socket
        for (; ! rs.isEof(); rs.moveNext()) {
            MailId mailId = invalidMailId;
            ObjectId itemId = invalidObjectId;
            SocketSlotId slotId = invalidSocketSlotId;
            uint16_t index = 0;
            rs.getFieldValue(index++, mailId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, slotId);

            MailInfos::iterator pos = mails.find(mailId);
            if (pos == mails.end()) {
                assert(false);
                continue;
            }

            MailInfo& mailInfo = (*pos).second;                
            ItemInfo* itemInfo = mailInfo.contentInfo_.mailInventory_.getItemInfo(itemId);     

            if (! itemInfo) {
                assert(false);
                continue;
            }

            EquipSocketInfo socketInfo;            
            fetchSocketInfo(socketInfo, rs, index);    
            itemInfo->equipItemInfo_.addGemItem(slotId, socketInfo);
        }

        // assessory
        for (; ! rs.isEof(); rs.moveNext()) {
            MailId mailId = invalidMailId;
            ObjectId itemId = invalidObjectId;
            uint16_t index = 0;
            rs.getFieldValue(index++, mailId);
            rs.getFieldValue(index++, itemId);

            MailInfos::iterator pos = mails.find(mailId);
            if (pos == mails.end()) {
                assert(false);
                continue;
            }

            MailInfo& mailInfo = (*pos).second;  
            ItemInfo* itemInfo = mailInfo.contentInfo_.mailInventory_.getItemInfo(itemId);    
            if (!itemInfo) {
                assert(false);
                continue;
            }

            fetchItemScriptInfo(itemInfo->accessoryItemInfo_.addOptions_, rs, index);    
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::insertMail(MailInfo& mail, ObjectId playerId)
{

    try {
        insertMailCommand_.paramCharacterId_.setValue(playerId);
        insertMailCommand_.paramNickname_.setValue(mail.titleInfo_.nickname_);
        insertMailCommand_.paramSendId_.setValue(mail.titleInfo_.objectId_);
        insertMailCommand_.paramDataCode_.setValue(mail.titleInfo_.dataCode_);
        insertMailCommand_.paramTitle_.setValue(mail.titleInfo_.title_);
        insertMailCommand_.paramBody_.setValue(mail.contentInfo_.mailBody_);
        insertMailCommand_.sentAt_.setValue(mail.titleInfo_.time_);
        insertMailCommand_.paramGameMoney_.setValue(mail.contentInfo_.gameMoney_);
        insertMailCommand_.paramMailType_.setValue(mail.titleInfo_.mailType_);
        insertMailCommand_.execute();
        insertMailCommand_.paramMailId_.getValue(mail.titleInfo_.mailId_);
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addMailItem(ObjectId characterId, MailId mailld,
    const ItemInfo& itemInfo)
{
    try {
        addMailItemCommand_.paramMailId_.setValue(mailld);
        addMailItemCommand_.paramCharacterId_.setValue(characterId);
        addMailItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addMailItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addMailItemCommand_.paramItemCount_.setValue(itemInfo.count_);
        addMailItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addEquipMailItem(ObjectId characterId,
    MailId mailld, const ItemInfo& itemInfo, const EquipItemInfo& equipItemInfo)
{
    try {
        addMailEquipItemCommand_.paramMailId_.setValue(mailld);
        addMailEquipItemCommand_.paramCharacterId_.setValue(characterId);
        addMailEquipItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addMailEquipItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addMailEquipItemCommand_.paramItemCount_.setValue(itemInfo.count_);

        setOptionValue(addMailEquipItemCommand_, equipItemInfo.addOptions_);
        addMailEquipItemCommand_.paramSocketCount_.setValue(equipItemInfo.socketCount_);
        addMailEquipItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addMailEquipSocketOption(ObjectId characterId, MailId mailld, 
    ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo)
{
    try {
        addMailEquipSocketOptionCommand_.paramMailId_.setValue(mailld);
        addMailEquipSocketOptionCommand_.paramCharacterId_.setValue(characterId);
        addMailEquipSocketOptionCommand_.paramItemId_.setValue(itemId);
        addMailEquipSocketOptionCommand_.paramSlotId_.setValue(slotId);
        addMailEquipSocketOptionCommand_.paramGemCode_.setValue(socketInfo.gemCode_);
        setOptionValue(addMailEquipSocketOptionCommand_, socketInfo.scriptInfos_);
        addMailEquipSocketOptionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addAccessoryMailItem(ObjectId characterId, MailId mailld, 
    const ItemInfo& itemInfo, const AccessoryItemInfo& accessoryItemInfo)
{
    try {
        addMailAccessoryItemCommand_.paramMailId_.setValue(mailld);
        addMailAccessoryItemCommand_.paramCharacterId_.setValue(characterId);
        addMailAccessoryItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addMailAccessoryItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addMailAccessoryItemCommand_.paramItemCount_.setValue(itemInfo.count_);

        setOptionValue(addMailAccessoryItemCommand_, accessoryItemInfo.addOptions_);
        addMailAccessoryItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::readMail(MailId mailId)
{
    try {
        readMailCommand_.paramMailId_.setValue(mailId);
        readMailCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::deleteMail(MailId mailId)
{
    try {
        deleteMailCommand_.paramMailId_.setValue(mailId);
        deleteMailCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::deleteMailItems(MailId mailId)
{
    try {
        deleteMailItemCommand_.paramMailId_.setValue(mailId);
        deleteMailItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::hasNotReadMail(ObjectId characterId)
{
    bool result = false;
    try {
        hasNotReadMailCommand_.paramCharacterId_.setValue(characterId);
        hasNotReadMailCommand_.execute();
        hasNotReadMailCommand_.paramHasNotReadMail_.getValue(result);
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return result;
}


bool SqlServerGameDatabase::getAuctions(ServerAuctionInfoMap& auctionMap, ZoneId zoneId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! isValidObjectId(zoneId)) {
        return false;
    }
    typedef sne::core::HashMap<ObjectId, NpcCode> AuctionIdMap;
    AuctionIdMap auctionIdMap;

    try {
        getAuctionsCommand_.paramZoneId_.setValue(zoneId);

        getAuctionsCommand_.execute();

        sne::database::AdoRecordSet& rs = getAuctionsCommand_.adoRecordSet_;

                    
        for (; ! rs.isEof(); rs.moveNext()) {
            NpcCode npcCode = invalidNpcCode;
            FullAuctionInfo auctionInfo;

            rs.getFieldValue(0, auctionInfo.auctionId_);
            rs.getFieldValue(1, npcCode);
            rs.getFieldValue(2, auctionInfo.sellerId_);
            rs.getFieldValue(3, auctionInfo.buyerId_);
            rs.getFieldValue(4, auctionInfo.currentBidMoney_);
            rs.getFieldValue(5, auctionInfo.buyoutMoney_);
            rs.getFieldValue(6, auctionInfo.depositFee_);
            rs.getFieldValue(7, auctionInfo.expireTime_);

            auctionIdMap.emplace(auctionInfo.auctionId_, npcCode);
            ServerAuctionInfoMap::iterator pos = auctionMap.find(npcCode);
            if (pos != auctionMap.end()) {
                FullAuctionInfoMap& infos = (*pos).second;
                infos.emplace(auctionInfo.auctionId_, auctionInfo);
            }
            else {
                FullAuctionInfoMap infos;
                infos.emplace(auctionInfo.auctionId_, auctionInfo);
                auctionMap.emplace(npcCode, infos);
            }
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            AuctionId auctionId = invalidAuctionId;
            rs.getFieldValue(0, auctionId);

            AuctionIdMap::const_iterator idPos = auctionIdMap.find(auctionId);
            if (idPos == auctionIdMap.end()) {
                assert(false);
                // TODO 로그
                continue;
            }
            NpcCode npcCode = (*idPos).second;
            ServerAuctionInfoMap::iterator pos = auctionMap.find(npcCode);
            if (pos != auctionMap.end()) {
                FullAuctionInfoMap& infos = (*pos).second;
                FullAuctionInfoMap::iterator auctionPos = infos.find(auctionId);
                if (auctionPos == infos.end()) {
                    assert(false);
                    // TODO 로그
                    continue;                
                }
                FullAuctionInfo& auctionInfo = (*auctionPos).second;
                rs.getFieldValue(1, auctionInfo.itemInfo_.itemId_);
                rs.getFieldValue(2, auctionInfo.itemInfo_.itemCode_);
                rs.getFieldValue(3, auctionInfo.itemInfo_.count_);
            }            
        }

        rs.nextRecordset();

        // equip
        for (; ! rs.isEof(); rs.moveNext()) {
            AuctionId auctionId = invalidAuctionId;
            uint16_t index = 0;
            uint8_t socketCount = 0;
            rs.getFieldValue(index++, auctionId);
            rs.getFieldValue(index++, socketCount);
            AuctionIdMap::const_iterator idPos = auctionIdMap.find(auctionId);
            if (idPos == auctionIdMap.end()) {
                assert(false);
                // TODO 로그
                continue;
            }
            NpcCode npcCode = (*idPos).second;
            ServerAuctionInfoMap::iterator pos = auctionMap.find(npcCode);
            if (pos != auctionMap.end()) {
                FullAuctionInfoMap& infos = (*pos).second;
                FullAuctionInfoMap::iterator auctionPos = infos.find(auctionId);
                if (auctionPos == infos.end()) {
                    assert(false);
                    // TODO 로그
                    continue;                
                }
                FullAuctionInfo& auctionInfo = (*auctionPos).second;
                fetchItemScriptInfo(auctionInfo.itemInfo_.equipItemInfo_.addOptions_, rs, index);  
            }       
        }

        // socket
        for (; ! rs.isEof(); rs.moveNext()) {
            AuctionId auctionId = invalidAuctionId;
            SocketSlotId slotId = invalidSocketSlotId;
            uint16_t index = 0;
            rs.getFieldValue(index++, auctionId);
            rs.getFieldValue(index++, slotId);

            AuctionIdMap::const_iterator idPos = auctionIdMap.find(auctionId);
            if (idPos == auctionIdMap.end()) {
                assert(false);
                // TODO 로그
                continue;
            }
            NpcCode npcCode = (*idPos).second;
            ServerAuctionInfoMap::iterator pos = auctionMap.find(npcCode);
            if (pos != auctionMap.end()) {
                FullAuctionInfoMap& infos = (*pos).second;
                FullAuctionInfoMap::iterator auctionPos = infos.find(auctionId);
                if (auctionPos == infos.end()) {
                    assert(false);
                    // TODO 로그
                    continue;                
                }
                FullAuctionInfo& auctionInfo = (*auctionPos).second;
                EquipSocketInfo socketInfo;
                fetchSocketInfo(socketInfo, rs, index);  
                auctionInfo.itemInfo_.equipItemInfo_.addGemItem(slotId, socketInfo);
            }       
        }

        // accessory
        for (; ! rs.isEof(); rs.moveNext()) {
            AuctionId auctionId = invalidAuctionId;
            uint16_t index = 0;
            rs.getFieldValue(index++, auctionId);

            AuctionIdMap::const_iterator idPos = auctionIdMap.find(auctionId);
            if (idPos == auctionIdMap.end()) {
                assert(false);
                // TODO 로그
                continue;
            }
            NpcCode npcCode = (*idPos).second;
            ServerAuctionInfoMap::iterator pos = auctionMap.find(npcCode);
            if (pos != auctionMap.end()) {
                FullAuctionInfoMap& infos = (*pos).second;
                FullAuctionInfoMap::iterator auctionPos = infos.find(auctionId);
                if (auctionPos == infos.end()) {
                    assert(false);
                    // TODO 로그
                    continue;                
                }
                FullAuctionInfo& auctionInfo = (*auctionPos).second;
                fetchItemScriptInfo(auctionInfo.itemInfo_.accessoryItemInfo_.addOptions_, rs, index);  
            }       
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::insertAuction(ZoneId zoneId, NpcCode npcCode,
    const FullAuctionInfo& auctionInfo)
{
    try {
        insertAuctionCommand_.paramAuctionId_.setValue(auctionInfo.auctionId_);
        insertAuctionCommand_.paramZoneId_.setValue(zoneId);
        insertAuctionCommand_.paramNpcCode_.setValue(npcCode);
        insertAuctionCommand_.paramSellerId_.setValue(auctionInfo.sellerId_);
        insertAuctionCommand_.paramBidMoney_.setValue(auctionInfo.currentBidMoney_);
        insertAuctionCommand_.paramBuyoutMoney_.setValue(auctionInfo.buyoutMoney_);
        insertAuctionCommand_.paramDepositFee_.setValue(auctionInfo.depositFee_);
        insertAuctionCommand_.expireAt_.setValue(auctionInfo.expireTime_);
        insertAuctionCommand_.paramItemId_.setValue(auctionInfo.itemInfo_.itemId_);
        insertAuctionCommand_.paramItemCode_.setValue(auctionInfo.itemInfo_.itemCode_);
        insertAuctionCommand_.paramItemCount_.setValue(auctionInfo.itemInfo_.count_);
        insertAuctionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addAuctionEquipInfo(ZoneId zoneId, AuctionId auctionId,
    const EquipItemInfo& equipItemInfo)
{
    try {
        addAuctionEquipInfoCommand_.paramAuctionId_.setValue(auctionId);
        addAuctionEquipInfoCommand_.paramZoneId_.setValue(zoneId);
        setOptionValue(addAuctionEquipInfoCommand_, equipItemInfo.addOptions_);
        addAuctionEquipInfoCommand_.paramSocketCount_.setValue(equipItemInfo.socketCount_);
        addAuctionEquipInfoCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addAuctionEquipSocketOption(ZoneId zoneId, AuctionId auctionId, SocketSlotId slotId, const EquipSocketInfo& socketInfo)
{
    try {
        addAuctionEquipSocketOptionCommand_.paramAuctionId_.setValue(auctionId);
        addAuctionEquipSocketOptionCommand_.paramZoneId_.setValue(zoneId);
        addAuctionEquipSocketOptionCommand_.paramSlotId_.setValue(slotId);
        addAuctionEquipSocketOptionCommand_.paramGemCode_.setValue(socketInfo.gemCode_);
        setOptionValue(addAuctionEquipSocketOptionCommand_, socketInfo.scriptInfos_);
        addAuctionEquipSocketOptionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addAuctionAccessoryInfo(ZoneId zoneId, AuctionId auctionId, const AccessoryItemInfo& accessoryItemInfo)
{
    try {
        addAuctionAccessoryInfoCommand_.paramAuctionId_.setValue(auctionId);
        addAuctionAccessoryInfoCommand_.paramZoneId_.setValue(zoneId);
        setOptionValue(addAuctionAccessoryInfoCommand_, accessoryItemInfo.addOptions_);
        addAuctionAccessoryInfoCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateBid(ZoneId zoneId, AuctionId auctionId,
    ObjectId buyerId, GameMoney currentBidMoney)
{
    try {
        updateBidCommand_.paramAuctionId_.setValue(auctionId);
        updateBidCommand_.paramZoneId_.setValue(zoneId);
        updateBidCommand_.paramBuyerId_.setValue(buyerId);
        updateBidCommand_.paramBidMoney_.setValue(currentBidMoney);

        updateBidCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::deleteAuction(ZoneId zoneId, AuctionId auctionId)
{
    try {
        deleteAuctionCommand_.paramAuctionId_.setValue(auctionId);
        deleteAuctionCommand_.paramZoneId_.setValue(zoneId);

        deleteAuctionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::getCharacterArenaRecords(CharacterArenaPlayResults& results, ObjectId characterId)
{
    try {
        getPlayerArenaRecordsCommand_.paramCharacterId_.setValue(characterId);

        getPlayerArenaRecordsCommand_.execute();

        sne::database::AdoRecordSet& rs = getPlayerArenaRecordsCommand_.adoRecordSet_;

        for (; ! rs.isEof(); rs.moveNext()) {
            ArenaPlayResult result;
            ArenaModeType modeType = amtUnknown;

            rs.getFieldValue(0, modeType);
            rs.getFieldValue(1, result.rank_);
            rs.getFieldValue(2, result.score_);
            rs.getFieldValue(3, result.winCount_);
            rs.getFieldValue(4, result.loseCount_);
            rs.getFieldValue(5, result.drawCount_);
            if (isValid(modeType)) {
                results.insertArena(modeType, result);
            }
            else {                
                return false;
            }
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::updateArenaRecord(ObjectId characterId,
    ArenaModeType modeType, const ArenaPlayResult& result)
{
    try {
        updateArenaRecordCommand_.paramCharacterId_.setValue(characterId);
        updateArenaRecordCommand_.paramMode_.setValue(modeType);
        updateArenaRecordCommand_.paramScore_.setValue(result.score_);
        updateArenaRecordCommand_.paramWin_.setValue(result.winCount_);
        updateArenaRecordCommand_.paramLose_.setValue(result.loseCount_);
        updateArenaRecordCommand_.paramDraw_.setValue(result.drawCount_);

        updateArenaRecordCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::updateArenaPoint(ObjectId characterId, ArenaPoint arenaPoint)
{
    try {
        updateArenaPointCommand_.paramCharacterId_.setValue(characterId);
        updateArenaPointCommand_.paramArenaPoint_.setValue(arenaPoint);

        updateArenaPointCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::updateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime)
{
    try {
        updateDeserterExpireTimeCommand_.paramCharacterId_.setValue(characterId);
        updateDeserterExpireTimeCommand_.paramDeserterExpireTime_.setValue(deserterExpireTime);

        updateDeserterExpireTimeCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::createBuilding(const BuildingInfo& buildInfo)
{
    try {
        createBuildingCommand_.paramBuildingId_.setValue(buildInfo.objectId_);
        createBuildingCommand_.paramBuildingCode_.setValue(buildInfo.buildingCode_);    
        createBuildingCommand_.paramPosX_.setValue(buildInfo.position_.x_);
        createBuildingCommand_.paramPosY_.setValue(buildInfo.position_.y_);
        createBuildingCommand_.paramPosZ_.setValue(buildInfo.position_.z_);
        createBuildingCommand_.paramPosHeading_.setValue(buildInfo.position_.heading_);
        createBuildingCommand_.paramCurrentHp_.setValue(buildInfo.points_.hp_);
        createBuildingCommand_.paramState_.setValue(buildInfo.state_);
        createBuildingCommand_.paramBuildTime_.setValue(buildInfo.startBuildTime_);
        createBuildingCommand_.paramExpireTime_.setValue(buildInfo.fundationExpireTime_);
        createBuildingCommand_.paramOwnerType_.setValue(buildInfo.ownerInfo_.ownerType_);
        createBuildingCommand_.paramOwnerCharacterId_.setValue(buildInfo.ownerInfo_.playerOwnerInfo_.objectId_);
        createBuildingCommand_.paramOwnerGuildId_.setValue(buildInfo.ownerInfo_.guildInfo_.guildId_);
        createBuildingCommand_.paramCreateTime_.setValue(buildInfo.createTime_);
        createBuildingCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::getBuildingInfos(ObjectIdSet& deleteItems, 
    ObjectIdSet& deleteBuilds, ObjectId& maxBuildingId,
    BuildingInfoMap& buildingInfoMap, ZoneId zoneId)
{
    const ObjectId minId = getMinZoneObjectId(zoneId);
    const ObjectId maxId = getMaxZoneObjectId(zoneId);
    
    try {
        getBuildingsCommand_.paramMinId_.setValue(minId);
        getBuildingsCommand_.paramMaxId_.setValue(maxId);

        getBuildingsCommand_.execute();

        sne::database::AdoRecordSet& rs = getBuildingsCommand_.adoRecordSet_;
        getBuildingsCommand_.paramZoneMaxBuildingId_.getValue(maxBuildingId);
        const sec_t now = getTime() + 30; // 근방 지워질거 같은거는 그냥 삭재함(여유초 30)

        for (; ! rs.isEof(); rs.moveNext()) {            
            BuildingInfo buildingInfo;
            rs.getFieldValue(0, buildingInfo.objectId_);
            rs.getFieldValue(1, buildingInfo.buildingCode_);
            if (BUILDING_TABLE) {
                const datatable::BuildingTemplate* buildingTemplate = BUILDING_TABLE->getBuildingTemplate(buildingInfo.buildingCode_);
                if (buildingTemplate) {
                    buildingInfo.buildingInventory_.defaultInvenCount_ = buildingTemplate->getBuildingInventoryCount();
                    buildingInfo.mercenaryInventory_.defaultInvenCount_ = buildingTemplate->getMercenaryInventoryCount();
                    buildingInfo.productionInventory_.defaultInvenCount_ = buildingTemplate->getProductionInventoryCount();
                    buildingInfo.selectProductionInventory_.defaultInvenCount_ = buildingTemplate->getSelectProductionInventoryCount();
                    buildingInfo.fundationInventory_.defaultInvenCount_ = maxFundationBuildingSlotId;
                }
            }
            
            rs.getFieldValue(2, buildingInfo.position_.x_);
            rs.getFieldValue(3, buildingInfo.position_.y_);
            rs.getFieldValue(4, buildingInfo.position_.z_);
            rs.getFieldValue(5, buildingInfo.position_.heading_);
            rs.getFieldValue(6, buildingInfo.points_.hp_);
            rs.getFieldValue(7, buildingInfo.state_);
            rs.getFieldValue(8, buildingInfo.startBuildTime_);
            rs.getFieldValue(9, buildingInfo.fundationExpireTime_);
            if (now > buildingInfo.fundationExpireTime_ && buildingInfo.fundationExpireTime_ != 0) {
                deleteBuilds.insert(buildingInfo.objectId_);
                continue;
            }
            rs.getFieldValue(10, buildingInfo.ownerInfo_.ownerType_);
            rs.getFieldValue(11, buildingInfo.ownerInfo_.playerOwnerInfo_.objectId_);
            rs.getFieldValue(12, buildingInfo.ownerInfo_.guildInfo_.guildId_);
            rs.getFieldValue(13, buildingInfo.ownerInfo_.playerOwnerInfo_.nickname_.ref());
            rs.getFieldValue(14, buildingInfo.ownerInfo_.guildInfo_.guildName_.ref());
            rs.getFieldValue(15, buildingInfo.ownerInfo_.guildInfo_.guildMarkCode_);
            rs.getFieldValue(16, buildingInfo.createTime_);

            buildingInfoMap.emplace(buildingInfo.objectId_, buildingInfo);
        }

        rs.nextRecordset();

        // Inventory
        ItemInfo item;
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId buildingId = invalidObjectId;
            InvenType invenType;
            rs.getFieldValue(0, buildingId);
            rs.getFieldValue(1, item.itemId_);
            rs.getFieldValue(2, item.itemCode_);
            rs.getFieldValue(3, item.slotId_);
            rs.getFieldValue(4, item.count_);
            rs.getFieldValue(5, invenType);
            rs.getFieldValue(6, item.expireTime_);

            BuildingInfoMap::iterator pos = buildingInfoMap.find(buildingId);
            if (pos == buildingInfoMap.end()) {
                if (deleteBuilds.find(buildingId) != deleteBuilds.end()) {
                    continue;
                }
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find building(%" PRIu64 ")", buildingId);
                return false;
            }
            
            if (now > item.expireTime_ && item.expireTime_ != 0 && ! item.isBuildingGuard()) {
                deleteItems.insert(item.itemId_);
                continue;
            }

            BuildingInfo& buildingInfo = (*pos).second;
            buildingInfo.addItemInfo(invenType, item);            
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId buildingId = invalidObjectId;
            ObjectId itemId = invalidObjectId;
            uint16_t index = 0;
            uint8_t socketCount = 0;
            rs.getFieldValue(index++, buildingId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, socketCount);
            
            if (deleteItems.find(itemId) != deleteItems.end()) {
                continue;
            }

            BuildingInfoMap::iterator pos = buildingInfoMap.find(buildingId);
            if (pos == buildingInfoMap.end()) {
                if (deleteBuilds.find(buildingId) != deleteBuilds.end()) {
                    continue;
                }
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find building(%" PRIu64 ")", buildingId);
                return false;
            }
            // TODO skill
            BuildingInfo& buildingInfo = (*pos).second;
            ItemInfo* itemInfo = buildingInfo.getItemInfo(itemId);
            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(%" PRIu64 ")", buildingId, itemId);
                return false;
            }
            itemInfo->equipItemInfo_.socketCount_ = socketCount;
            fetchItemScriptInfo(itemInfo->equipItemInfo_.addOptions_, rs, index);
        }

        rs.nextRecordset();
        
        // equip socket
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId buildingId = invalidObjectId;
            ObjectId itemId = invalidObjectId;
            SocketSlotId socketId = 0;
            uint16_t index = 0;
            rs.getFieldValue(index++, buildingId);
            rs.getFieldValue(index++, itemId);
            rs.getFieldValue(index++, socketId);

            if (deleteItems.find(itemId) != deleteItems.end()) {
                continue;
            }

            BuildingInfoMap::iterator pos = buildingInfoMap.find(buildingId);
            if (pos == buildingInfoMap.end()) {
                if (deleteBuilds.find(buildingId) != deleteBuilds.end()) {
                    continue;
                }
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find building(%" PRIu64 ")", buildingId);
                return false;
            }

            BuildingInfo& buildingInfo = (*pos).second;
            ItemInfo* itemInfo = buildingInfo.getItemInfo(itemId);
            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(B%" PRIu64 " I:%" PRIu64 ")",
                    buildingId, itemId);
                return false;
            }

            EquipSocketInfo socketInfo;
            fetchSocketInfo(socketInfo, rs, index);
            itemInfo->equipItemInfo_.addGemItem(socketId, socketInfo);
        }
                
        rs.nextRecordset();
        // accessory socket
        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId buildingId = invalidObjectId;
            ObjectId itemId;
            uint16_t index = 0;
            rs.getFieldValue(index++, buildingId);
            rs.getFieldValue(index++, itemId);

            if (deleteItems.find(itemId) != deleteItems.end()) {
                continue;
            }

            BuildingInfoMap::iterator pos = buildingInfoMap.find(buildingId);
            if (pos == buildingInfoMap.end()) {
                if (deleteBuilds.find(buildingId) != deleteBuilds.end()) {
                    continue;
                }
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find building(%" PRIu64 ")", buildingId);
                return false;
            }

            BuildingInfo& buildingInfo = (*pos).second;
            ItemInfo* itemInfo = buildingInfo.getItemInfo(itemId);

            if (! itemInfo) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find equip item Info(B%" PRIu64 ", I%" PRIu64 ")",
                    buildingId, itemId);
                return false;
            }

            fetchItemScriptInfo(itemInfo->accessoryItemInfo_.addOptions_, rs, index);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId buildingId = invalidObjectId;
            BuildingSelectProductionInfo info;
            rs.getFieldValue(0, buildingId);
            rs.getFieldValue(1, info.recipeCode_);
            rs.getFieldValue(2, info.completeCount_);
            rs.getFieldValue(3, info.completeTime_);

            BuildingInfoMap::iterator pos = buildingInfoMap.find(buildingId);
            if (pos == buildingInfoMap.end()) {
                if (deleteBuilds.find(buildingId) != deleteBuilds.end()) {
                    continue;
                }
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find building(%" PRIu64 ")", buildingId);
                return false;
            }
            BuildingInfo& buildingInfo = (*pos).second;
            
            buildingInfo.selectProducionInfos_.push_back(info);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {
            ObjectId buildingId = invalidObjectId;
            BuildingGuardInfo info;
            rs.getFieldValue(0, buildingId);
            rs.getFieldValue(1, info.objectId_);
            rs.getFieldValue(2, info.npcCode_);
            rs.getFieldValue(3, info.position_.x_);
            rs.getFieldValue(4, info.position_.y_);
            rs.getFieldValue(5, info.position_.z_);
            rs.getFieldValue(6, info.position_.heading_);
            rs.getFieldValue(7, info.expireTime_);

            BuildingInfoMap::iterator pos = buildingInfoMap.find(buildingId);
            if (pos == buildingInfoMap.end()) {
                SNE_LOG_ERROR(__FUNCTION__ " - Can't find building guard(%" PRIu64 ")", buildingId);
                continue;
            }
            BuildingInfo& buildingInfo = (*pos).second;
            buildingInfo.buildingGuardInfos_.push_back(info);
        }

        getBuildingsCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;    
}


bool SqlServerGameDatabase::removeBuilding(ObjectId buildingId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeBuildingCommand_.paramBuildingId_.setValue(buildingId);

        removeBuildingCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateBuildingState(ObjectId buildingId, BuildingStateType state,
    sec_t startBuildTime, sec_t expireTime, HitPoint currentHitPoint)
{
    try {
        updateBuildingStateCommand_.paramBuildingId_.setValue(buildingId);
        updateBuildingStateCommand_.paramState_.setValue(state);
        updateBuildingStateCommand_.paramBuildTime_.setValue(startBuildTime);
        updateBuildingStateCommand_.paramExpireTime_.setValue(expireTime);
        updateBuildingStateCommand_.paramCurrentHp_.setValue(currentHitPoint);

        updateBuildingStateCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::addSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode, uint8_t count, sec_t completeTime)
{
    try {
        addSelectRecipeProductionCommand_.paramBuildingId_.setValue(buildingId);
        addSelectRecipeProductionCommand_.paramCode_.setValue(recipeCode);
        addSelectRecipeProductionCommand_.paramCount_.setValue(count);
        addSelectRecipeProductionCommand_.paramCompleteTime_.setValue(completeTime);
        addSelectRecipeProductionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::removeSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode)
{
    try {
        removeSelectRecipeProductionCommand_.paramBuildingId_.setValue(buildingId);
        removeSelectRecipeProductionCommand_.paramCode_.setValue(recipeCode);

        removeSelectRecipeProductionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::updateSelectRecipeProduction(ObjectId buildingId, RecipeCode recipeCode, uint8_t count, sec_t completeTime)
{
    try {
        updateSelectRecipeProductionCommand_.paramBuildingId_.setValue(buildingId);
        updateSelectRecipeProductionCommand_.paramCode_.setValue(recipeCode);
        updateSelectRecipeProductionCommand_.paramCount_.setValue(count);
        updateSelectRecipeProductionCommand_.paramCompleteTime_.setValue(completeTime);
        updateSelectRecipeProductionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}



bool SqlServerGameDatabase::updateBuildingOwner(ObjectId buildingId,
    BuildingOwnerType ownerType, ObjectId playerId, GuildId guildId)
{

    try {
        updateBuildingOwnerCommand_.paramBuildingId_.setValue(buildingId);
        updateBuildingOwnerCommand_.paramOwnerType_.setValue(ownerType);
        updateBuildingOwnerCommand_.paramPlayerId_.setValue(playerId);
        updateBuildingOwnerCommand_.paramGuildId_.setValue(guildId);
        
        updateBuildingOwnerCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}



bool SqlServerGameDatabase::moveBuildingInventoryItem(ObjectId itemId, SlotId slotId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        moveBuildingItemCommand_.paramItemId_.setValue(itemId);
        moveBuildingItemCommand_.paramSlotId_.setValue(uint8_t(slotId));

        moveBuildingItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::switchBuildingInventoryItem(ObjectId itemId1, SlotId slotId1,
    ObjectId itemId2, SlotId slotId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        switchBuildingItemCommand_.paramItemId1_.setValue(itemId1);
        switchBuildingItemCommand_.paramSlotId1_.setValue(uint8_t(slotId1));
        switchBuildingItemCommand_.paramItemId2_.setValue(itemId2);
        switchBuildingItemCommand_.paramSlotId2_.setValue(uint8_t(slotId2));

        switchItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addBuildingInventoryItem(ObjectId buildingId,
    const ItemInfo& itemInfo, InvenType invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBuildingItemCommand_.paramBuildingId_.setValue(buildingId);
        addBuildingItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addBuildingItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addBuildingItemCommand_.paramSlotId1_.setValue(uint8_t(itemInfo.slotId_));
        addBuildingItemCommand_.paramItemCount_.setValue(uint8_t(itemInfo.count_));
        addBuildingItemCommand_.paramInvenType_.setValue(invenType);
        addBuildingItemCommand_.expireAt_.setValue(uint64_t(itemInfo.expireTime_));
        addBuildingItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addBuildingInventoryEquipItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBuildingEquipItemCommand_.paramBuildingId_.setValue(buildingId);
        addBuildingEquipItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addBuildingEquipItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addBuildingEquipItemCommand_.paramSlotId1_.setValue(uint8_t(itemInfo.slotId_));
        addBuildingEquipItemCommand_.paramItemCount_.setValue(uint8_t(itemInfo.count_));
        addBuildingEquipItemCommand_.expireAt_.setValue(uint64_t(itemInfo.expireTime_));
        setOptionValue(addBuildingEquipItemCommand_, itemInfo.equipItemInfo_.addOptions_);
        addBuildingEquipItemCommand_.paramSocketCount_.setValue(itemInfo.equipItemInfo_.socketCount_);
        addBuildingEquipItemCommand_.paramInvenType_.setValue(invenType);

        addBuildingEquipItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addBuildingEquipSocketOption(ObjectId buildingId, ObjectId itemId, SocketSlotId slotId, const EquipSocketInfo& socketInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBuildingEquipSocketOptionCommand_.paramBuildingId_.setValue(buildingId);
        addBuildingEquipSocketOptionCommand_.paramItemId_.setValue(itemId);
        addBuildingEquipSocketOptionCommand_.paramSlotId_.setValue(slotId);     
        addBuildingEquipSocketOptionCommand_.paramGemCode_.setValue(socketInfo.gemCode_);
        setOptionValue(addBuildingEquipSocketOptionCommand_, socketInfo.scriptInfos_);
        addBuildingEquipSocketOptionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addBuildingInventoryAccessoryItem(ObjectId buildingId, const ItemInfo& itemInfo, InvenType invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBuildingAccessoryItemCommand_.paramBuildingId_.setValue(buildingId);
        addBuildingAccessoryItemCommand_.paramItemId_.setValue(itemInfo.itemId_);
        addBuildingAccessoryItemCommand_.paramItemCode_.setValue(itemInfo.itemCode_);
        addBuildingAccessoryItemCommand_.paramSlotId_.setValue(uint8_t(itemInfo.slotId_));
        addBuildingAccessoryItemCommand_.paramItemCount_.setValue(uint8_t(itemInfo.count_));
        addBuildingAccessoryItemCommand_.expireAt_.setValue(uint64_t(itemInfo.expireTime_));
        setOptionValue(addBuildingAccessoryItemCommand_, itemInfo.equipItemInfo_.addOptions_);
        addBuildingAccessoryItemCommand_.paramInvenType_.setValue(invenType);

        addBuildingAccessoryItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}



bool SqlServerGameDatabase::removeBuildingInventoryItem(ObjectId itemId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeBuildingItemCommand_.paramItemId_.setValue(itemId);

        removeBuildingItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateBuildingInventoryItemCount(ObjectId itemId, uint8_t itemCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateBuildingItemCountCommand_.paramItemId_.setValue(itemId);
        updateBuildingItemCountCommand_.paramItemCount_.setValue(itemCount);

        updateBuildingItemCountCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeAllBuildingInventoryItem(ObjectId buildingId, InvenType invenType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeAllBuildingItemCommand_.paramBuildingId_.setValue(buildingId);
        removeAllBuildingItemCommand_.paramInvenType_.setValue(invenType);

        removeAllBuildingItemCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addBuildingGuard(ObjectId buildingId, const BuildingGuardInfo& info)
{
   sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBuildingGuardCommand_.paramGuardId_.setValue(info.objectId_);
        addBuildingGuardCommand_.paramBuildingId_.setValue(buildingId);
        addBuildingGuardCommand_.paramNpcCode_.setValue(info.npcCode_);
        addBuildingGuardCommand_.paramPosX_.setValue(info.position_.x_);
        addBuildingGuardCommand_.paramPosY_.setValue(info.position_.y_);
        addBuildingGuardCommand_.paramPosZ_.setValue(info.position_.z_);
        addBuildingGuardCommand_.paramPosHeading_.setValue(info.position_.heading_);
        addBuildingGuardCommand_.paramExpireTime_.setValue(info.expireTime_);

        addBuildingGuardCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeBuildingGuard(ObjectId guraderId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeBuildingGuardCommand_.paramGuardId_.setValue(guraderId);

        removeBuildingGuardCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addBindRecallInfo(AccountId accountId, ObjectId characterId,
    const BindRecallInfo& bindRecallInfo)
{
    try {
        addBindRecallInfoCommand_.paramAccountId_.setValue(accountId);
        addBindRecallInfoCommand_.paramCharacterId_.setValue(characterId);    
        addBindRecallInfoCommand_.paramLinkId_.setValue(bindRecallInfo.linkId_);
        addBindRecallInfoCommand_.paramDataCode_.setValue(bindRecallInfo.dataCode_);
        addBindRecallInfoCommand_.paramWorldMapCode_.setValue(bindRecallInfo.wordlPosition_.mapCode_);
        addBindRecallInfoCommand_.paramPosX_.setValue(bindRecallInfo.wordlPosition_.x_);
        addBindRecallInfoCommand_.paramPosY_.setValue(bindRecallInfo.wordlPosition_.y_);
        addBindRecallInfoCommand_.paramPosZ_.setValue(bindRecallInfo.wordlPosition_.z_);
        addBindRecallInfoCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::removeBindRecallInfo(ObjectId linkId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeBindRecallCommand_.paramLinkId_.setValue(linkId);

        removeBindRecallCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::releaseBeginnerProtection(ObjectId characterId)
{

    sne::server::Profiler profiler(__FUNCTION__);

    try {
        releaseBeginnerProtectionCommand_.paramCharacterId_.setValue(characterId);

        releaseBeginnerProtectionCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addCooldown(AccountId accountId, ObjectId characterId, DataCode dataCode, sec_t expireTime)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addCooldownCommand_.paramAccountId_.setValue(accountId);
        addCooldownCommand_.paramCharacterId_.setValue(characterId);
        addCooldownCommand_.paramDataCode_.setValue(dataCode);
        addCooldownCommand_.paramExpireTime_.setValue(expireTime);

        addCooldownCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeAllCooldown(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeAllCooldownCommand_.paramCharacterId_.setValue(characterId);

        removeAllCooldownCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addRemainEffect(AccountId accountId, ObjectId characterId, const RemainEffectInfo& effectInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addRemainEffectCommand_.paramAccountId_.setValue(accountId);
        addRemainEffectCommand_.paramCharacterId_.setValue(characterId);
        addRemainEffectCommand_.paramDataCode_.setValue(effectInfo.dataCode_);
        addRemainEffectCommand_.paramUseTargetStatusType_.setValue(effectInfo.useTargetStatusType_);
        addRemainEffectCommand_.paramIsCaster_.setValue(effectInfo.isCaster_);
        addRemainEffectCommand_.paramEffectValue1_.setValue(effectInfo.effects_[0].effectValue_);
        addRemainEffectCommand_.paramExpireTime1_.setValue(effectInfo.effects_[0].expireActivateTime_);
        addRemainEffectCommand_.paramEffectValue2_.setValue(effectInfo.effects_[1].effectValue_);
        addRemainEffectCommand_.paramExpireTime2_.setValue(effectInfo.effects_[1].expireActivateTime_);
        addRemainEffectCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::removeAllRemainEffect(ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeAllRemainEffectCommand_.paramCharacterId_.setValue(characterId);

        removeAllRemainEffectCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateCharacterInventoryInfo(ObjectId characterId, 
    InvenType invenType, bool isCashSlot, uint8_t count)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateCharacterInventoryInfoCommand_.paramCharacterId_.setValue(characterId);
        updateCharacterInventoryInfoCommand_.paramInvenType_.setValue(invenType);
        updateCharacterInventoryInfoCommand_.paramIsCashSlot_.setValue(isCashSlot);
        updateCharacterInventoryInfoCommand_.paramCount_.setValue(count);

        updateCharacterInventoryInfoCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::createVehicle(AccountId accountId, ObjectId characterId, const VehicleInfo& info)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        createVehicleCommand_.paramCharacterId_.setValue(characterId);
        createVehicleCommand_.paramAccountId_.setValue(accountId);
        createVehicleCommand_.paramId_.setValue(info.objectId_);
        createVehicleCommand_.paramCode_.setValue(info.vehicleCode_);
        createVehicleCommand_.paramBirthday_.setValue(info.birthDay_);
        createVehicleCommand_.paramPeakAge_.setValue(info.peakAge_);

        createVehicleCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::createGlider(AccountId accountId, ObjectId characterId, const GliderInfo& info)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        createGliderCommand_.paramCharacterId_.setValue(characterId);
        createGliderCommand_.paramAccountId_.setValue(accountId);
        createGliderCommand_.paramId_.setValue(info.objectId_);
        createGliderCommand_.paramCode_.setValue(info.gliderCode_);
        createGliderCommand_.paramDurability_.setValue(info.durability_);

        createGliderCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::deleteVehicle(ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        deleteVehicleCommand_.paramId_.setValue(id);

        deleteVehicleCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::deleteGlider(ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        deleteGliderCommand_.paramId_.setValue(id);

        deleteGliderCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::selectVehicle(ObjectId characterId, ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        selectVehicleCommand_.paramCharacterId_.setValue(characterId);
        selectVehicleCommand_.paramVehicleId_.setValue(id);

        selectVehicleCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::selectGlider(ObjectId characterId, ObjectId id)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        selectGliderCommand_.paramCharacterId_.setValue(characterId);
        selectGliderCommand_.paramGliderId_.setValue(id);

        selectGliderCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateGliderDurability(ObjectId id, uint32_t currentValue)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateGliderDurabilityCommand_.paramId_.setValue(id);
        updateGliderDurabilityCommand_.paramDurability_.setValue(currentValue);

        updateGliderDurabilityCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;	
}


bool SqlServerGameDatabase::getBuddies(BuddyInfos& buddyInfos, BlockInfos& blockInfos, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getBuddiesCommand_.paramCharacterId_.setValue(characterId);
    
        getBuddiesCommand_.execute();

        sne::database::AdoRecordSet& rs = getBuddiesCommand_.adoRecordSet_;

        BuddyInfo buddyInfo;
        for (; ! rs.isEof(); rs.moveNext()) {     
            rs.getFieldValue(0, buddyInfo.accountId_);
            rs.getFieldValue(1, buddyInfo.playerId_);
            rs.getFieldValue(2, buddyInfo.nickname_.ref());
            buddyInfos.push_back(buddyInfo);
        }

        rs.nextRecordset();

        for (; ! rs.isEof(); rs.moveNext()) {     
            rs.getFieldValue(0, buddyInfo.accountId_);
            rs.getFieldValue(1, buddyInfo.playerId_);
            rs.getFieldValue(2, buddyInfo.nickname_.ref());
            buddyInfos.push_back(buddyInfo);
        }

        rs.nextRecordset();

        BlockInfo blockInfo;
        for (; ! rs.isEof(); rs.moveNext()) {     
            rs.getFieldValue(0, blockInfo.accountId_);
            rs.getFieldValue(1, blockInfo.playerId_);
            rs.getFieldValue(2, blockInfo.nickname_.ref());
            blockInfos.push_back(blockInfo);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerGameDatabase::addBuddy(ObjectId characterId1, ObjectId characterId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBuddyCommand_.paramCharacterId1_.setValue(characterId1);
        addBuddyCommand_.paramCharacterId2_.setValue(characterId2);

        addBuddyCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;	
}


bool SqlServerGameDatabase::removeBuddy(ObjectId characterId1, ObjectId characterId2)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeBuddyCommand_.paramCharacterId1_.setValue(characterId1);
        removeBuddyCommand_.paramCharacterId2_.setValue(characterId2);

        removeBuddyCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;	
}


bool SqlServerGameDatabase::addBlock(ObjectId ownerId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBlockCommand_.paramOwnerId_.setValue(ownerId);
        addBlockCommand_.paramCharacterId_.setValue(characterId);

        addBlockCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;	
}


bool SqlServerGameDatabase::removeBlock(ObjectId ownerId, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeBlockCommand_.paramOwnerId_.setValue(ownerId);
        removeBlockCommand_.paramCharacterId_.setValue(characterId);

        removeBlockCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getCharacterTitles(CharacterTitleCodeSet& titleCodes, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getCharacterTitlesCommand_.paramCharacterId_.setValue(characterId);
        getCharacterTitlesCommand_.execute();
        sne::database::AdoRecordSet& rs = getCharacterTitlesCommand_.adoRecordSet_;

        for (; ! rs.isEof(); rs.moveNext()) {
            CharacterTitleCode titleCode = invalidCharacterTitleCode;
            rs.getFieldValue(0, titleCode);
            if (! isValidCharacterTitleCode(titleCode)) {
                assert(false);
                return false;
            }
            titleCodes.insert(titleCode);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getCompleteAchievements(CompleteAchievementInfoMap& completeAchievementMap, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getCompleteAchievementsCommand_.paramCharacterId_.setValue(characterId);

        getCompleteAchievementsCommand_.execute();
        sne::database::AdoRecordSet& rs = getCompleteAchievementsCommand_.adoRecordSet_;

        for (; ! rs.isEof(); rs.moveNext()) {
            AchievementCode achievementCode = invalidAchievementCode;
            sec_t completeTime = 0;
            rs.getFieldValue(0, achievementCode);
            if (! isValidAchievementCode(achievementCode)) {
                assert(false);
                return false;
            }
            rs.getFieldValue(1, completeTime);
            completeAchievementMap.emplace(achievementCode,
                CompleteAchievementInfo(achievementCode, completeTime));
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::getProcessAchievements(ProcessAchievementInfoMap& achievementMap, ObjectId characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getProcessAchievementsCommand_.paramCharacterId_.setValue(characterId);
        getProcessAchievementsCommand_.execute();
        sne::database::AdoRecordSet& rs = getProcessAchievementsCommand_.adoRecordSet_;

        for (; ! rs.isEof(); rs.moveNext()) {
            AchievementCode achievementCode = invalidAchievementCode;
            ProcessAchievementInfo missionInfo;
            rs.getFieldValue(0, achievementCode);
            if (! isValidAchievementCode(achievementCode)) {
                assert(false);
                return false;
            }
            rs.getFieldValue(1, missionInfo.param1_);
            rs.getFieldValue(2, missionInfo.param2_);
            rs.getFieldValue(3, missionInfo.lastUpdateTime_);

            ProcessAchievementInfoMap::iterator pos = achievementMap.find(achievementCode);
            if (pos != achievementMap.end()) {
                assert(false);
                return false;
            }
            achievementMap.emplace(achievementCode, missionInfo);
            
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addCharacterTitle(ObjectId characterId, CharacterTitleCode titleCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addCharacterTitleCommand_.paramPlayerId_.setValue(characterId);
        addCharacterTitleCommand_.paramTitleCode_.setValue(titleCode);

        addCharacterTitleCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::addProcessAchievement(ObjectId characterId, AchievementCode code,
    const ProcessAchievementInfo& missionInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addProcessAchievementCommand_.paramPlayerId_.setValue(characterId);
        addProcessAchievementCommand_.paramAchievementCode_.setValue(code);
        addProcessAchievementCommand_.paramParam1_.setValue(missionInfo.param1_);
        addProcessAchievementCommand_.paramParam2_.setValue(missionInfo.param2_);
        addProcessAchievementCommand_.paramLastUpdateTime_.setValue(missionInfo.lastUpdateTime_);

        addProcessAchievementCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::updateProcessAchievement(ObjectId characterId, AchievementCode code,
    const ProcessAchievementInfo& missionInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateProcessAchievementCommand_.paramPlayerId_.setValue(characterId);
        updateProcessAchievementCommand_.paramAchievementCode_.setValue(code);
        updateProcessAchievementCommand_.paramParam1_.setValue(missionInfo.param1_);
        updateProcessAchievementCommand_.paramParam2_.setValue(missionInfo.param2_);
        updateProcessAchievementCommand_.paramLastUpdateTime_.setValue(missionInfo.lastUpdateTime_);

        updateProcessAchievementCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerGameDatabase::completeAchievement(ObjectId characterId, AchievementCode code, AchievementPoint point, sec_t now)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        completeAchievementCommand_.paramPlayerId_.setValue(characterId);
        completeAchievementCommand_.paramAchievementCode_.setValue(code);
        completeAchievementCommand_.paramAchievementPoint_.setValue(point);
        completeAchievementCommand_.paramCompleteTime_.setValue(now);

        completeAchievementCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}

}} // namespace gideon { namespace serverbase {
