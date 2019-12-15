#include "DatabaseProxyServerPCH.h"
#include "AccountCacheImpl.h"
#include <gideon/serverbase/database/GameDatabase.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/CharacterDefaultSkillTable.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace databaseproxyserver {

namespace {


ErrorCode fillDefaultSkills(datatable::DefaultSkillInfos& defaultSkillInfos, EquipType& equipType, EquipCode mainWeaponCode)
{
    const gdt::equip_t* equip = EQUIP_TABLE->getEquip(mainWeaponCode);
    if (! equip) {
        return ecItemInvalidEquiptItem;
    }

    equipType = toEquipType(equip->equip_type());
    const datatable::CharacterDefaultSkillInfo* defaultSkillInfo = 
        CHARACTER_DEFAULT_SKILL_TABLE->getCharacterDefaultSkillInfo(equipType);
    if (! defaultSkillInfo) {
        return ecServerInternalError;
    }

    defaultSkillInfos = defaultSkillInfo->getCharacterDefaultSkills();
    return ecOk;
}

void createCharacterSetPassiveSkill(FullCharacterInfo& characterInfo)
{
    characterInfo;
    /*for (SkillCode skillCode : characterInfo.skills_) {
        const datatable::SkillTemplate* skill = SKILL_TABLE->getSkill(skillCode);
        if (! skill) {
            assert(false);
            continue;
        }

        const datatable::SkillCommonInfo& commonInfo = skill->getSkillCommonInfo();
        if ( commonInfo.coolTime_ == 0 && commonInfo.globalCooldownTime_ == 0 && skill->getUseType() == utPassive) {
            for (int i = eanStart; i < eanCount; ++i) {
                const datatable::BaseEffectInfo& casteeInfo = skill->getCasterEffectInfo(toEffectApplyNum(i));
                if (casteeInfo.isValid()) {
                    if (casteeInfo.scriptType_ == estHpCapacityUp && ! casteeInfo.isUseEffectValueByPercent() == 0) {
                        characterInfo.currentPoints_.hp_ = toHitPoint(characterInfo.currentPoints_.hp_ + casteeInfo.effectValue_);
                    }
                    if (casteeInfo.scriptType_ == estMpCapacityUp && ! casteeInfo.isUseEffectValueByPercent()) {
                        characterInfo.currentPoints_.mp_ = toManaPoint(characterInfo.currentPoints_.mp_ + casteeInfo.effectValue_);
                    }
                }
            }                                       
        }
    }*/
}


} // namespace {


AccountCacheImpl::AccountCacheImpl()
{
    reset();
}


AccountCacheImpl::~AccountCacheImpl()
{
}

// = AccountCache overriding

ErrorCode AccountCacheImpl::initialize(AccountId accountId)
{
    FullUserInfo userInfo;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

        if (! db->getFullUserInfo(userInfo, accountId)) {
            return ecDatabaseInternalError;
        }

        {
            for (FullCharacterInfos::value_type& value : userInfo.characters_) {
                FullCharacterInfo& info = value.second;
                if (!db->getQuests(info.objectId_, info.completedQuests_, info.acceptedQuestInfos_)) {
                    return ecDatabaseInternalError;
                }
            }
        }

        {
            for (FullCharacterInfos::value_type& value : userInfo.characters_) {
                FullCharacterInfo& info = value.second;
                QuestCodes removeCompleteRepeatQuestCodes;
                if (!db->getRepeatQuests(info.objectId_, info.completedRepeatQuestInfos_,
                    info.acceptedRepeatQuestInfos_, removeCompleteRepeatQuestCodes)) {
                    return ecDatabaseInternalError;
                }
                for (const QuestCodes::value_type& qcValue : removeCompleteRepeatQuestCodes) {
                    db->removeCompleteRepeatQuest(info.objectId_, qcValue);
                }
            }
        }

        {
            for (FullCharacterInfos::value_type& value : userInfo.characters_) {
                FullCharacterInfo& info = value.second;
                if (!db->getCharacterArenaRecords(info.arenaPlayResults_, info.objectId_)) {
                    return ecDatabaseInternalError;
                }
            }
        }

        {
            for (FullCharacterInfos::value_type& value : userInfo.characters_) {
                const ObjectId characterId = value.first;
                Property property;
                if (db->loadCharacterProperties(property.config_, property.prefs_, characterId)) {
                    propertyMap_[characterId] = property;
                }
            }
        }
    }

    {
        std::unique_lock<LockType> lock(lock_);

        userInfo_ = userInfo;
    }

    touched();
    return ecOk;
}


void AccountCacheImpl::finalize()
{
    reset();
}


void AccountCacheImpl::removeOldCompleteRepeatQuest()
{
    QuestCodes deleteQuestCodes;
    {
        time_t now = time(0);
        struct tm* nowInfo = localtime(&now);

        std::unique_lock<LockType> lock(lock_);

        for (FullCharacterInfos::value_type& fcValue : userInfo_.characters_) {
            FullCharacterInfo& info = fcValue.second;	
            for (const CompletedRepeatQuestInfos::value_type& rqValue : info.completedRepeatQuestInfos_) {
                const time_t questAcceptTime = static_cast<time_t>(rqValue.acceptTime_);
                struct tm* acceptInfo = localtime(&questAcceptTime);
                if (! nowInfo || ! acceptInfo) {
                    // TODO 로그
                    continue;
                }
                if (nowInfo->tm_yday == acceptInfo->tm_yday) {
                    continue;   
                }
                deleteQuestCodes.insert(rqValue.questCode_);
            }   
            
            for (const QuestCodes::value_type& qcValue : deleteQuestCodes) {
                info.removeCompleteRepeatQuest(qcValue);
                sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
                db->removeCompleteRepeatQuest(info.objectId_, qcValue);
            }
        }
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);

}


