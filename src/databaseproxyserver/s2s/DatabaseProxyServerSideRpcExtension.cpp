#include "DatabaseProxyServerPCH.h"
#include "DatabaseProxyServerSideRpcExtension.h"
#include "../cache/account/AccountCacheManager.h"
#include "../cache/account/AccountCache.h"
#include <gideon/serverbase/database/GameDatabase.h>
#include <gideon/cs/shared/data/ServerType.h>
#include <sne/server/utility/Profiler.h>
#include <sne/database/proxy/DatabaseRequest.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/utility/Logger.h>
#include <sne/base/utility/Assert.h>

namespace gideon { namespace databaseproxyserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(DatabaseProxyServerSideRpcExtension);

void DatabaseProxyServerSideRpcExtension::attachedTo(
    sne::base::SessionImpl& sessionImpl)
{
    Parent::attachedTo(sessionImpl);

    sne::sgp::RpcingExtension* extension =
        sessionImpl.getExtension<sne::sgp::RpcingExtension>();
    extension->registerRpcForwarder(*this);
    extension->registerRpcReceiver(*this);
}


// = sne::srpc::RpcForwarder overriding

void DatabaseProxyServerSideRpcExtension::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("DatabaseProxyServerSideRpcExtension::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void DatabaseProxyServerSideRpcExtension::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("DatabaseProxyServerSideRpcExtension::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_getProperties,
    RequestId, requestId)
{
    sne::server::Properties properties;
    bool result = true;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getProperties(properties);
    }

    s2d_onGetProperties(requestId, result ? ecOk : ecDatabaseInternalError,
        properties);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetProperties,
    RequestId, requestId, ErrorCode, errorCode,
    sne::server::Properties, properties);


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_getServerSpec,
    RequestId, requestId, std::string, serverName, std::string, suffix)
{
    sne::server::ServerSpec serverSpec;
    bool result = true;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getServerSpec(serverSpec, serverName, suffix);
    }

    s2d_onGetServerSpec(requestId, result ? ecOk : ecDatabaseInternalError,
        serverSpec);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetServerSpec,
    RequestId, requestId, ErrorCode, errorCode, sne::server::ServerSpec, spec);


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getShardInfo,
    RequestId, requestId, ShardId, shardId)
{
    FullShardInfo shardInfo;
    bool result = true;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getShardInfo(shardInfo, shardId);
    }

    s2d_onGetShardInfo(requestId, result ? ecOk : ecDatabaseInternalError,
        shardInfo);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetShardInfo,
    RequestId, requestId, ErrorCode, errorCode, FullShardInfo, shardInfo);


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getZoneInfo,
    RequestId, requestId, std::string, name) {
    ZoneInfo zoneInfo;
    bool result = true;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getZoneInfo(zoneInfo, name);
    }

    zoneInfo.name_ = name;
    s2d_onGetZoneInfo(requestId, result ? ecOk : ecDatabaseInternalError,
        zoneInfo);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetZoneInfo,
    RequestId, requestId, ErrorCode, errorCode, ZoneInfo, zoneInfo);


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getZoneInfoList,
    RequestId, requestId, ShardId, shardId)
{
    ZoneInfos zoneInfos;
    bool result = true;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getZoneInfoList(zoneInfos, shardId);
    }

    s2d_onGetZoneInfoList(requestId, result ? ecOk : ecDatabaseInternalError,
        zoneInfos);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetZoneInfoList,
    RequestId, requestId, ErrorCode, errorCode, ZoneInfos, zoneInfos);


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getWorldTime,
    RequestId, requestId, ZoneId, zoneId)
{
    WorldTime worldTime = minWorldTime;
    bool result = true;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getWorldTime(worldTime, zoneId);
    }

    s2d_onGetWorldTime(requestId, result ? ecOk : ecDatabaseInternalError,
        worldTime);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetWorldTime,
    RequestId, requestId, ErrorCode, errorCode, WorldTime, worldTime);


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getFullUserInfo,
    RequestId, requestId, AccountId, accountId)
{
    ErrorCode errorCode = ecOk;
    FullUserInfo userInfo;

    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->removeOldCompleteRepeatQuest();
        cache->deleteAccountExpireItems();
        userInfo = cache->getFullUserInfo();
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);

        userInfo.accountId_ = accountId;
        errorCode = ecDatabaseInternalError;
    }

    s2d_onGetFullUserInfo(requestId, errorCode, userInfo);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetFullUserInfo,
    RequestId, requestId, ErrorCode, errorCode, FullUserInfo, userInfo);


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_createCharacter,
    RequestId, requestId, CreateCharacterInfo, createCharacterInfo, CreateCharacterEquipments, createCharacterEquipments, 
    ZoneId, zoneId, ObjectPosition, position)
{
    FullCharacterInfo characterInfo;

    ErrorCode errorCode = ecOk;
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(createCharacterInfo.accountId_);
    if (cache != nullptr) {
        errorCode = cache->createCharacter(characterInfo, createCharacterInfo, createCharacterEquipments, zoneId, position);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            createCharacterInfo.accountId_);
        errorCode = ecDatabaseInternalError;
    }

    s2d_onCreateCharacter(requestId, errorCode, characterInfo);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onCreateCharacter,
    RequestId, requestId, ErrorCode, errorCode, FullCharacterInfo, characterInfo);


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_deleteCharacter,
    RequestId, requestId, AccountId, accountId, ObjectId, characterId)
{
	GuildId guildId = invalidGuildId;
    ErrorCode errorCode = ecOk;
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        errorCode = cache->deleteCharacter(guildId, characterId);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
        errorCode = ecDatabaseInternalError;
    }

    s2d_onDeleteCharacter(requestId, errorCode, characterId, guildId);
}


