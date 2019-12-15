#include "ZoneServerPCH.h"
#include "MailService.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/item/Inventory.h"
#include "../../model/bank/BankAccount.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/MailCallback.h"
#include "../../helper/InventoryHelper.h"
#include "../../user/ZoneUserManager.h"
#include "../../world/World.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/utility/Logger.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver {

namespace {

ErrorCode getNpcCode(NpcCode& npcCode, go::Entity& /*owner*/, ObjectId /*npcId*/)
{
    //go::Knowable* knowable = owner.queryKnowable();
    //if (! knowable) {
    //    return ecAuctionNotFindNpc;
    //}

    //go::Npc* auctionNpc = static_cast<go::Npc*>(knowable->getEntity(GameObjectInfo(otNpc, npcId)));
    //if (! auctionNpc) {
    //    return ecAuctionNotFindNpc;
    //}

    //if (! auctionNpc->hasMail()) {
    //    return ecAuctionCannotAuctionable;
    //}

    //npcCode = auctionNpc->getNpcCode();
    npcCode = 100728892;
    return ecOk;
}


/**
 * @class DatabaseCallbackResisterTask
 */
class DatabaseCallbackResisterTask : public sne::database::DatabaseTask
{
public:
    DatabaseCallbackResisterTask(serverbase::GameMailCallback& callback) :
        callback_(callback) {}

    virtual void run(sne::database::Database& database) {
        serverbase::ProxyGameDatabase& db = 
            static_cast<serverbase::ProxyGameDatabase&>(database);
        db.registGameMailCallback(callback_);
    }

private:
    serverbase::GameMailCallback& callback_;
};

} // namespace {

//= MailService

SNE_DEFINE_SINGLETON(MailService)

bool MailService::initialize()
{
    DatabaseCallbackResisterTask callbackTask(*this);
    SNE_DATABASE_MANAGER->enumerate(callbackTask);

    MailService::instance(this);
    return true;
}


void MailService::releaseSyncMail(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    waitingPlayers_.erase(playerId);
    mails_.erase(playerId);
}


ErrorCode MailService ::sendMail(go::Entity& sender, const SendMailInfo& mailInfo)
{   
    if (ZONEUSER_MANAGER->hasNickname(sender.getAccountId(), mailInfo.nickname_)) {
        return ecMailNotSendSelf;
    }

    if (! mailInfo.isValid()) {
        return ecMailInvalidMail;
    }

    const GameMoney neededGameMoney = payMailFee + mailInfo.gameMoney_;

    Inventory& inventory = sender.queryInventoryable()->getInventory();
    MailInfo mail;
    {
        std::lock_guard<go::Entity::LockType> lock(sender.getLock());

        const GameMoney currentGameMoney = sender.queryMoneyable()->getGameMoney();
        if (currentGameMoney < neededGameMoney) {
            return ecMailNotEnoughMoney;
        }

        for (ObjectId itemId : mailInfo.itemIds_) {
            ItemInfo item = inventory.getItemInfo(itemId);
            if (! item.isValid()) {
                return ecMailInvalidSendMailItem;
            }
            mail.contentInfo_.mailInventory_.addItem(item);            
        }
        for (ObjectId itemId : mailInfo.itemIds_) {
            inventory.removeItem(itemId);
        }

        sender.queryMoneyable()->downGameMoney(neededGameMoney);
    }

    mail.titleInfo_.nickname_ = sender.getNickname();
    mail.titleInfo_.objectId_ = sender.getObjectId();
    mail.titleInfo_.title_ = mailInfo.title_;
    mail.titleInfo_.hasAttachment_ = ! mailInfo.itemIds_.empty() || mailInfo.gameMoney_ > 0;
    mail.titleInfo_.time_ = getTime();
    mail.titleInfo_.mailType_ = mtNormal;
    mail.contentInfo_.mailBody_ = mailInfo.mailBody_;
    mail.contentInfo_.gameMoney_ = mailInfo.gameMoney_;

    DatabaseGuard db(SNE_DATABASE_MANAGER);

    db->asyncSendMail(mailInfo.nickname_, mail);

    return ecOk;
}