ErrorCode AccountCacheImpl::createCharacter(FullCharacterInfo& characterInfo,
    const CreateCharacterInfo& createCharacterInfo, const CreateCharacterEquipments& createCharacterEquipments,
    ZoneId zoneId, const ObjectPosition& position)
{
    characterInfo.zoneId_ = zoneId;
    datatable::DefaultSkillInfos defaultSkillInfos;
    EquipType equipType = EquipType::etUnknown;
    ErrorCode errorCode = fillDefaultSkills(defaultSkillInfos, equipType, createCharacterInfo.selectMainWeaponCode_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    ObjectId characterId = invalidObjectId;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        errorCode = db->createCharacter(characterId,
            userInfo_.accountId_, createCharacterInfo.nickname_, equipType,
            ccAny, createCharacterInfo.sexType_,
            createCharacterEquipments, createCharacterInfo.appearance_, zoneId, position);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    
        for (const datatable::DefaultSkillInfo& skillInfo : defaultSkillInfos) {
            bool result = db->learnSkill(userInfo_.accountId_, characterId,
                invalidSkillCode, skillInfo.skillCode_);
            if (! result) {
                return ecDatabaseInternalError;
            }
            if (isValid(skillInfo.position_)) {
                result = db->saveActionBar(characterId, abcStartIndex, skillInfo.position_, skillInfo.skillCode_);
                if (! result) {
                    return ecDatabaseInternalError;
                }
            }
        }

        if (! db->getCharacterInfo(characterInfo, characterId)) {
            return ecDatabaseInternalError;
        }

        if (! db->getQuests(characterId, characterInfo.completedQuests_, characterInfo.acceptedQuestInfos_)) {
            return ecDatabaseInternalError;
        }
    }

    {
        std::unique_lock<LockType> lock(lock_);

        assert(! userInfo_.getCharacter(characterInfo.objectId_));
        
        userInfo_.characters_.insert(
            FullCharacterInfos::value_type(characterInfo.objectId_, characterInfo));
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ",%W,%d) created.",
        userInfo_.accountId_, characterInfo.objectId_, createCharacterInfo.nickname_.c_str(),
        zoneId);

    return ecOk;
}


ErrorCode AccountCacheImpl::deleteCharacter(GuildId& guildId, ObjectId characterId)
{
    ErrorCode errorCode = ecOk;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        errorCode = db->deleteCharacter(guildId, characterId);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }

    {
        std::unique_lock<LockType> lock(lock_);

        userInfo_.characters_.erase(characterId);
        propertyMap_.erase(characterId);
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") deleted.",
        userInfo_.accountId_, characterId);

    return ecOk;
}


void AccountCacheImpl::saveCharacterStats(ObjectId characterId,
    const DBCharacterStats& saveInfo)
{
    {
        std::unique_lock<LockType> lock(lock_);

        userInfo_.lastAccessCharacterId_ = characterId;

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->currentLevel_.level_ = saveInfo.level_;
        characterInfo->currentLevel_.levelStep_= saveInfo.levelStep_;
        characterInfo->currentPoints_ = saveInfo.currentPoints_;
        characterInfo->stateInfo_.states_ = saveInfo.state_;
        characterInfo->position_ = static_cast<const ObjectPosition&>(saveInfo.position_);
        characterInfo->exp_ = saveInfo.exp_;
        characterInfo->sp_ = saveInfo.sp_;
        characterInfo->zoneId_ = saveInfo.zoneId_;
        characterInfo->gameMoney_ = saveInfo.gameMoney_;
        characterInfo->eventCoin_ = saveInfo.eventCoin_;
        characterInfo->bankAccountInfo_.gameMoney_ = saveInfo.bankGameMoney_;
        characterInfo->actionBars_.isActionBarLock_ = saveInfo.isActionBarLocked_;
        characterInfo->chaotic_ = saveInfo.chaotic_;
        characterInfo->characterClass_ = saveInfo.cc_; 
        characterInfo->arenaPoint_ = saveInfo.arenaPoint_;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->saveCharacterStats(userInfo_.accountId_, characterId, saveInfo)) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - Failed to save character(C%" PRIu64 ") stats.",
                userInfo_.accountId_, characterId);
            return;
        }
    }
    
    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") stats saved.",
        userInfo_.accountId_, characterId);
}


void AccountCacheImpl::saveSelectCharacterTitle(ObjectId characterId, CharacterTitleCode titileCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        userInfo_.lastAccessCharacterId_ = characterId;

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->selectTitleCode_ = titileCode;
    }
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->saveSelectCharacterTitle(characterId, titileCode)) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - Failed to save select character(C%" PRIu64 ") title(C%" PRIu64 ") .",
                userInfo_.accountId_, characterId, titileCode);
            return;
        }
    }
}


ErrorCode AccountCacheImpl::getCharacterProperties(std::string& config, std::string& prefs,
    ObjectId characterId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        const PropertyMap::const_iterator pos = propertyMap_.find(characterId);
        if (pos != propertyMap_.end()) {
            const Property& property = (*pos).second;
            config = property.config_;
            prefs = property.prefs_;
            return ecOk;
        }
    }

    Property property;
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->loadCharacterProperties(property.config_, property.prefs_, characterId)) {
            return ecDatabaseInternalError;
        }
    }
    config = property.config_;
    prefs = property.prefs_;

    {
        std::unique_lock<LockType> lock(lock_);

        propertyMap_[characterId] = property;
    }

    return ecOk;
}


void AccountCacheImpl::saveCharacterProperties(ObjectId characterId,
    const std::string& config, const std::string& prefs)
{
    {
        std::unique_lock<LockType> lock(lock_);

        propertyMap_[characterId] = Property(config, prefs);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->saveCharacterProperties(userInfo_.accountId_, characterId, config, prefs)) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - Failed to save character(C%" PRIu64 ") properties.",
                userInfo_.accountId_, characterId);
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") properties saved.",
        userInfo_.accountId_, characterId);
}


ObjectIdSet AccountCacheImpl::getCharacterIds() const
{
    std::unique_lock<LockType> lock(lock_);

    ObjectIdSet objectIds;
    for (const FullCharacterInfos::value_type& value : userInfo_.characters_) {
        objectIds.insert(value.first);
    }
    return objectIds;
}