FORWARD_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_onDeleteCharacter,
    RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId, GuildId, guildId);


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_checkDuplicateNickname,
    RequestId, requestId, AccountId, accountId, Nickname, nickname)
{
    ErrorCode errorCode = ecOk;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        errorCode = db->checkDuplicateNickname(nickname);
    }
    s2d_onCheckDuplicateNickname(requestId, errorCode, accountId, nickname);
}


FORWARD_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_onCheckDuplicateNickname,
    RequestId, requestId, ErrorCode, errorCode, AccountId, accountId, Nickname, nickname);


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_saveCharacterStats, 
    AccountId, accountId, ObjectId, characterId, DBCharacterStats, saveInfo)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->saveCharacterStats(characterId, saveInfo);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_saveSelectCharacterTitle,
    AccountId, accountId, ObjectId, characterId,
    CharacterTitleCode, titleCode)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->saveSelectCharacterTitle(characterId, titleCode);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_loadCharacterProperties, 
    RequestId, requestId, AccountId, accountId, ObjectId, characterId)
{
    ErrorCode errorCode = ecOk;
    std::string config;
    std::string prefs;
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        errorCode = cache->getCharacterProperties(config, prefs, characterId);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
        errorCode = ecDatabaseInternalError;
    }

    s2d_onLoadCharacterProperties(requestId, errorCode, characterId, config, prefs);
}


FORWARD_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_onLoadCharacterProperties,
    RequestId, requestId, ErrorCode, errorCode, ObjectId, characterId,
    std::string, config, std::string, prefs);


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_saveCharacterProperties, 
    AccountId, accountId, ObjectId, characterId,
    std::string, config, std::string, prefs)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->saveCharacterProperties(characterId, config, prefs);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getMaxInventoryId,
    RequestId, requestId, ZoneId, zoneId)
{
    ObjectId maxInventoryId = invalidObjectId;
    bool result = true;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getMaxInventoryId(maxInventoryId, zoneId);
    }

    s2d_onGetMaxInventoryId(requestId, result ? ecOk : ecDatabaseInternalError,
        maxInventoryId);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetMaxInventoryId,
    RequestId, requestId, ErrorCode, errorCode, ObjectId, maxInventoryId);


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_moveInventoryItem,
    AccountId, accountId, ObjectId, characterId,
    InvenType, invenType, ObjectId, itemId, SlotId, slotId)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {		
		cache->moveInventoryItem(characterId, itemId, slotId, invenType);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_switchInventoryItem,
    AccountId, accountId, ObjectId, characterId,
    InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
		cache->switchInventoryItem(characterId, itemId1, itemId2, invenType);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_addInventoryItem,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, ItemInfo, itemInfo)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
		cache->addInventoryItem(characterId, itemInfo, invenType);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_changeEquipItemInfo,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->changeEquipItemInfo(characterId, itemId, newEquipCode, socketCount);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_enchantEquipItem,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
    SocketSlotId, id, EquipSocketInfo, socketInfo)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->enchantEquipItem(characterId, itemId, id, socketInfo);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_unenchantEquipItem,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId, SocketSlotId, id)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->unenchantEquipItem(characterId, itemId, id);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_removeInventoryItem,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, ObjectId, itemId)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
		cache->removeInventoryItem(characterId, itemId, invenType);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}

    
RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_updateInventoryItemCount,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType, 
	ObjectId, itemId, uint8_t, itemCount)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {		
		cache->updateInventoryItemCount(characterId, itemId, itemCount, invenType);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addQuestItem,
    AccountId, accountId, ObjectId, characterId, QuestItemInfo, questItemInfo)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {		
        cache->addQuestItem(characterId, questItemInfo);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_removeQuestItem,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {		
        cache->removeQuestItem(characterId, itemId);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateQuestItemUsableCount,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
    uint8_t, ueableCount)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {		
        cache->updateQuestItemUsableCount(characterId, itemId, ueableCount);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}

RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateQuestItemCount,
    AccountId, accountId, ObjectId, characterId, ObjectId, itemId,
    uint8_t, stackCount)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {		
        cache->updateQuestItemCount(characterId, itemId, stackCount);		
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_equipItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, EquipPart, equipPart)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->equipItem(characterId, itemId, equipPart);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_unequipItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, SlotId, slotId, EquipPart, unequipPart)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->unequipItem(characterId, itemId, slotId, unequipPart);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_6(DatabaseProxyServerSideRpcExtension, s2d_replaceInventoryWithEquipItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, unequipItemId, EquipPart, unequipPart, ObjectId, equipItemId,
    EquipPart, equipPart)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->replaceInventoryWithEquipItem(characterId, unequipItemId,
            unequipPart, equipItemId, equipPart);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_equipAccessoryItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, AccessoryIndex, equipPart)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->equipAccessoryItem(characterId, itemId, equipPart);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_unequipAccessoryItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPart)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->unequipAccessoryItem( characterId, itemId, slotId, unequipPart);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_6(DatabaseProxyServerSideRpcExtension, s2d_replaceInventoryWithAccessoryItem,
    AccountId, accountId, ObjectId, characterId,
    ObjectId, unequipItemId, AccessoryIndex, unequipPart, ObjectId, equipItemId,
    AccessoryIndex, equipPart)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->replaceInventoryWithAccessoryItem(characterId, unequipItemId,
            unequipPart, equipItemId, equipPart);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}

RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_changeCharacterState,
    AccountId, accountId, ObjectId, characterId, CreatureStateType, state)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->changeCharacterState(characterId, state);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_saveActionBar,
    AccountId, accountId, ObjectId, characterId, ActionBarIndex, abiIndex, 
    ActionBarPosition, abpIndex, DataCode, code)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->saveActionBar(characterId, abiIndex, abpIndex, code);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_learnSkill,
    AccountId, accountId, ObjectId, characterId, SkillCode, currentSkillCode, 
    SkillCode, learnSkillCode)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->learnSkill(characterId, currentSkillCode, learnSkillCode);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_removeSkill,
    AccountId, accountId, ObjectId, characterId, SkillCode, skillCode)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->removeSkill(characterId, skillCode);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeAllSkill,
    AccountId, accountId, ObjectId, characterId)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->removeAllSkill(characterId);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_acceptQuest,
	AccountId, accountId, ObjectId, characterId, QuestCode, questCode)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->acceptQuest(characterId, questCode);
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_acceptRepeatQuest,
	AccountId, accountId, ObjectId, characterId, QuestCode, questCode, sec_t, acceptRepeatTime)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->acceptRepeatQuest(characterId, questCode, acceptRepeatTime);
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_cancelQuest,
	AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->cancelQuest(characterId, questCode, isRepeatQuest);
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_completeQuest,
	AccountId, accountId, ObjectId, characterId, QuestCode, questCode, bool, isRepeatQuest)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->completeQuest(characterId, questCode, isRepeatQuest);
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_6(DatabaseProxyServerSideRpcExtension, s2d_updateQuestMission,
	AccountId, accountId, ObjectId, characterId, QuestCode, questCode,
	QuestMissionCode, missionCode, QuestGoalInfo, goalInfo, bool, isRepeatQuest)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->updateQuestMission(characterId, questCode, missionCode, goalInfo, isRepeatQuest);
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_removeCompleteQuest,
	AccountId, accountId, ObjectId, characterId, QuestCode, questCode)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->removeCompleteQuest(characterId, questCode);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_getGuildInfos,
    RequestId, requestId)
{
    GuildInfos guildInfos;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->getGuildInfos(guildInfos)) {
            s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
            return;
        }

        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildRanks(guildInfo, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }

        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildBankRights(guildInfo, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }

        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildVaults(guildInfo, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }
        
        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildMemberInfos(guildInfo.guildId_, guildInfo.guildMemberInfos_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }

        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildSkills(guildInfo, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }

        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildRelationships(guildInfo.guildId_, guildInfo.relytionships_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }
        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildApplicants(guildInfo.guildId_, guildInfo.guildApplicantInfoMap_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }
        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildInventory(guildInfo.inventoryInfo_, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }
        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildEventLog(guildInfo.eventLogManager_, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }
        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildBankEventLog(guildInfo.eventBankLogManager_, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }
        for (GuildInfo& guildInfo : guildInfos) {
            if (! db->getGuildGameMoneyEventLog(guildInfo.guildGameMoneyEventLogManager_, guildInfo.guildId_)) {
                s2d_onGetGuildInfos(requestId, ecDatabaseInternalError, GuildInfos());
                return;
            }
        }
    }
    s2d_onGetGuildInfos(requestId, ecOk, guildInfos);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetGuildInfos,
    RequestId, requestId, ErrorCode, errorCode, GuildInfos, guildInfos);


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_getMaxGuildId,
    RequestId, requestId)
{
    GuildId maxGuildId = invalidGuildId;
    bool result = false;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getMaxGuildId(maxGuildId);         
    }
    s2d_onGetMaxGuildId(requestId, result ? ecOk : ecDatabaseInternalError, maxGuildId);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetMaxGuildId,
    RequestId, requestId, ErrorCode, errorCode, GuildId, guildId);


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_createGuild,
    BaseGuildInfo, guildInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->createGuild(guildInfo)) {
        // TODO: 로그를 남긴다
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addGuildMember,
    AccountId, accountId, GuildId, guildId, GuildMemberInfo, memberInfo)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->setPlayerGuildInfo(guildId, memberInfo, true);        
    }
    else {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->addGuildMember(guildId, memberInfo)) {
            // TODO: 로그를 남긴다
        }        
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_removeGuildMember,
    ObjectId, characterId)
{
    AccountId accountId = ACCOUNTCACHE_MANAGER->getAccountId(characterId);

    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->leaveGuild(characterId);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->removeGuildMember(characterId)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addGuildRelationship,
    GuildId, ownerGuildId, GuildId, targetGuildId, GuildRelatioshipType, type)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->addGuildRelationship(ownerGuildId, targetGuildId, type)) {
        SNE_LOG_ERROR("s2d_addGuildRelationship(OG%u, TG%u) Failed - " __FUNCTION__,
            ownerGuildId, targetGuildId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeGuildRelationship,
    GuildId, ownerGuildId, GuildId, targetGuildId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->removeGuildRelationship(ownerGuildId, targetGuildId)) {
        SNE_LOG_ERROR("s2d_removeGuildRelationship(OG%u, TG%u) Failed - " __FUNCTION__,
            ownerGuildId, targetGuildId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_changeGuildMemberPosition,
	ObjectId, characterId, GuildMemberPosition, position)
{
	sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

	if (! db->changeGuildMemberPosition(characterId, position)) {
        SNE_LOG_ERROR("s2d_changeGuildMemberPosition(C%" PRIu64 ", %d) Failed - " __FUNCTION__,
            characterId, position);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_addGuildApplicant,
    ObjectId, characterId, GuildId, guildId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->addGuildApplicant(characterId, guildId)) {
        SNE_LOG_ERROR("s2d_addGuildApplicant(C%" PRIu64 ", G%u) Failed - " __FUNCTION__,
            characterId, guildId);
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_removeGuildApplicant,
    ObjectId, characterId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->removeGuildApplicant(characterId)) {
        SNE_LOG_ERROR("s2d_removeGuildApplicant(C%" PRIu64 ") Failed - " __FUNCTION__,
            characterId);
        return;
    }
}

RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_modifyGuildIntroduction,
    GuildId, guildId, GuildIntroduction, introduction)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->modifyGuildIntroduction(guildId, introduction)) {
        SNE_LOG_ERROR("s2d_modifyGuildIntroduction(G%u, L%u) Failed - " __FUNCTION__,
            guildId, introduction.size());
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_modifyGuildNotice,
    GuildId, guildId, GuildNotice, notice)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->modifyGuildNotice(guildId, notice)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addGuildRank,
    GuildId, guildId, GuildRankInfo, rankInfo, uint8_t, vaultCount)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->addGuildRank(guildId, rankInfo, vaultCount)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_addGuilBankRank,
    GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, GuildBankVaultRightInfo, bankRankInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->addGuildBankRights(guildId, rankId, vaultId, bankRankInfo)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_deleteGuildRank,
    GuildId, guildId, GuildRankId, rankId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->deleteGuildRank(guildId, rankId)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_swapGuildRank,
    GuildId, guildId, GuildRankId, rankId1, GuildRankId, rankId2)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->swapGuildRank(guildId, rankId1, rankId2)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_updateGuildRankName,
    GuildId, guildId, GuildRankId, rankId, GuildRankName, rankName)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->updateGuildRankName(guildId, rankId, rankName)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateGuildRankRights,
    GuildId, guildId, GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->updateGuildRankRights(guildId, rankId, rights, goldWithdrawalPerDay)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_updateGuildBankRights,
    GuildId, guildId, GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->updateGuildBankRights(guildId, rankId, vaultId, rights, itemWithdrawalPerDay)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_updateGuildRank,
    GuildId, guildId, ObjectId, playerId, GuildRankId, rankId)
{
    guildId;

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->updateGuildRank(playerId, rankId)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_removeAllGuildSkills,
    GuildId, guildId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->removeAllGuildSkills(guildId)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeGuildSkills,
    GuildId, guildId, SkillCodes, skillCodes)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    for (SkillCode skillCode : skillCodes) {
        if (! db->removeGuildSkill(guildId, skillCode)) {
            // TODO: 로그를 남긴다
            return;
        }
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_addGuildSkill,
    GuildId, guildId, SkillCode, skillCode)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->addGuildSkill(guildId, skillCode)) {
        // TODO: 로그를 남긴다
        return;
    }    
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_updateGuildExp,
    GuildId, guildId, GuildLevelInfo, guildLevelInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->updateGuildExp(guildId, guildLevelInfo.guildExp_, guildLevelInfo.skillPoint_,
        guildLevelInfo.dayGuildAddExp_, guildLevelInfo.lastUpdateExpTime_)) {
        // TODO: 로그를 남긴다
        return;
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_moveGuildInventoryItem,
    GuildId, guildId, VaultId, vaultId, ObjectId, itemId, SlotId, slotId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->moveGuildInventoryItem(itemId, vaultId, slotId)) {
        SNE_LOG_ERROR("s2d_moveGuildInventoryItem(%u, %" PRIu64 ") Failed - " __FUNCTION__,
            guildId, itemId);
    }
}