void MailService::sendMailFromAuction(MailType mailType, DataCode npcCode,
    ObjectId receiverId, const InventoryInfo& inventoryInfo, GameMoney gameMoney)
{
    MailInfo mail;
    mail.titleInfo_.mailType_ = mailType;
    mail.titleInfo_.dataCode_ = npcCode;
    mail.titleInfo_.time_ = getTime();
    mail.contentInfo_.gameMoney_ = gameMoney;
    mail.contentInfo_.mailInventory_ = inventoryInfo;
    mail.titleInfo_.hasAttachment_ = true;
    DatabaseGuard db(SNE_DATABASE_MANAGER);

    db->asyncSendMail(receiverId, mail);
}


void MailService::sendMailFromAchievement(ObjectId receiverId, 
    DataCode achievementCode, const InventoryInfo& inventoryInfo)
{
    MailInfo mail;
    mail.titleInfo_.mailType_ = mtAchievementCompleteReward;
    mail.titleInfo_.dataCode_ = achievementCode;
    mail.titleInfo_.time_ = getTime();
    mail.contentInfo_.gameMoney_ = 0;
    mail.contentInfo_.mailInventory_ = inventoryInfo;
    mail.titleInfo_.hasAttachment_ = true;
    DatabaseGuard db(SNE_DATABASE_MANAGER);

    db->asyncSendMail(receiverId, mail);
}


ErrorCode MailService::syncMail(ObjectId playerId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        if (isWaitingPlayer(playerId)) {
            return ecMailSync;
        }
        else if (isSyncPlayer(playerId)) {
            return ecMailAlreadySync;
        }

        waitingPlayers_.insert(playerId);
    }

    
    DatabaseGuard db(SNE_DATABASE_MANAGER);

    db->asyncSyncMail(playerId);

    return ecOk;
}


ErrorCode MailService::queryMailTitle(MailTitleInfo& mailInfo, ObjectId playerId, MailId mailId)
{
    std::lock_guard<LockType> lock(lock_);

    const MailInfo& mail = getMailInfo(playerId, mailId);
    if (! mail.isValid()) {
        return ecMailInvalidMail;
    }
    mailInfo = mail.titleInfo_;
    return ecOk;
}


ErrorCode MailService::queryMailContent(MailContentInfo& mailInfo, ObjectId playerId, MailId mailId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        MailInfo& mail = getMailInfo(playerId, mailId);
        if (! mail.isValid()) {
            return ecMailInvalidMail;
        }
        mailInfo = mail.contentInfo_;
        if (! mail.titleInfo_.isRead_) {
            mail.titleInfo_.isRead_ = true;
        }
    }

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncReadMail(mailId);
    return ecOk;
}


ErrorCode MailService::aquireMailItems(go::Entity& owner, MailId mailId)
{
    Inventory& inventory = owner.queryInventoryable()->getInventory();
    MailInfo& mail = getMailInfo(owner.getObjectId(), mailId);
    if (! mail.isValid()) {
        return ecMailInvalidMail;
    }
    
    {
        std::lock_guard<LockType> lock(lock_);

        BaseItemInfos baseItemInfos;
        for (const ItemMap::value_type& value : mail.contentInfo_.mailInventory_.items_) {
            baseItemInfos.push_back(BaseItemInfo(value.second.itemCode_, value.second.count_));
        }

        if (! inventory.canAddItemByBaseItemInfos(baseItemInfos)) {
            return ecInventoryIsFull;
        }

        for (const ItemMap::value_type& value : mail.contentInfo_.mailInventory_.items_) {
            const ItemInfo& itemInfo = value.second;
            inventory.addItem(createAddItemInfoByItemInfo(itemInfo));
        }

        if (mail.contentInfo_.gameMoney_ > 0) {
            owner.queryMoneyable()->upGameMoney(mail.contentInfo_.gameMoney_);
        }
        mail.contentInfo_.gameMoney_ = 0;
        mail.contentInfo_.mailInventory_.reset();
    }

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAquireItemsInMail(mailId);
    return ecOk;
}