void AccountCacheImpl::moveInventoryItem(ObjectId characterId,
    ObjectId itemId, SlotId slotId, InvenType invenType)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        
        ItemInfo* itemInfo = nullptr;
        if (invenType == itPc) {
            assert(characterInfo->inventory_.isStorable(slotId));
            itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        }
        else if (invenType == itBank) {
            assert(characterInfo->bankAccountInfo_.isStorable(slotId));
            itemInfo = characterInfo->bankAccountInfo_.getItemInfo(itemId);
        }
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        assert(! itemInfo->isEquipped());

        itemInfo->setSlotId(slotId);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->moveInventoryItem(itemId, slotId);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") move item(I%" PRIu64 ").",
        userInfo_.accountId_, characterId, itemId);
}


void AccountCacheImpl::switchInventoryItem(ObjectId characterId,
    ObjectId itemId1, ObjectId itemId2, InvenType invenType)
{
    SlotId slotId1 = invalidSlotId;
    SlotId slotId2 = invalidSlotId;

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        ItemInfo* itemInfo1 = nullptr;
        ItemInfo* itemInfo2 = nullptr;

        if (invenType == itPc) {
            itemInfo1 = characterInfo->inventory_.getItemInfo(itemId1);
            itemInfo2 = characterInfo->inventory_.getItemInfo(itemId2);
        }
        else if (invenType == itBank) {
            itemInfo1 = characterInfo->bankAccountInfo_.getItemInfo(itemId1);
            itemInfo2 = characterInfo->bankAccountInfo_.getItemInfo(itemId2);
        }

        if ((! itemInfo1) || (! itemInfo2)) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        assert(! itemInfo1->isEquipped());
        assert(! itemInfo2->isEquipped());

        std::swap(itemInfo1->slotId_, itemInfo2->slotId_);

        slotId1 = itemInfo1->slotId_;
        slotId2 = itemInfo2->slotId_;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result =
            db->switchInventoryItem(itemId1, slotId1, itemId2, slotId2);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") switch item(I%" PRIu64 ", I%" PRIu64 ").",
        userInfo_.accountId_, characterId, itemId1, itemId2);
}

void AccountCacheImpl::addInventoryItem(ObjectId characterId, const ItemInfo& itemInfo, InvenType invenType)
{
    if (! itemInfo.isValid()) {
        // TODO: 로그를 남기고 유저의 접속을 해제한다
        return;
    }

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }


        if (invenType == itPc) {
            if (characterInfo->inventory_.isOccupiedSlot(itemInfo.slotId_)) {
                // TODO: 로그를 남기고 유저의 접속을 해제한다
                return;
            }

            characterInfo->inventory_.addItem(itemInfo);
        }
        else if (invenType == itBank) {
            if (characterInfo->bankAccountInfo_.isOccupiedSlot(itemInfo.slotId_)) {
                // TODO: 로그를 남기고 유저의 접속을 해제한다
                return;
            }

            characterInfo->bankAccountInfo_.addItem(itemInfo);
        }

        
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (itemInfo.isEquipment()) {
            if (! db->addInventoryEquipItem(userInfo_.accountId_, characterId, itemInfo, invenType)) {
                // TODO: 로그를 남기고 유저의 접속을 해제한다
                return;
            }

            for (const EquipSocketInfoMap::value_type& value : itemInfo.equipItemInfo_.equipSocketInfoMap_) {            
                if (! db->addEquipSocketOption(userInfo_.accountId_, characterId, itemInfo.itemId_, value.first, value.second)) {
                    // TODO: 로그를 남기고 유저의 접속을 해제한다
                    return;
                }
            }
        }
        else if (itemInfo.isAccessory()) {
            if (! db->addInventoryAccessoryItem(userInfo_.accountId_, characterId, itemInfo, invenType)) {
                // TODO: 로그를 남기고 유저의 접속을 해제한다
                return;
            }
        }
        else {
            if (! db->addInventoryItem(userInfo_.accountId_, characterId, itemInfo, invenType)) {
                // TODO: 로그를 남기고 유저의 접속을 해제한다
                return;
            }
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") add item(I%" PRIu64 ", %u, %d, %d).",
        userInfo_.accountId_, characterId, itemInfo.itemId_, itemInfo.itemCode_,
        itemInfo.slotId_, itemInfo.count_);
}


void AccountCacheImpl::changeEquipItemInfo(ObjectId characterId, ObjectId itemId, EquipCode newEquipCode, uint8_t socketCount)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        ItemInfo* itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        itemInfo->itemCode_ = newEquipCode;        
        itemInfo->equipItemInfo_.socketCount_ = socketCount;
    }
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->changeEquipItemInfo(itemId, newEquipCode, socketCount);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") change equipItemInfo(I%" PRIu64 ", %u, %d, %d).",
        userInfo_.accountId_, characterId, itemId, newEquipCode);
}

void AccountCacheImpl::enchantEquipItem(ObjectId characterId, ObjectId itemId, SocketSlotId slotId, 
    const EquipSocketInfo& socketInfo)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        ItemInfo* itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        itemInfo->equipItemInfo_.addGemItem(slotId, socketInfo);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->addEquipSocketOption(userInfo_.accountId_, characterId, itemId, slotId, socketInfo)) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }
}


void AccountCacheImpl::unenchantEquipItem(ObjectId characterId, ObjectId itemId, SocketSlotId slotId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        ItemInfo* itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        itemInfo->equipItemInfo_.removeGemItem(slotId);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->removeEquipSocketOption(itemId, slotId)) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }
}


void AccountCacheImpl::removeInventoryItem(ObjectId characterId,ObjectId itemId, InvenType invenType)
{
    DataCode itemCode = 0;

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        ItemInfo* itemInfo = nullptr;
        if (invenType == itPc) {
            itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        }
        else if (invenType == itBank) {
            itemInfo = characterInfo->bankAccountInfo_.getItemInfo(itemId);
        }
        
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        itemCode = itemInfo->itemCode_;

        if (invenType == itPc) {
            characterInfo->inventory_.removeItem(itemId);
        }
        else if (invenType == itBank) {
            characterInfo->bankAccountInfo_.removeItem(itemId);
        }

    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result =
            db->removeInventoryItem(itemId);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") remove item(I%" PRIu64 ", %u).",
        userInfo_.accountId_, characterId, itemId, itemCode);
}