RECEIVE_SRPC_METHOD_6(DatabaseProxyServerSideRpcExtension, s2d_switchGuildInventoryItem,
    GuildId, guildId, VaultId, vaultId, ObjectId, itemId1, SlotId, slotId1, ObjectId, itemId2, SlotId, slotId2)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->switchGuildInventoryItem(itemId1, vaultId, slotId1, itemId2, vaultId, slotId2)) {
        SNE_LOG_ERROR("s2d_switchGuildInventoryItem(%u, %" PRIu64 ", %" PRIu64 ") Failed - " __FUNCTION__,
            guildId, itemId1, itemId2);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addGuildInventoryItem,
    GuildId, guildId, VaultId, vaultId, ItemInfo, itemInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    

    if (itemInfo.isEquipment()) {
        if (db->addGuildInventoryEquipItem(guildId, vaultId, itemInfo)) {
            for (const EquipSocketInfoMap::value_type& value : itemInfo.equipItemInfo_.equipSocketInfoMap_) {
                const EquipSocketInfo& socketInfo = value.second;
                if (! db->addGuildEquipSocketOption(guildId, itemInfo.itemId_, value.first, socketInfo)) {
                    SNE_LOG_ERROR("addGuildEquipSocketOption(%u, %" PRIu64 ") Failed - " __FUNCTION__,
                        guildId, itemInfo.itemId_);
                }
            }
        }
    }
    else if (itemInfo.isAccessory()) {
        if (! db->addGuildInventoryAccessoryItem(guildId, vaultId, itemInfo)) {
            SNE_LOG_ERROR("s2d_addGuildInventoryAccessoryItem(%u, %" PRIu64 ") Failed - " __FUNCTION__,
                guildId, itemInfo.itemId_);
        }
    }
    else {
        if (! db->addGuildInventoryItem(guildId, vaultId, itemInfo)) {
            SNE_LOG_ERROR("s2d_addGuildInventoryItem(%u, %" PRIu64 ") Failed - " __FUNCTION__,
                guildId, itemInfo.itemId_);
        }
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeGuildInventoryItem,
    GuildId, guildId, ObjectId, itemId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->removeGuildInventoryItem(itemId)) {
        SNE_LOG_ERROR("s2d_removeGuildInventoryItem(%u, %" PRIu64 ") Failed - " __FUNCTION__,
            guildId, itemId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_updateGuildInventoryItemCount,
    GuildId, guildId, ObjectId, itemId, uint8_t, count)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->updateGuildInventoryItemCount(itemId, count)) {
        SNE_LOG_ERROR("s2d_updateGuildInventoryItemCount(%u, %" PRIu64 ") Failed - " __FUNCTION__,
            guildId, itemId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_updateGuildGameMoney,
    GuildId, guildId, GameMoney, gameMoney)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->updateGuildGameMoney(guildId, gameMoney)) {
        SNE_LOG_ERROR("s2d_updateGuildGameMoney(%u, %u) Failed - " __FUNCTION__,
            guildId, gameMoney);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_updateGuildMemberWithdraw,
    ObjectId, playerId, GameMoney, dayWithdraw, sec_t, resetTime)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->updateGuildMemberWithdraw(playerId, dayWithdraw, resetTime)) {
        SNE_LOG_ERROR("s2d_updateGuildMemberWithdraw(%u, %u) Failed - " __FUNCTION__,
            playerId, dayWithdraw);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_updateGuildVaultName,
    GuildId, guildId, VaultId, vaultId, VaultName, name)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->updateGuildVaultName(guildId, vaultId, name)) {
        SNE_LOG_ERROR("s2d_updateGuildVaultName(%u, %u) Failed - " __FUNCTION__,
            guildId, vaultId);
    }
}

    
RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_addGuildVault,
    GuildId, guildId, VaultInfo, vaultInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->addGuildVault(guildId, vaultInfo)) {
        SNE_LOG_ERROR("s2d_addGuildVault(%u, %u) Failed - " __FUNCTION__,
            guildId);
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_removeGuild,
    GuildId, guildId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);    
    if (! db->removeGuild(guildId)) {
        SNE_LOG_ERROR("s2d_removeGuild(%u) Failed - " __FUNCTION__,
            guildId);
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_syncMail,
    ObjectId, characterId)
{
    MailInfos mailInfos;

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->getMails(mailInfos, characterId)) {
        // ERROR LOG
        return;
    }
    s2d_onSyncMail(characterId, mailInfos);
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_sendMailByNickname,
    Nickname, reveiverNickname, MailInfo, maillInfo)
{
    MailInfo realMailInfo = maillInfo;
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    
    ObjectId reveiverId = invalidObjectId;
    if (! isValidNpcCode(realMailInfo.titleInfo_.dataCode_)) {
        reveiverId = db->getPlayerId(reveiverNickname);
        if (! isValidObjectId(reveiverId)) {
            realMailInfo.titleInfo_.mailType_ = mtReturnMail;
            reveiverId = realMailInfo.titleInfo_.objectId_;
        }
    }

    if (db->insertMail(realMailInfo, reveiverId)) {
        for (const ItemMap::value_type& imValue : realMailInfo.contentInfo_.mailInventory_.items_) {
            const ItemInfo& itemInfo = imValue.second;
            if (itemInfo.isEquipment()) {
                if (db->addEquipMailItem(reveiverId, realMailInfo.titleInfo_.mailId_, itemInfo, itemInfo.equipItemInfo_)) {
                    for (const EquipSocketInfoMap::value_type& esValue : itemInfo.equipItemInfo_.equipSocketInfoMap_) {
                        if (! db->addMailEquipSocketOption(reveiverId, realMailInfo.titleInfo_.mailId_, itemInfo.itemId_, esValue.first, esValue.second)) {
                            SNE_LOG_ERROR("EquipInfo Not Find(M:%" PRIu64 ", itemId:%" PRIu64 ", itemCode: %u),",
                                maillInfo.titleInfo_.mailId_, itemInfo.itemId_, itemInfo.itemCode_);
                        }
                    }
                }
            }
            else if (itemInfo.isAccessory()) {
                if (! db->addAccessoryMailItem(reveiverId, realMailInfo.titleInfo_.mailId_, itemInfo, itemInfo.accessoryItemInfo_)) {
                    //SNE_LOG_ERROR("EquipInfo Not Find(M:%" PRIu64 ", itemId:%" PRIu64 ", itemCode: %u),",
                    //    maillInfo.titleInfo_.mailId_, itemInfo.itemId_, itemInfo.itemCode_)
                }
            }
            else {
                db->addMailItem(reveiverId, realMailInfo.titleInfo_.mailId_, itemInfo);
            }
        }
    }
    // TODO: 추후에 정상정일 경우 브로드 케스팅으로 수정
    s2d_evMailReceived(reveiverId, realMailInfo);
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_sendMailByPlayerId,
    ObjectId, playerId, MailInfo, maillInfo)
{
    MailInfo realMailInfo = maillInfo;
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

    if (db->insertMail(realMailInfo, playerId)) {
        for (const ItemMap::value_type& imValue : realMailInfo.contentInfo_.mailInventory_.items_) {
            const ItemInfo& itemInfo = imValue.second;
            if (itemInfo.isEquipment()) {
                if (db->addEquipMailItem(playerId, realMailInfo.titleInfo_.mailId_, itemInfo, itemInfo.equipItemInfo_)) {                    
                    for (const EquipSocketInfoMap::value_type& esValue : itemInfo.equipItemInfo_.equipSocketInfoMap_) {
                        if (! db->addMailEquipSocketOption(playerId, realMailInfo.titleInfo_.mailId_, itemInfo.itemId_, esValue.first, esValue.second)) {
                            SNE_LOG_ERROR("addMailEquipSocketOption(%" PRIu64 ", %" PRIu64 ") Failed - " __FUNCTION__,
                                playerId, itemInfo.itemId_);
                        }
                    }
                }
            }
            if (itemInfo.isAccessory()) {
                if (! db->addAccessoryMailItem(playerId, realMailInfo.titleInfo_.mailId_, itemInfo, itemInfo.accessoryItemInfo_)) {
                    /*SNE_LOG_ERROR("Accessory Not Find(M:%" PRIu64 ", itemId:%" PRIu64 ", itemCode: %u),",
                        maillInfo.titleInfo_.mailId_, itemInfo.itemId_, itemInfo.itemCode_);*/

                }
            }
            else {
                db->addMailItem(playerId, realMailInfo.titleInfo_.mailId_, itemInfo);
            }
        }
    }
    // TODO: 추후에 정상정일 경우 브로드 케스팅으로 수정
    s2d_evMailReceived(playerId, realMailInfo);
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_aquireItemsInMail,
    MailId, mailId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    db->deleteMailItems(mailId);
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_deleteMail,
    MailId, mailId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    db->deleteMail(mailId);
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_readMail,
    MailId, mailId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    db->readMail(mailId);
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_queryNotReadMail,
    ObjectId, characterId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    bool result = db->hasNotReadMail(characterId);
    s2d_onNotReadMail(characterId, result);
}