ErrorCode MailService::deleteMail(ObjectId playerId, MailId mailId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        MailInfo& mail = getMailInfo(playerId, mailId);
        if (! mail.isValid()) {
            return ecMailInvalidMail;
        }
        mails_.erase(mailId);
    }
    
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncDeleteMail(mailId);
    return ecOk;
}


void MailService::onSyncMail(ObjectId playerId, const MailInfos& mails)
{
    go::Entity* player = WORLD->getPlayer(playerId);
    if (! player) {
        return;        
    }
    MailTitleInfos titles;
    {
        std::lock_guard<LockType> lock(lock_);

        waitingPlayers_.erase(playerId);
        mails_.emplace(playerId, mails);
        queryMails_i(titles, playerId);
    }

    gc::MailCallback* mailCallback = player->getController().queryMailCallback();
    if (mailCallback) {
        mailCallback->mailSynchronized(titles);
    }
}


void MailService::onReceive(ObjectId receiverId, const MailInfo& mail)
{
    go::Entity* receiver = WORLD->getPlayer(receiverId);
    if (! receiver) {
        return;        
    }
    MailInfos* mailInfos = nullptr;
    mailInfos = getMailInfos(receiverId);
    if (mailInfos) {
        std::lock_guard<LockType> lock(lock_);

        mailInfos->emplace(mail.titleInfo_.mailId_, mail);
    }

   
    gc::MailCallback* mailCallback = receiver->getController().queryMailCallback();
    if (mailCallback) {
        mailCallback->mailReceived(mail.titleInfo_.mailId_);
    }    
}


void MailService::onNotReadMail(ObjectId playerId)
{
    go::Entity* receiver = WORLD->getPlayer(playerId);
    if (! receiver) {
        return;        
    }
    gc::MailCallback* mailCallback = receiver->getController().queryMailCallback();
    if (mailCallback) {
        mailCallback->mailReceived(invalidMailId);
    }    
}


void MailService::queryMails_i(MailTitleInfos& mailInfos, ObjectId playerId)
{
    PlayerMailInfos::const_iterator pos = mails_.find(playerId);
    if (pos != mails_.end()) {
        const MailInfos& mails = (*pos).second;
        for (const MailInfos::value_type& value : mails) {
            mailInfos.push_back(value.second.titleInfo_);
        }
    }    
}


bool MailService::isWaitingPlayer(ObjectId playerId) const
{
    return waitingPlayers_.find(playerId) != waitingPlayers_.end();
}


bool MailService::isSyncPlayer(ObjectId playerId) const
{
    return mails_.find(playerId) != mails_.end();
}


MailInfos* MailService::getMailInfos(ObjectId playId)
{
    PlayerMailInfos::iterator pos = mails_.find(playId);
    if (pos != mails_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


const MailInfos* MailService::getMailInfos(ObjectId playId) const
{
    PlayerMailInfos::const_iterator pos = mails_.find(playId);
    if (pos != mails_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}



MailInfo& MailService::getMailInfo(ObjectId playId, MailId mailId)
{
    PlayerMailInfos::iterator pos = mails_.find(playId);
    if (pos != mails_.end()) {
        MailInfos& mails = (*pos).second;
        MailInfos::iterator mailPos = mails.find(mailId);
        if (mailPos != mails.end()) {
            return (*mailPos).second;
        }
    }
    static MailInfo nullMailInfo;
    return nullMailInfo;
}


const MailInfo& MailService::getMailInfo(ObjectId playId, MailId mailId) const
{
    PlayerMailInfos::const_iterator pos = mails_.find(playId);
    if (pos != mails_.end()) {
        const MailInfos& mails = (*pos).second;
        MailInfos::const_iterator mailPos = mails.find(mailId);
        if (mailPos != mails.end()) {
            return (*mailPos).second;
        }
    }
    static const MailInfo nullMailInfo;
    return nullMailInfo;
}

}} // namespace gideon { namespace zoneserver {