void AccountCacheImpl::updateInventoryItemCount(ObjectId characterId,
    ObjectId itemId, uint8_t itemCount, InvenType invenType)
{
    DataCode itemCode = 0;

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        ItemInfo* itemInfo = nullptr;
        if (invenType == itPc) {
            itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        }
        else if (invenType == itBank) {
            itemInfo = characterInfo->bankAccountInfo_.getItemInfo(itemId);
        }

        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        itemCode = itemInfo->itemCode_;
        itemInfo->count_ = itemCount;
    }
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result =
            db->updateInventoryItemCount(itemId, itemCount);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") update itemCount(I%" PRIu64 ", %u, I%d).",
        userInfo_.accountId_, characterId, itemId, itemCode, itemCount);
}


void AccountCacheImpl::addQuestItem(ObjectId characterId, const QuestItemInfo& questItemInfo)
{
    if (! questItemInfo.isValid()) {
        // TODO: 로그를 남기고 유저의 접속을 해제한다
        return;
    }

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->questInventory_.addItem(questItemInfo);       
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result =
            db->addQuestItem(userInfo_.accountId_, characterId, questItemInfo);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") add Questitem(I%" PRIu64 ", %u, %d, Q%u).",
        userInfo_.accountId_, characterId, questItemInfo.itemId_, questItemInfo.questItemCode_,
        questItemInfo.usableCount_, questItemInfo.questCode_);
}


void AccountCacheImpl::removeQuestItem(ObjectId characterId, ObjectId itemId)
{
    DataCode itemCode = 0;

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        QuestItemInfo* itemInfo = characterInfo->questInventory_.getItemInfoById(itemId);
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        itemCode = itemInfo->questItemCode_;
        characterInfo->questInventory_.removeItem(itemId);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result =
            db->removeQuestItem(itemId);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") remove item(I%" PRIu64 ", %u).",
        userInfo_.accountId_, characterId, itemId, itemCode);
}


void AccountCacheImpl::updateQuestItemUsableCount(ObjectId characterId, ObjectId itemId, uint8_t usableCount)
{
    DataCode itemCode = 0;

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        QuestItemInfo* itemInfo = characterInfo->questInventory_.getItemInfoById(itemId);
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        itemCode = itemInfo->questItemCode_;
        itemInfo->usableCount_ = usableCount;
    }
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result =
            db->updateQuestItemUsableCount(itemId, usableCount);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") update UsableCount QuestItem(I%" PRIu64 ", %u, %d).",
        userInfo_.accountId_, characterId, itemId, itemCode, usableCount);
}


void AccountCacheImpl::updateQuestItemCount(ObjectId characterId, ObjectId itemId, uint8_t stackCount)
{
    DataCode itemCode = 0;

    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        QuestItemInfo* itemInfo = characterInfo->questInventory_.getItemInfoById(itemId);
        if (! itemInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        itemCode = itemInfo->questItemCode_;
        itemInfo->stackCount_ = stackCount;
    }
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result =
            db->updateQuestItemCount(itemId, stackCount);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") update stackCount QuestItem(I%" PRIu64 ", %u, %d).",
        userInfo_.accountId_, characterId, itemId, itemCode, stackCount);
}


void AccountCacheImpl::equipAccessoryItem(ObjectId characterId,
    ObjectId itemId, AccessoryIndex equipPart)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find character(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, equipPart);
            return;
        }

        ItemInfo* itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find item(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, equipPart);
            return;
        }

        assert(! itemInfo->isEquipped());
        assert(! isValidObjectId(characterInfo->equipments_[equipPart]));

        itemInfo->setSlotId(equippedSlotId);
        characterInfo->characterAccessories_[equipPart] = itemId;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->equipAccessoryItem(userInfo_.accountId_,
            characterId, itemId, equipPart);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equip item(I%" PRIu64 ").",
        userInfo_.accountId_, characterId, itemId);
}


void AccountCacheImpl::unequipAccessoryItem(ObjectId characterId,
    ObjectId itemId, SlotId slotId, AccessoryIndex unequipPart)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip item error not find character(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, unequipPart);
            return;
        }

        ItemInfo* itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip item error not find accessoryitem(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, unequipPart);
            return;
        }
        assert(itemInfo->isEquipped());
        if (! isValidObjectId(characterInfo->characterAccessories_[unequipPart])) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip item error invalid accessoryequip item(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, unequipPart);
        }

        itemInfo->setSlotId(slotId);
        characterInfo->characterAccessories_[unequipPart] = invalidObjectId;        
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->unequipAccessoryItem(userInfo_.accountId_,
            characterId, itemId, slotId, unequipPart);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip accessoryitem(I%" PRIu64 ").",
        userInfo_.accountId_, characterId, itemId);
}


void AccountCacheImpl::replaceInventoryWithAccessoryItem(ObjectId characterId,
    ObjectId unequipItemId, AccessoryIndex unequipPart, ObjectId equipItemId,
    AccessoryIndex equipPart)
{
    SlotId slotId = invalidSlotId;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find character(I%" PRIu64 ", %d, I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, unequipItemId, unequipPart, equipItemId, equipPart);
            return;
        }

        ItemInfo* equipToInvenItem = characterInfo->inventory_.getItemInfo(unequipItemId);
        ItemInfo* invenToEquipItem = characterInfo->inventory_.getItemInfo(equipItemId);
        if ((! equipToInvenItem) || (! invenToEquipItem)) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find item(I%" PRIu64 ", %d, I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, unequipItemId, unequipPart, equipItemId, equipPart); 
            return;
        }

        assert(equipToInvenItem->isEquipped());
        assert(! invenToEquipItem->isEquipped());

        slotId = invenToEquipItem->slotId_;

        
        characterInfo->characterAccessories_[unequipPart] = invalidObjectId;
        characterInfo->characterAccessories_[equipPart] = equipItemId;
        equipToInvenItem->setSlotId(slotId);
        invenToEquipItem->setSlotId(equippedSlotId);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->replaceInventoryWithAccessoryItem(userInfo_.accountId_,
            characterId, unequipItemId, unequipPart, slotId, equipItemId, equipPart);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") replace inventory with accessoryItem (I%" PRIu64 ", I%" PRIu64 ").",
        userInfo_.accountId_, characterId, unequipItemId, equipItemId);
}