FORWARD_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_onNotReadMail,
    ObjectId, characterId, bool, hasNotReadMail);


FORWARD_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_onSyncMail,
    ObjectId, characterId, MailInfos, mailInfos);

    
FORWARD_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_evMailReceived,
    ObjectId, receiverId, MailInfo, maillInfo);


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getAuctionInfos,
    RequestId, requestId, ZoneId, zoneId)
{
    ServerAuctionInfoMap auctionMap;
    bool result = false;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        result = db->getAuctions(auctionMap, zoneId);         
    }
    s2d_onAuctionInfos(requestId, result ? ecOk : ecDatabaseInternalError, auctionMap);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onAuctionInfos,
    RequestId, requestId, ErrorCode, errorCode, ServerAuctionInfoMap, serverAuctionInfoMap);


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_createAuction,
    ZoneId, zoneId, NpcCode, npcCode, FullAuctionInfo, auctionInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->insertAuction(zoneId, npcCode, auctionInfo)) {
        // TODO: 로그
    }
    if (auctionInfo.itemInfo_.isEquipment()) {
        if (db->addAuctionEquipInfo(zoneId, auctionInfo.auctionId_, auctionInfo.itemInfo_.equipItemInfo_)) {
            for (const EquipSocketInfoMap::value_type& value : auctionInfo.itemInfo_.equipItemInfo_.equipSocketInfoMap_) {
                if (! db->addAuctionEquipSocketOption(zoneId, auctionInfo.auctionId_, value.first, value.second)) {
                    /*SNE_LOG_ERROR("addAuctionEquipSocketOption(%u, %" PRIu64 ") Failed - " __FUNCTION__,
                        zoneId, itemInfo.itemId_);*/
                }
            }
        }
    }
    else if (auctionInfo.itemInfo_.isAccessory()) {
        if (db->addAuctionAccessoryInfo(zoneId, auctionInfo.auctionId_,
            auctionInfo.itemInfo_.accessoryItemInfo_)) {
                // TODO: 로그
        }
    }

}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_deleteAuction,
    ZoneId, zoneId, AuctionId, auctionId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->deleteAuction(zoneId, auctionId)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateBid,
    ZoneId, zoneId, AuctionId, auctionId, ObjectId, buyerId, GameMoney, currentBidMoney)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->updateBid(zoneId, auctionId, buyerId, currentBidMoney)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_updateArenaRecord,
    AccountId, accountId, ObjectId, characterId, ArenaModeType, arenaMode,
    uint32_t, resultScore, ArenaResultType, resultType)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->updateArenaRecord(characterId, arenaMode, resultScore, resultType);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_updateArenaPoint,
    AccountId, accountId, ObjectId, characterId, ArenaPoint, arenaPoint)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->updateArenaPoint(characterId, arenaPoint);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_updateDeserterExpireTime,
    ObjectId, characterId, sec_t, deserterExpireTime)
{
    AccountId accountId = ACCOUNTCACHE_MANAGER->getAccountId(characterId);
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->updateDeserterExpireTime(characterId, deserterExpireTime);        
    }
    else {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->updateDeserterExpireTime(characterId, deserterExpireTime)) {
            // TODO: 로그
        }
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_moveBuildingInventoryItem,
    ObjectId, /*buildingId*/, InvenType, /*invenType*/, ObjectId, itemId, SlotId, slotId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->moveBuildingInventoryItem(itemId, slotId)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_6(DatabaseProxyServerSideRpcExtension, s2d_switchBuildingInventoryItem,
    ObjectId, /*buildingId*/, InvenType, /*invenType*/, ObjectId, itemId1, ObjectId, itemId2,
    SlotId, slot1, SlotId, slot2)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->switchBuildingInventoryItem(itemId1, slot1, itemId2, slot2)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addBuildingInventoryItem,
    ObjectId, buildingId, InvenType, invenType, ItemInfo, itemInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->addBuildingInventoryItem(buildingId, itemInfo, invenType)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_removeBuildingInventoryItem,
    ObjectId, /*buildingId*/, InvenType, /*invenType*/, ObjectId, itemId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->removeBuildingInventoryItem(itemId)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateBuildingInventoryItemCount,
    ObjectId, /*buildingId*/, InvenType, /*invenType*/, ObjectId, itemId, uint8_t, itemCount)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->updateBuildingInventoryItemCount(itemId, itemCount)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeAllBuildingInventoryItem,
    ObjectId, buildingId, InvenType, invenType)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->removeAllBuildingInventoryItem(buildingId, invenType)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_addBuildingGuard,
    ObjectId, buildingId, BuildingGuardInfo, info)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->addBuildingGuard(buildingId, info)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_removeBuildingGuard,
    ObjectId, guardId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->removeBuildingGuard(guardId)) {
        // TODO: 로그
    }
}



RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_createBuilding,
    BuildingInfo, buildingInfo)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->createBuilding(buildingInfo)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_1(DatabaseProxyServerSideRpcExtension, s2d_deleteBuilding,
    ObjectId, buildingId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->removeBuilding(buildingId)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_updateBuildingState,
    ObjectId, buildingId, BuildingStateType, state, sec_t, startBuildTime,
    sec_t, expireTime, HitPoint, currentHitPoint)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->updateBuildingState(buildingId, state, startBuildTime, expireTime, currentHitPoint)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateBuildingOwner,
    ObjectId, buildingId, BuildingOwnerType, ownerType, ObjectId, playerId, GuildId, guildId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->updateBuildingOwner(buildingId, ownerType, playerId, guildId)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addBindRecallInfo,
    AccountId, accountId, ObjectId, characterId, BindRecallInfo, bindRecallInfo)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->addBindRecallInfo(characterId, bindRecallInfo);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_removeBindRecallIInfo,
    AccountId, accountId, ObjectId, characterId, ObjectId, linkId)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->removeBindRecallInfo(characterId, linkId);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_addSelectRecipeProduction,
    ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->addSelectRecipeProduction(buildingId, recipeCode, count, completeTime)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeSelectRecipeProduction,
    ObjectId, buildingId, RecipeCode, recipeCode)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->removeSelectRecipeProduction(buildingId, recipeCode)) {
        // TODO: 로그
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateSelectRecipeProduction,
    ObjectId, buildingId, RecipeCode, recipeCode, uint8_t, count, sec_t, completeTime)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->updateSelectRecipeProduction(buildingId, recipeCode, count, completeTime)) {
        // TODO: 로그
    }
}

RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_releaseBeginnerProtection,
    AccountId, accountId, ObjectId, characterId)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->releaseBeginnerProtection(characterId);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_loadBuildingInfo,
    RequestId, requestId, ZoneId, zoneId)
{
    ObjectIdSet deleteBuilds; 
    ObjectIdSet deleteItems;
    ObjectId maxZoneBuildingId = invalidObjectId;
    BuildingInfoMap buildingInfoMap;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

        if (! db->getBuildingInfos(deleteItems, deleteBuilds, maxZoneBuildingId, buildingInfoMap, zoneId)) {
            s2d_onLoadBuildingInfo(requestId, ecDatabaseInternalError, invalidObjectId, BuildingInfoMap());
            return;
        }
        for (ObjectIdSet::value_type value : deleteBuilds) {
            db->removeBuilding(value);
        }

        for (ObjectIdSet::value_type value : deleteItems) {
            db->removeBuildingInventoryItem(value);
        }
    }
    s2d_onLoadBuildingInfo(requestId, ecOk, maxZoneBuildingId, buildingInfoMap);
}


FORWARD_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_onLoadBuildingInfo,
    RequestId, requestId, ErrorCode, errorCode, ObjectId, maxBuildingId,
    BuildingInfoMap, buildingInfoMap);


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addCooldownInfos,
    AccountId, accountId, ObjectId, characterId, CooltimeInfos, infos)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->addCooldownInfos(characterId, infos);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addRemainEffects,
    AccountId, accountId, ObjectId, characterId, RemainEffectInfos, infos)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->addRemainEffects(characterId, infos);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_5(DatabaseProxyServerSideRpcExtension, s2d_updateCharacterInventoryInfo,
    AccountId, accountId, ObjectId, characterId, InvenType, invenType,
    bool, isCashSlot, uint8_t, count)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->updateCharacterInventoryInfo(characterId, invenType, isCashSlot, count);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_createVehicle,
	AccountId, accountId, ObjectId, characterId, VehicleInfo, info)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->createVehicle(characterId, info);        
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_createGlider,
	AccountId, accountId, ObjectId, characterId, GliderInfo, info)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->createGlider(characterId, info);        
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_deleteVehicle,
	AccountId, accountId, ObjectId, characterId, ObjectId, id)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->deleteVehicle(characterId, id);        
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_deleteGlider,
	AccountId, accountId, ObjectId, characterId, ObjectId, id)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->deleteGlider(characterId, id);        
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_selectVehicle,
	AccountId, accountId, ObjectId, characterId, ObjectId, id)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->selectVehicle(characterId, id);        
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_selectGlider,
	AccountId, accountId, ObjectId, characterId, ObjectId, id)
{
	AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
	if (cache != nullptr) {
		cache->selectGlider(characterId, id);        
	}
	else {
		SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
			accountId);
	}
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateGliderDurability,
    AccountId, accountId, ObjectId, characterId, ObjectId, id, uint32_t, currentValue)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->updateGliderDurability(characterId, id, currentValue);
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_getBuddies,
    AccountId, accountId, ObjectId, characterId)
{
    BuddyInfos buddyInfos;
    BlockInfos blockInfos;
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (!db->getBuddies(buddyInfos, blockInfos, characterId)) {
        SNE_LOG_ERROR("s2d_getBuddies(A%" PRIu64 ",C%" PRIu64 ") db failed - " __FUNCTION__,
            accountId, characterId);
    }
    else {
        s2d_onGetBuddies(accountId, buddyInfos, blockInfos);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_addBuddy,
	ObjectId, characterId1, ObjectId, characterId2)
{
	sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
	if (! db->addBuddy(characterId1, characterId2)) {
		SNE_LOG_ERROR("s2d_addBuddy(C%" PRIu64 ", C%" PRIu64 ") db failed - " __FUNCTION__,
			characterId1, characterId2);
	}
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeBuddy,
	ObjectId, characterId1, ObjectId, characterId2)
{
	sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
	if (! db->removeBuddy(characterId1, characterId2)) {
		SNE_LOG_ERROR("s2d_removeBuddy(%" PRIu64 ", %" PRIu64 ") db failed - " __FUNCTION__,
			characterId1, characterId2);
	}
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_addBlock,
    ObjectId, ownerId, ObjectId, characterId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->addBlock(ownerId, characterId)) {
        SNE_LOG_ERROR("s2d_addBlock(C%" PRIu64 ", C%" PRIu64 ") db failed - " __FUNCTION__,
            ownerId, characterId);
    }
}


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_removeBlock,
    ObjectId, ownerId, ObjectId, characterId)
{
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->removeBlock(ownerId, characterId)) {
        SNE_LOG_ERROR("s2d_removeBlock(C%" PRIu64 ", C%" PRIu64 ") db failed - " __FUNCTION__,
            ownerId, characterId);
    }
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_onGetBuddies,
	AccountId, accountId, BuddyInfos, buddyInfos, BlockInfos, blockInfos);


RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension, s2d_queryAchievements,
    AccountId, accountId, ObjectId, characterId)
{
    ErrorCode errorCode = ecDatabaseInternalError;
    ProcessAchievementInfoMap processInfoMap;
    CompleteAchievementInfoMap completeInfoMap;

    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        errorCode = cache->queryAchievements(processInfoMap, completeInfoMap, characterId);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }

    s2d_onQueryAchievements(errorCode, characterId, processInfoMap, completeInfoMap);
}


FORWARD_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_onQueryAchievements,
    ErrorCode, errorCode, ObjectId, characterId, ProcessAchievementInfoMap, processInfoMap,
    CompleteAchievementInfoMap, completeInfoMap);


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_updateProcessAchievement,
    AccountId, accountId, ObjectId, characterId, AchievementCode, code, ProcessAchievementInfo, missionInfo)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->updateProcessAchievement(characterId, code, missionInfo);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_4(DatabaseProxyServerSideRpcExtension, s2d_completeAchievement,
    AccountId, accountId, ObjectId, characterId, AchievementCode, code, AchievementPoint, point)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->completeAchievement(characterId, code, point);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}


RECEIVE_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension, s2d_addCharacterTitle,
    AccountId, accountId, ObjectId, characterId, CharacterTitleCode, titleCode)
{
    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        cache->addCharacterTitle(characterId, titleCode);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }
}



RECEIVE_SRPC_METHOD_2(DatabaseProxyServerSideRpcExtension,s2d_queryCharacterTitles,
    AccountId, accountId, ObjectId, characterId)
{
    ErrorCode errorCode = ecDatabaseInternalError;
    CharacterTitleCodeSet titleCodeSet;

    AccountCache* cache = ACCOUNTCACHE_MANAGER->loadCache(accountId);
    if (cache != nullptr) {
        errorCode = cache->queryCharacterTitles(titleCodeSet, characterId);        
    }
    else {
        SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") not found - " __FUNCTION__,
            accountId);
    }

    s2d_onQueryCharacterTitles(errorCode, characterId, titleCodeSet);
}


FORWARD_SRPC_METHOD_3(DatabaseProxyServerSideRpcExtension,s2d_onQueryCharacterTitles,
    ErrorCode, errorCode, ObjectId, characterId, CharacterTitleCodeSet, titleCodeSet);

}} // namespace gideon { namespace databaseproxyserver {