void AccountCacheImpl::equipItem(ObjectId characterId,
    ObjectId itemId, EquipPart equipPart)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find character(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, equipPart);
            return;
        }

        ItemInfo* itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find item(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, equipPart);
            return;
        }

        assert(! itemInfo->isEquipped());
        assert(! isValidObjectId(characterInfo->equipments_[equipPart]));

        itemInfo->setSlotId(equippedSlotId);
        characterInfo->equipments_[equipPart] = itemId;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->equipItem(userInfo_.accountId_,
            characterId, itemId, equipPart);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equip item(I%" PRIu64 ").",
        userInfo_.accountId_, characterId, itemId);
}


void AccountCacheImpl::unequipItem(ObjectId characterId,
    ObjectId itemId, SlotId slotId, EquipPart unequipPart)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip item error not find character(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, unequipPart);
            return;
        }

        ItemInfo* itemInfo = characterInfo->inventory_.getItemInfo(itemId);
        if (! itemInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip item error not find item(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, unequipPart);
            return;
        }
        assert(itemInfo->isEquipped());
        if (! isValidObjectId(characterInfo->equipments_[unequipPart])) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip item error invalid equip item(I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, itemId, unequipPart);
        }

        itemInfo->setSlotId(slotId);
        characterInfo->equipments_[unequipPart] = invalidObjectId;        
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->unequipItem(userInfo_.accountId_,
            characterId, itemId, slotId, unequipPart);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") unequip item(I%" PRIu64 ").",
        userInfo_.accountId_, characterId, itemId);
}


void AccountCacheImpl::replaceInventoryWithEquipItem(ObjectId characterId,
    ObjectId unequipItemId, EquipPart unequipPart, ObjectId equipItemId,
    EquipPart equipPart)
{
    SlotId slotId = invalidSlotId;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find character(I%" PRIu64 ", %d, I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, unequipItemId, unequipPart, equipItemId, equipPart);
            return;
        }

        ItemInfo* equipToInvenItem = characterInfo->inventory_.getItemInfo(unequipItemId);
        ItemInfo* invenToEquipItem = characterInfo->inventory_.getItemInfo(equipItemId);
        if ((! equipToInvenItem) || (! invenToEquipItem)) {
            SNE_LOG_ERROR("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") equipItem item error not find item(I%" PRIu64 ", %d, I%" PRIu64 ", %d).",
                userInfo_.accountId_, characterId, unequipItemId, unequipPart, equipItemId, equipPart); 
            return;
        }

        assert(equipToInvenItem->isEquipped());
        assert(! invenToEquipItem->isEquipped());

        slotId = invenToEquipItem->slotId_;

        
        characterInfo->equipments_[unequipPart] = invalidObjectId;
        characterInfo->equipments_[equipPart] = equipItemId;
        equipToInvenItem->setSlotId(slotId);
        invenToEquipItem->setSlotId(equippedSlotId);

        //CreatureStatusInfo* CreatureStatusInfo = getCurrentCreatureStatusInfo(characterId);
        /*EQUIP_TABLE->applyCreatureStatusInfoByUnequip(*CreatureStatusInfo, characterInfo->currentPoints_,
            equipToInvenItem->itemCode_);
        EQUIP_TABLE->applyCreatureStatusInfoByEquip(*CreatureStatusInfo, characterInfo->currentPoints_,
            invenToEquipItem->itemCode_);*/
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->replaceInventoryWithEquipItem(userInfo_.accountId_,
            characterId, unequipItemId, unequipPart, slotId, equipItemId, equipPart);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") replace inventory with equipItem (I%" PRIu64 ", I%" PRIu64 ").",
        userInfo_.accountId_, characterId, unequipItemId, equipItemId);
}

void AccountCacheImpl::changeCharacterState(ObjectId characterId,
    CreatureStateType state)
{
    {
        std::unique_lock<LockType> lock(lock_);
        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->stateInfo_.states_ = state;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->changeCharacterState(characterId, state);
        if (! result) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") change character state(I%" PRIu64 ").",
        userInfo_.accountId_, characterId, state);
}


void AccountCacheImpl::saveActionBar(ObjectId characterId, 
    ActionBarIndex abiIndex, ActionBarPosition abpIndex, DataCode code)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->actionBars_.save(abiIndex, abpIndex, code);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->saveActionBar(characterId, abiIndex, abpIndex, code);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") save action bar state(%d, %d, %d).",
        userInfo_.accountId_, characterId, abiIndex, abpIndex, code);
}


void AccountCacheImpl::learnSkill(ObjectId characterId, 
    SkillCode currentSkillCode, SkillCode learnSkillCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->skills_.erase(currentSkillCode);
        characterInfo->skills_.insert(learnSkillCode);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->learnSkill(userInfo_.accountId_, characterId, currentSkillCode, learnSkillCode);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") learn skill(%u, %u).",
        userInfo_.accountId_, characterId, currentSkillCode, learnSkillCode);
}


void AccountCacheImpl::removeSkill(ObjectId characterId, SkillCode skillCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->skills_.erase(skillCode);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->removeSkill(characterId, skillCode);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") remove skill(%u).",
        userInfo_.accountId_, characterId, skillCode);
}


void AccountCacheImpl::removeAllSkill(ObjectId characterId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->skills_.clear();
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->removeAllSkill(characterId);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }

    SNE_LOG_INFO("AccountCache(A%" PRIu64 ") - character(C%" PRIu64 ") remove all skill.",
        userInfo_.accountId_, characterId);
}


void AccountCacheImpl::acceptQuest(ObjectId characterId, QuestCode questCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        
        if (characterInfo->isAcceptedQuest(questCode)) {
            assert(false);
            return;
        }
        
        characterInfo->acceptedQuestInfos_.push_back(AcceptedQuestInfo(questCode));
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->acceptQuest(characterId, questCode);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }
}


void AccountCacheImpl::acceptRepeatQuest(ObjectId characterId, QuestCode questCode, sec_t acceptRepeatTime)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        if (characterInfo->isAcceptRepeatQuest(questCode)) {
            assert(false);
            return;
        }

        CompletedRepeatQuestInfos::iterator pos = characterInfo->completedRepeatQuestInfos_.begin();
        CompletedRepeatQuestInfos::iterator end = characterInfo->completedRepeatQuestInfos_.end();
        for(; pos != end; ++pos) {
            CompletedRepeatQuestInfo info = (*pos);
            if (info.questCode_ == questCode) {
                characterInfo->completedRepeatQuestInfos_.erase(pos);
                break;
            }
        }

        characterInfo->acceptedRepeatQuestInfos_.push_back(AcceptedRepeatQuestInfo(questCode, acceptRepeatTime));
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->acceptRepeatQuest(characterId, questCode, acceptRepeatTime);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }
}


void AccountCacheImpl::cancelQuest(ObjectId characterId, QuestCode questCode, bool isRepeatQuest)
{
    if (isRepeatQuest) {
        cancelRepeatQuest(characterId, questCode);
    }
    else {
        cancelQuest(characterId, questCode);
    }
}


void AccountCacheImpl::completeQuest(ObjectId characterId, QuestCode questCode, bool isRepeatQuest)
{
    if (isRepeatQuest) {
        completeRepeatQuest(characterId, questCode);
    }
    else {
        completeQuest(characterId, questCode);
    }
}


void AccountCacheImpl::updateQuestMission(ObjectId characterId, QuestCode questCode,
    QuestMissionCode missionCode, const QuestGoalInfo& goalInfo, bool isRepeatQuest)
{
    if (isRepeatQuest) {
        updateRepeatQuestMission(characterId, questCode, missionCode, goalInfo);
    }
    else {
        updateQuestMission(characterId, questCode, missionCode, goalInfo);
    }
}


void AccountCacheImpl::removeCompleteQuest(ObjectId characterId, QuestCode questCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->completedQuests_.erase(questCode);
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    const bool result = db->removeCompleteQuest(characterId, questCode);
    if (! result) {
        // TODO: 로그를 남긴다
        return;
    }
}


void AccountCacheImpl::setPlayerGuildInfo(GuildId guildId, const GuildMemberInfo& memberInfo,
    bool dbSave)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(memberInfo.playerId_);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->guildId_ = guildId;
    }

    if (dbSave) {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->addGuildMember(guildId, memberInfo);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }    
}


void AccountCacheImpl::leaveGuild(ObjectId characterId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->guildId_ = invalidGuildId;
    }
}


FullUserInfo AccountCacheImpl::getFullUserInfo() const
{
    std::unique_lock<LockType> lock(lock_);

    return userInfo_;
}


size_t AccountCacheImpl::getCharacterCount() const
{
    std::unique_lock<LockType> lock(lock_);

    return userInfo_.characters_.size();
}


void AccountCacheImpl::cancelQuest(ObjectId characterId, QuestCode questCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->removeQuest(questCode);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->cancelQuest(characterId, questCode);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }
}


void AccountCacheImpl::completeQuest(ObjectId characterId, QuestCode questCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->removeQuest(questCode);
        characterInfo->completedQuests_.insert(questCode);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->completeQuest(characterId, questCode);
        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }
}


void AccountCacheImpl::updateQuestMission(ObjectId characterId, QuestCode questCode,
    QuestMissionCode missionCode, const QuestGoalInfo& goalInfo)
{
    bool hasMission = false;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        if (characterInfo->hasQuestGoalMission(questCode, missionCode, goalInfo)) {
            hasMission = true;	
        }
        characterInfo->updateQuestGoalMission(questCode, missionCode, goalInfo);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        bool result = false;
        if (hasMission) {
            result = db->updateQuestMission(characterId, questCode, missionCode, goalInfo);            
        }
        else {
            result = db->insertQuestMission(characterId, questCode, missionCode, goalInfo);
        }

        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }
}


void AccountCacheImpl::cancelRepeatQuest(ObjectId characterId, QuestCode questCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        characterInfo->removeRepeatQuest(questCode);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        const bool result = db->cancelRepeatQuest(characterId, questCode);
        if (! result) {
            // todo: 로그를 남긴다
            return;
        }
    }
}


void AccountCacheImpl::completeRepeatQuest(ObjectId characterId, QuestCode questCode)
{
    bool isUpdate = false;
    sec_t acceptTime = 0;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        acceptTime = characterInfo->getRepeatQuestAcceptTime(questCode);
        characterInfo->removeRepeatQuest(questCode);
        const CompletedRepeatQuestInfos::iterator pos =
            characterInfo->completedRepeatQuestInfos_.find(CompletedRepeatQuestInfo(questCode));
        if (pos != characterInfo->completedRepeatQuestInfos_.end()) {
            CompletedRepeatQuestInfo& info = const_cast<CompletedRepeatQuestInfo&>(*pos);
            info.acceptTime_ = acceptTime;
            isUpdate = true;
        }
        else {
            characterInfo->completedRepeatQuestInfos_.insert(
                CompletedRepeatQuestInfo(questCode, acceptTime));
        }
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (isUpdate) {
            (void)db->updateCompleteRepeatQuest(characterId, questCode, acceptTime);
        }
        else {
            (void)db->completeRepeatQuest(characterId, questCode, acceptTime);
        }
        
    }
}


void AccountCacheImpl::updateRepeatQuestMission(ObjectId characterId, QuestCode questCode,
    QuestMissionCode missionCode, const QuestGoalInfo& goalInfo)
{
    bool hasMission = false;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        if (characterInfo->hasRepeatQuestGoalMission(questCode, missionCode, goalInfo)) {
            hasMission = true;	
        }
        characterInfo->updateRepeatQuestGoalMission(questCode, missionCode, goalInfo);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        bool result = false;
        if (hasMission) {
            result = db->updateRepeatQuestMission(characterId, questCode, missionCode, goalInfo);            
        }
        else {
            result = db->insertRepeatQuestMission(characterId, questCode, missionCode, goalInfo);
        }

        if (! result) {
            // TODO: 로그를 남긴다
            return;
        }
    }
}

void AccountCacheImpl::updateArenaRecord(ObjectId characterId, ArenaModeType arenaMode,
    uint32_t resultScore, ArenaResultType resultType)
{
    ArenaPlayResult playResult;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->arenaPlayResults_.updateResult(arenaMode, resultScore, resultType);
        playResult = *characterInfo->arenaPlayResults_.getPlayResult(arenaMode);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->updateArenaRecord(characterId, arenaMode, playResult)) {
            // 로그 남긴다
        }
    }
}


void AccountCacheImpl::updateArenaPoint(ObjectId characterId, ArenaPoint arenaPoint)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->arenaPoint_ = arenaPoint;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->updateArenaPoint(characterId, arenaPoint)) {
            // 로그 남긴다
        }
    }
}


void AccountCacheImpl::updateDeserterExpireTime(ObjectId characterId, sec_t deserterExpireTime)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->deserterExpireTime_ = deserterExpireTime;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->updateDeserterExpireTime(characterId, deserterExpireTime)) {
            // 로그 남긴다
        }
    }
}


void AccountCacheImpl::addBindRecallInfo(ObjectId characterId, const BindRecallInfo& bindRecallInfo)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->bindRecallInfos_.push_back(bindRecallInfo);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->addBindRecallInfo(userInfo_.accountId_, characterId, bindRecallInfo)) {
            // 로그 남긴다
        }
    }
}


void AccountCacheImpl::removeBindRecallInfo(ObjectId characterId, ObjectId linkId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->removeBindRecall(linkId);
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->removeBindRecallInfo(linkId)) {
            // 로그 남긴다
        }
    }
}


void AccountCacheImpl::releaseBeginnerProtection(ObjectId characterId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->isBeginnerProtection_ = false;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->releaseBeginnerProtection(characterId)) {
            // 로그 남긴다
        }
    }
}


void AccountCacheImpl::addCooldownInfos(ObjectId characterId, const CooltimeInfos& infos)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->cooltimeInfos_ = infos;
    }

    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        db->removeAllCooldown(characterId);
        for (const CooltimeInfo& info : infos) {
            if (! db->addCooldown(userInfo_.accountId_, characterId, info.dataCode_, info.endTime_)) {
                // 로그 남긴다
            }
        }
    }
}


void AccountCacheImpl::addRemainEffects(ObjectId characterId, const RemainEffectInfos& infos)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        characterInfo->remainEffects_ = infos;       
    }
    {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        db->removeAllRemainEffect(characterId);
        for (const RemainEffectInfo& info : infos) {
            if (! db->addRemainEffect(userInfo_.accountId_, characterId, info)) {
                // 로그 남긴다
            }
        }
    }
}


void AccountCacheImpl::updateCharacterInventoryInfo(ObjectId characterId,
    InvenType invenType, bool isCashSlot, uint8_t count)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        if (itPc == invenType) {
            if (isCashSlot) {
                characterInfo->inventory_.cashSlotCount_ = count;
            }
            else {
                characterInfo->inventory_.freeSlotCount_ = count;
            }
        }
    }

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->updateCharacterInventoryInfo(characterId, invenType, isCashSlot, count)) {
        // TODO 로그
    }
}


void AccountCacheImpl::createVehicle(ObjectId characterId, const VehicleInfo& info)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        if (! characterInfo->vehicleInventory_.add(info)) {
            // TODO: log
            return;
        }
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->createVehicle(userInfo_.accountId_, characterId, info)) {
        // TODO: log
    }
}


void AccountCacheImpl::createGlider(ObjectId characterId, const GliderInfo& info)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        if (! characterInfo->gliderInventory_.add(info)) {
            // TODO: log
            return;
        }
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->createGlider(userInfo_.accountId_, characterId, info)) {
        // TODO: log
    }

}


void AccountCacheImpl::deleteVehicle(ObjectId characterId, ObjectId id)
{
    bool selectIdRemove = false;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        if (characterInfo->vehicleInventory_.selectId_ == id) {
            selectIdRemove = true;
        }
        characterInfo->vehicleInventory_.remove(id);
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (selectIdRemove) {
        if (! db->selectVehicle(characterId, invalidObjectId)) {
            // TODO: log
        }
    }
    if (! db->deleteVehicle(id)) {
        // TODO: log
    }

}


void AccountCacheImpl::deleteGlider(ObjectId characterId, ObjectId id)
{
    bool selectIdRemove = false;
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        if (characterInfo->gliderInventory_.selectId_ == id) {
            selectIdRemove = true;
        }
        characterInfo->gliderInventory_.remove(id);
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (selectIdRemove) {
        if (! db->selectGlider(characterId, invalidObjectId)) {
            // TODO: log
        }
    }
    if (! db->deleteGlider(id)) {
        // TODO: log
    }

}


void AccountCacheImpl::selectVehicle(ObjectId characterId, ObjectId id)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        if (! characterInfo->vehicleInventory_.isExist(id)) {
            // TODO: log
            return;
        }
        characterInfo->vehicleInventory_.selectId_ = id;
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->selectVehicle(characterId, id)) {
        // TODO: log
    }
}



void AccountCacheImpl::selectGlider(ObjectId characterId, ObjectId id)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        if (! characterInfo->gliderInventory_.isExist(id)) {
            // TODO: log
            return;
        }
        characterInfo->gliderInventory_.selectId_ = id;
    }
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->selectGlider(characterId, id)) {
        // TODO: log
    }
}


void AccountCacheImpl::updateGliderDurability(ObjectId characterId, ObjectId id, uint32_t currentValue)
{
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        GliderInfo* gliderInfo = characterInfo->gliderInventory_.getGliderInfo(id);
        if (! gliderInfo) {
            return;
        }
        gliderInfo->durability_ = currentValue;
    }

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->updateGliderDurability(id, currentValue)) {
        // TODO: log
    }
}


void AccountCacheImpl::addCharacterTitle(ObjectId characterId, CharacterTitleCode titleCode)
{
    {
        std::unique_lock<LockType> lock(lock_);

        AccountCharacterTitleCodeSet::iterator pos = accountCharacterTitleCodeSet_.find(characterId);
        if (pos == accountCharacterTitleCodeSet_.end()) {
            return;
        }
        CharacterTitleCodeSet& titleSet = (*pos).second;
        titleSet.insert(titleCode);        
    }

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->addCharacterTitle(characterId, titleCode)) {
        // TODO: log
    }
}


void AccountCacheImpl::updateProcessAchievement(ObjectId characterId, AchievementCode code, const ProcessAchievementInfo& missionInfo)
{
    bool isUpdate = false;
    {
        std::unique_lock<LockType> lock(lock_);

        AccountProcessAchievementInfoMap::iterator pPos = accountProcessAchievementInfoMap_.find(characterId);
        if (pPos == accountProcessAchievementInfoMap_.end()) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        ProcessAchievementInfoMap& processMap = (*pPos).second;
        ProcessAchievementInfoMap::iterator pos = processMap.find(code);
        if (pos != processMap.end()) {
            (*pos).second = missionInfo;
            isUpdate = true;
        }
        else {
            processMap.emplace(code, missionInfo);
        }           
    }

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (isUpdate) {
        if (! db->updateProcessAchievement(characterId, code, missionInfo)) {
            // TODO: log
        }
    }
    else {
        if (! db->addProcessAchievement(characterId, code, missionInfo)) {
            // TODO: log
        }
    }
}


void AccountCacheImpl::completeAchievement(ObjectId characterId, AchievementCode code, AchievementPoint point)
{
    sec_t now = getTime();
    {
        std::unique_lock<LockType> lock(lock_);

        FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        if (! characterInfo) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }

        AccountProcessAchievementInfoMap::iterator pPos = accountProcessAchievementInfoMap_.find(characterId);
        if (pPos == accountProcessAchievementInfoMap_.end()) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        AccountCompleteAchievementInfoMap::iterator cPos = accountCompleteAchievementInfoMap_.find(characterId);
        if (cPos == accountCompleteAchievementInfoMap_.end()) {
            // TODO: 로그를 남기고 유저의 접속을 해제한다
            return;
        }
        ProcessAchievementInfoMap& processMap = (*pPos).second;
        CompleteAchievementInfoMap& completeMap = (*cPos).second;
        ProcessAchievementInfoMap::iterator achievePos = processMap.find(code);
        if (achievePos != processMap.end()) {
            processMap.erase(code);            
        }

        completeMap.emplace(code, CompleteAchievementInfo(code, now));
        characterInfo->achievementPoint_ = point;
    }

    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    if (! db->completeAchievement(characterId, code, point, now)) {
        // TODO: log
    }
}


ErrorCode AccountCacheImpl::queryAchievements(ProcessAchievementInfoMap& processInfoMap, CompleteAchievementInfoMap& completeInfoMap, ObjectId characterId)
{
    std::unique_lock<LockType> lock(lock_);

    AccountProcessAchievementInfoMap::iterator pPos = accountProcessAchievementInfoMap_.find(characterId);
    AccountCompleteAchievementInfoMap::iterator cPos = accountCompleteAchievementInfoMap_.find(characterId);
    
    
    if (pPos != accountProcessAchievementInfoMap_.end()) {
        processInfoMap = (*pPos).second;
    }
    else {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->getProcessAchievements(processInfoMap, characterId)) {
            return ecDatabaseInternalError;
        }
        accountProcessAchievementInfoMap_.emplace(characterId, processInfoMap);
    }

    if (cPos != accountCompleteAchievementInfoMap_.end()) {
        completeInfoMap = (*cPos).second;
    }
    else {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->getCompleteAchievements(completeInfoMap, characterId)) {
            return ecDatabaseInternalError;
        }
        accountCompleteAchievementInfoMap_.emplace(characterId, completeInfoMap);
    }
    return ecOk;
}


ErrorCode AccountCacheImpl::queryCharacterTitles(CharacterTitleCodeSet& titleCodeSet, ObjectId characterId)
{
    std::unique_lock<LockType> lock(lock_);

    AccountCharacterTitleCodeSet::iterator pos = accountCharacterTitleCodeSet_.find(characterId);
    if (pos != accountCharacterTitleCodeSet_.end()) {
        titleCodeSet = (*pos).second;
    }
    else {
        sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
        if (! db->getCharacterTitles(titleCodeSet, characterId)) {
            return ecDatabaseInternalError;
        }
        accountCharacterTitleCodeSet_.emplace(characterId, titleCodeSet);
    }
    return ecOk;
}


void AccountCacheImpl::reset()
{
    std::unique_lock<LockType> lock(lock_);

    accessedAt_ = -1;

    userInfo_.reset();
    accountProcessAchievementInfoMap_.clear();
    accountCompleteAchievementInfoMap_.clear();
    accountCharacterTitleCodeSet_.clear();
    propertyMap_.clear();
}


void AccountCacheImpl::deleteAccountExpireItems()
{
    std::unique_lock<LockType> lock(lock_);

    for (FullCharacterInfos::value_type& value : userInfo_.characters_) {
        FullCharacterInfo& character = value.second;
        deleteCharacterExpireItems(character);        
    }
}

void AccountCacheImpl::deleteCharacterExpireItems(FullCharacterInfo& character)
{
    sec_t now = getTime();
    ItemMap::iterator pos = character.inventory_.items_.begin();
    while (pos != character.inventory_.items_.end()) {
        ItemInfo& itemInfo = (*pos).second;
        if (itemInfo.expireTime_ > 0 && itemInfo.expireTime_ < now && ! itemInfo.isBuildingGuard()) {
            sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
            if (itemInfo.isEquipped() && itemInfo.isAccessory()) {
                AccessoryIndex index = character.getAccessoryIndex(itemInfo.itemId_);
                character.characterAccessories_[index] = invalidObjectId;
                if (! db->unequipAccessoryItem(userInfo_.accountId_,
                    character.objectId_, itemInfo.itemId_, equippedSlotId, 
                    character.getAccessoryIndex(itemInfo.itemId_))) {
                    // TODO: log
                }
            }
            if (! db->removeInventoryItem(itemInfo.itemId_)) {
                // TODO: log
            }
            pos = character.inventory_.items_.erase(pos);
            continue;
        }
        ++pos;
    }
}

}} // namespace gideon { namespace databaseproxyserver {
