#include "ZoneServerPCH.h"
#include "AuctionHouse.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../mail/MailService.h"
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <sne/base/utility/Logger.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver {

namespace
{
/**
 * @class DecidebidderTask
 */
class DecidebidderTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<DecidebidderTask>
{
public:
    DecidebidderTask(AuctionHouse& auctionHouse) :
        auctionHouse_(auctionHouse) {}

private:
    virtual void run() {
        auctionHouse_.decideBidder();
    }

private:
    AuctionHouse& auctionHouse_;
};

} // namespace


AuctionHouse::AuctionHouse(ZoneId zoneId, NpcCode npcCode) :
    zoneId_(zoneId),
    npcCode_(npcCode)
{
    startDecidebidderTask();
}


AuctionHouse::~AuctionHouse()
{
    stopDecidebidderTask();
    for (ActionMap::value_type& value : allActionInfoMap_) {
        delete value.second;
    }
}


ErrorCode AuctionHouse::createAuction(const FullAuctionInfo& auctionInfo, bool isDbSave)
{
    {
        std::unique_lock<LockType> lock(lock_);

        if (getAuctionInfo(auctionInfo.auctionId_)) {
            return ecServerInternalError;
        }

        FullAuctionInfo* auctionPtr = new FullAuctionInfo(auctionInfo);
        allActionInfoMap_.emplace(auctionInfo.auctionId_, auctionPtr);

        FullAuctionInfoIdPtr auctionIdPtr(auctionPtr);
        insertSellerAuctionInfo(auctionIdPtr);
        insertBuyerAuctionInfo(auctionIdPtr);
        insertCodeTypeAuctionInfo(auctionIdPtr);
        insertEquipTypeAuctionInfo(auctionIdPtr);
        insertEquipPartAuctionInfo(auctionIdPtr);
        insertWeaponAuctionInfo(auctionPtr);
        insertArmorAuctionInfo(auctionPtr);
        insertTimeAuctionInfo(FullAuctionInfoTimePtr(auctionPtr));
    }

    
    if (isDbSave) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncCreateAuction(zoneId_, npcCode_, auctionInfo);
    }

    return ecOk;
}


ErrorCode AuctionHouse::bid(go::Entity& owner, AuctionId auctionId, GameMoney gameMoney)
{
    if (owner.queryMoneyable()->getGameMoney() < gameMoney) {
        return ecAuctionNotEnoughGameMoney;
    }

    {
        std::unique_lock<LockType> lock(lock_);

        FullAuctionInfo* auctionInfo = getAuctionInfo(auctionId);
        if (! auctionInfo) {
            return ecAuctionInvalidAuction;
        }

        if (auctionInfo->sellerId_ == owner.getObjectId()) {
            return ecAuctionSelfNotBidAndBuyOut;
        }

        if (auctionInfo->currentBidMoney_ >= gameMoney) {
            return ecAuctionBidHigherCurrentBid;
        }

        if (isValidObjectId(auctionInfo->buyerId_)) {
            MAIL_SERVICE->sendMailFromAuction(mtAuctionFailedBid, npcCode_, auctionInfo->buyerId_,
                InventoryInfo(), auctionInfo->currentBidMoney_);
            deleteBuyerAuctionInfo(auctionInfo->buyerId_, FullAuctionInfoIdPtr(auctionInfo));
        }
        auctionInfo->buyerId_ = owner.getObjectId();
        auctionInfo->currentBidMoney_ = gameMoney;
        insertBuyerAuctionInfo(FullAuctionInfoIdPtr(auctionInfo));
    }

    owner.queryMoneyable()->downGameMoney(gameMoney);
    
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncUpdateBid(zoneId_, auctionId, owner.getObjectId(), gameMoney);
    }
    return ecOk;     
}


ErrorCode AuctionHouse::buyOut(go::Entity& owner, AuctionId auctionId)
{
    GameMoney buyerMoney = owner.queryMoneyable()->getGameMoney();
    GameMoney buyoutMoney = 0;
    GameMoney depositFee = 0;
    ObjectId sellerId = invalidObjectId;
    InventoryInfo inventory;
    {
        std::unique_lock<LockType> lock(lock_);

        FullAuctionInfo* auctionInfo = getAuctionInfo(auctionId);
        if (! auctionInfo) {
            return ecAuctionInvalidAuction;
        }

        if (auctionInfo->sellerId_ == owner.getObjectId()) {
            return ecAuctionSelfNotBidAndBuyOut;
        }
        
        if (buyerMoney < auctionInfo->buyoutMoney_) {
            return ecAuctionNotEnoughGameMoney;
        }  

        buyoutMoney = auctionInfo->buyoutMoney_;
        sellerId = auctionInfo->sellerId_;
        depositFee = auctionInfo->depositFee_;
        if (isValidObjectId(auctionInfo->buyerId_)) {
            MAIL_SERVICE->sendMailFromAuction(mtAuctionFailedBid, npcCode_, auctionInfo->buyerId_,
                InventoryInfo(), auctionInfo->currentBidMoney_);
            deleteBuyerAuctionInfo(auctionInfo->buyerId_, FullAuctionInfoIdPtr(auctionInfo));
        }

        inventory.addItem(auctionInfo->itemInfo_);               
        deleteAuctionInfo(auctionId);
    }
    
    owner.queryMoneyable()->downGameMoney(buyoutMoney);

    MAIL_SERVICE->sendMailFromAuction(mtAuctionSuccessfulBid, npcCode_, owner.getObjectId(), inventory, 0);  
    const GameMoney commission = calculateAuctionCommission(buyoutMoney);
    MAIL_SERVICE->sendMailFromAuction(mtAuctionSucceeded, npcCode_, sellerId, InventoryInfo(), 
        (buyoutMoney + depositFee) - commission);  

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncDeleteAuction(zoneId_, auctionId);
    }
    return ecOk;
}


ErrorCode AuctionHouse::cancelAuction(go::Entity& owner, AuctionId auctionId)
{
    InventoryInfo inventory;
    {
        std::unique_lock<LockType> lock(lock_);

        const FullAuctionInfo* auctionInfo = getAuctionInfo(auctionId);
        if (! auctionInfo) {
            return ecAuctionInvalidAuction;
        }

        if (auctionInfo->sellerId_ != owner.getObjectId()) {
            return ecAuctionNotSelfAuction;
        }

        if (isValidObjectId(auctionInfo->buyerId_)) {
            MAIL_SERVICE->sendMailFromAuction(mtAuctionFailedBid, npcCode_, auctionInfo->buyerId_,
                InventoryInfo(), auctionInfo->currentBidMoney_);
        }


        inventory.addItem(auctionInfo->itemInfo_);       

        deleteAuctionInfo(auctionId);
    }
    
    MAIL_SERVICE->sendMailFromAuction(mtAuctionCancelled, npcCode_, owner.getObjectId(), inventory, 0);         
    
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncDeleteAuction(zoneId_, auctionId);
    }

    return ecOk;
}


void AuctionHouse::searchEquipmentByEquipType(AuctionInfos& auctionInfos, EquipType equipType) const
{
    std::unique_lock<LockType> lock(lock_);

    EquipTypeAuctionsMap::const_iterator pos = equipTypeAuctions_.find(equipType);
    if (pos != equipTypeAuctions_.end()) {
        const FullAuctionSet& auctionSet = (*pos).second;
        for (const FullAuctionSet::value_type& value : auctionSet) {
            const FullAuctionInfoIdPtr& fullInfo = (value);
            auctionInfos.push_back(*fullInfo);
        }
    }
}


void AuctionHouse::searchEquipmentByEquipPart(AuctionInfos& auctionInfos, EquipPart equipPart) const
{
    std::unique_lock<LockType> lock(lock_);

    EquipPartAuctionsMap::const_iterator pos = equipPartAuctions_.find(equipPart);
    if (pos != equipPartAuctions_.end()) {
        const FullAuctionSet& auctionSet = (*pos).second;
        for (const FullAuctionSet::value_type& value : auctionSet) {
            const FullAuctionInfoIdPtr& fullInfo = (value);
            auctionInfos.push_back(*fullInfo);
        }
    }
}


void AuctionHouse::searchWeapon(AuctionInfos& auctionInfos) const
{
    std::unique_lock<LockType> lock(lock_);

    for (const FullAuctionSet::value_type& value : weaponAuctions_) {
        const FullAuctionInfoIdPtr& fullInfo = (value);
        auctionInfos.push_back(*fullInfo);
    }
}


void AuctionHouse::searchArmor(AuctionInfos& auctionInfos) const
{
    std::unique_lock<LockType> lock(lock_);

    for (const FullAuctionSet::value_type& value : armorAuctions_) {
        const FullAuctionInfoIdPtr& fullInfo = (value);
        auctionInfos.push_back(*fullInfo);
    }
}


void AuctionHouse::searchItemsByCodeType(AuctionInfos& auctionInfos, CodeType codeType)  const
{
    std::unique_lock<LockType> lock(lock_);

    CodeTypeAuctionsMap::const_iterator pos = codeTypeAuctions_.find(codeType);
    if (pos != codeTypeAuctions_.end()) {
        const FullAuctionSet& auctionSet = (*pos).second;
        for (const FullAuctionSet::value_type& value : auctionSet) {
            const FullAuctionInfoIdPtr& fullInfo = (value);
            auctionInfos.push_back(*fullInfo);
        }
    }
}


void AuctionHouse::querySellingItems(AuctionInfos& auctionInfos, ObjectId ownerId) const
{
    std::unique_lock<LockType> lock(lock_);

    ObjectIdAuctionsMap::const_iterator pos = sellerAuctions_.find(ownerId);
    if (pos != sellerAuctions_.end()) {
        const FullAuctionSet& auctionSet = (*pos).second;
        for (const FullAuctionSet::value_type& value : auctionSet) {
            const FullAuctionInfoIdPtr& fullInfo = (value);
            auctionInfos.push_back(*fullInfo);
        }
    }
}


void AuctionHouse::queryBiddingItems(AuctionInfos& auctionInfos, ObjectId ownerId) const
{
    std::unique_lock<LockType> lock(lock_);

    ObjectIdAuctionsMap::const_iterator pos = buyerAuctions_.find(ownerId);
    if (pos != buyerAuctions_.end()) {
        const FullAuctionSet& auctionSet = (*pos).second;
        for (const FullAuctionSet::value_type& value : auctionSet) {
            const FullAuctionInfoIdPtr& fullInfo = (value);
            auctionInfos.push_back(*fullInfo);
        }
    }
}


void AuctionHouse::decideBidder()
{
    std::unique_lock<LockType> lock(lock_);

    const sec_t now = getTime();
    FullAuctionTimeSet timeSet_;
    FullAuctionTimeSet::iterator pos = timeAuctionSet_.begin();
    FullAuctionTimeSet::iterator end = timeAuctionSet_.end();
    for (; pos != end; ++pos) {
        FullAuctionInfoTimePtr auctionInfo = (*pos);
        if (auctionInfo->expireTime_ < now) {
            timeSet_.insert(auctionInfo);
            continue;
        }
    }

    FullAuctionTimeSet::iterator copyPos = timeSet_.begin();
    FullAuctionTimeSet::iterator copyEnd = timeSet_.end();
    for (; copyPos != copyEnd; ++copyPos) {
        FullAuctionInfoTimePtr auctionInfo = (*copyPos);
        InventoryInfo inventory;
        inventory.addItem(auctionInfo->itemInfo_);
        

        if (isValidObjectId(auctionInfo->buyerId_)) {
            const GameMoney commission = 
                calculateAuctionCommission(auctionInfo->currentBidMoney_);

            MAIL_SERVICE->sendMailFromAuction(mtAuctionSucceeded, npcCode_, auctionInfo->sellerId_,
                InventoryInfo(), (auctionInfo->currentBidMoney_ + auctionInfo->depositFee_)- commission);
            MAIL_SERVICE->sendMailFromAuction(mtAuctionSuccessfulBid, npcCode_, auctionInfo->buyerId_,
                inventory, 0);   
        }
        else {
            MAIL_SERVICE->sendMailFromAuction(mtAuctionFailed, npcCode_, auctionInfo->sellerId_,
                inventory, 0);
        }

        {
            DatabaseGuard db(SNE_DATABASE_MANAGER);
            db->asyncDeleteAuction(zoneId_, auctionInfo->auctionId_);
        }
        deleteAuctionInfo(auctionInfo->auctionId_);
    }
}


void AuctionHouse::insertSellerAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    ObjectIdAuctionsMap::iterator sellerPos = sellerAuctions_.find(auctionPtr->sellerId_);
    if (sellerPos != sellerAuctions_.end()) {
        FullAuctionSet& auctionSet = (*sellerPos).second;
        auctionSet.insert(auctionPtr);
    }
    else {
        FullAuctionSet auctionSet;
        auctionSet.insert(auctionPtr);
        sellerAuctions_.emplace(auctionPtr->sellerId_, auctionSet);
    }
}


void AuctionHouse::insertBuyerAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    if (! isValidObjectId(auctionPtr->buyerId_)) {
        return;
    }

    ObjectIdAuctionsMap::iterator sellerPos = buyerAuctions_.find(auctionPtr->buyerId_);
    if (sellerPos != buyerAuctions_.end()) {
        FullAuctionSet& auctionSet = (*sellerPos).second;
        auctionSet.insert(auctionPtr);
    }
    else {
        FullAuctionSet auctionSet;
        auctionSet.insert(auctionPtr);
        buyerAuctions_.emplace(auctionPtr->buyerId_, auctionSet);
    }
}


void AuctionHouse::insertCodeTypeAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    CodeType codeType = getCodeType(auctionPtr->itemInfo_.itemCode_);

    CodeTypeAuctionsMap::iterator codeTypePos = codeTypeAuctions_.find(codeType);
    if (codeTypePos != codeTypeAuctions_.end()) {
        FullAuctionSet& auctionSet = (*codeTypePos).second;
        auctionSet.insert(auctionPtr);
    }
    else {
        FullAuctionSet auctionSet;
        auctionSet.insert(auctionPtr);
        codeTypeAuctions_.emplace(codeType, auctionSet);
    }
}


void AuctionHouse::insertEquipTypeAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    if (! auctionPtr->itemInfo_.isEquipment()) {
        return;
    }

    const gdt::equip_t* equip = EQUIP_TABLE->getEquip(auctionPtr->itemInfo_.itemCode_);
    if (! equip) {
        assert(false);
        return;
    }

    EquipType equipType = toEquipType(equip->equip_type());
    EquipTypeAuctionsMap::iterator equipTypePos = equipTypeAuctions_.find(equipType);
    if (equipTypePos != equipTypeAuctions_.end()) {
        FullAuctionSet& auctionSet = (*equipTypePos).second;
        auctionSet.insert(auctionPtr);
    }
    else {
        FullAuctionSet auctionSet;
        auctionSet.insert(auctionPtr);
        equipTypeAuctions_.emplace(equipType, auctionSet);
    }
}


void AuctionHouse::insertEquipPartAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    if (! auctionPtr->itemInfo_.isEquipment()) {
        return;
    }

    const gdt::equip_t* equip = EQUIP_TABLE->getEquip(auctionPtr->itemInfo_.itemCode_);
    if (! equip) {
        assert(false);
        return;
    }

    EquipPart equipPart = getEquipPart(toEquipType(equip->equip_type()));
    EquipPartAuctionsMap::iterator equipPartPos = equipPartAuctions_.find(equipPart);
    if (equipPartPos != equipPartAuctions_.end()) {
        FullAuctionSet& auctionSet = (*equipPartPos).second;
        auctionSet.insert(auctionPtr);
    }
    else {
        FullAuctionSet auctionSet;
        auctionSet.insert(auctionPtr);
        equipPartAuctions_.emplace(equipPart, auctionSet);
    }
}


void AuctionHouse::insertWeaponAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    if (! auctionPtr->itemInfo_.isEquipment()) {
        return;
    }

    const gdt::equip_t* equip = EQUIP_TABLE->getEquip(auctionPtr->itemInfo_.itemCode_);
    if (! equip) {
        assert(false);
        return;
    }

    EquipPart equipPart = getEquipPart(toEquipType(equip->equip_type()));
    if (isWeaponPart(equipPart)) {
        weaponAuctions_.insert(auctionPtr);
    }
}


void AuctionHouse::insertArmorAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    if (! auctionPtr->itemInfo_.isEquipment()) {
        return;
    }

    const gdt::equip_t* equip = EQUIP_TABLE->getEquip(auctionPtr->itemInfo_.itemCode_);
    if (! equip) {
        assert(false);
        return;
    }

    EquipPart equipPart = getEquipPart(toEquipType(equip->equip_type()));
    if (! isWeaponPart(equipPart)) {
        armorAuctions_.insert(auctionPtr);
    }
}


void AuctionHouse::insertTimeAuctionInfo(FullAuctionInfoTimePtr auctionPtr)
{
    timeAuctionSet_.insert(auctionPtr);
}


void AuctionHouse::deleteAuctionInfo(AuctionId auctionId)
{
    // TODO: 메모리 관리 개선
    std::unique_ptr<FullAuctionInfo> auctionInfo(getAuctionInfo(auctionId));
    if (! auctionInfo) {
        return;
    }
    deleteSellerAuctionInfo(auctionInfo->sellerId_, FullAuctionInfoIdPtr(auctionInfo.get()));
    deleteBuyerAuctionInfo(auctionInfo->buyerId_, FullAuctionInfoIdPtr(auctionInfo.get()));
    CodeType codeType = getCodeType(auctionInfo->itemInfo_.itemCode_);
    deleteCodeTypeAuctionInfo(codeType, FullAuctionInfoIdPtr(auctionInfo.get()));
    if (auctionInfo->itemInfo_.isEquipment()) {
        const gdt::equip_t* equip = EQUIP_TABLE->getEquip(auctionInfo->itemInfo_.itemCode_);
        if (equip) {
            EquipType equipType = toEquipType(equip->equip_type());
            deleteEquipTypeAuctionInfo(equipType, FullAuctionInfoIdPtr(auctionInfo.get()));
            EquipPart equipPart = getEquipPart(equipType);
            deleteEquipPartAuctionInfo(equipPart, FullAuctionInfoIdPtr(auctionInfo.get()));
        }              
    }
    deleteWeaponAuctionInfo(FullAuctionInfoIdPtr(auctionInfo.get()));
    deleteArmorAuctionInfo(FullAuctionInfoIdPtr(auctionInfo.get()));
    deleteTimeTypeAuctionInfo(FullAuctionInfoTimePtr(auctionInfo.get()));
    allActionInfoMap_.erase(auctionId);
}


void AuctionHouse::deleteSellerAuctionInfo(ObjectId sellerId, FullAuctionInfoIdPtr auctionPtr)
{
    ObjectIdAuctionsMap::iterator sellerPos = sellerAuctions_.find(sellerId);
    if (sellerPos != sellerAuctions_.end()) {
        FullAuctionSet& auctionSet = (*sellerPos).second;
        auctionSet.erase(auctionPtr);
    }
}


void AuctionHouse::deleteBuyerAuctionInfo(ObjectId buyerId, FullAuctionInfoIdPtr auctionPtr)
{
    ObjectIdAuctionsMap::iterator sellerPos = buyerAuctions_.find(buyerId);
    if (sellerPos != buyerAuctions_.end()) {
        FullAuctionSet& auctionSet = (*sellerPos).second;
        auctionSet.erase(auctionPtr);
    }
}


void AuctionHouse::deleteCodeTypeAuctionInfo(CodeType codeType, FullAuctionInfoIdPtr auctionPtr)
{
    CodeTypeAuctionsMap::iterator codeTypePos = codeTypeAuctions_.find(codeType);
    if (codeTypePos != codeTypeAuctions_.end()) {
        FullAuctionSet& auctionSet = (*codeTypePos).second;
        auctionSet.erase(auctionPtr);        
    }
}


void AuctionHouse::deleteEquipTypeAuctionInfo(EquipType equipType, FullAuctionInfoIdPtr auctionPtr)
{
    EquipTypeAuctionsMap::iterator equipTypePos = equipTypeAuctions_.find(equipType);
    if (equipTypePos != equipTypeAuctions_.end()) {
        FullAuctionSet& auctionSet = (*equipTypePos).second;
        auctionSet.erase(auctionPtr);
    }
}


void AuctionHouse::deleteEquipPartAuctionInfo(EquipPart equipPart, FullAuctionInfoIdPtr auctionPtr)
{
    EquipPartAuctionsMap::iterator equipPartPos = equipPartAuctions_.find(equipPart);
    if (equipPartPos != equipPartAuctions_.end()) {
        FullAuctionSet& auctionSet = (*equipPartPos).second;
        auctionSet.erase(auctionPtr);
    }
}


void AuctionHouse::deleteWeaponAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    weaponAuctions_.erase(auctionPtr);
}


void AuctionHouse::deleteArmorAuctionInfo(FullAuctionInfoIdPtr auctionPtr)
{
    armorAuctions_.erase(auctionPtr);
}


void AuctionHouse::deleteTimeTypeAuctionInfo(FullAuctionInfoTimePtr auctionPtr)
{
    timeAuctionSet_.erase(auctionPtr);
}



FullAuctionInfo* AuctionHouse::getAuctionInfo(AuctionId auctionId)
{
    ActionMap::iterator pos = allActionInfoMap_.find(auctionId);
    if (pos != allActionInfoMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


const FullAuctionInfo* AuctionHouse::getAuctionInfo(AuctionId auctionId) const
{
    ActionMap::const_iterator pos = allActionInfoMap_.find(auctionId);
    if (pos != allActionInfoMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


void AuctionHouse::startDecidebidderTask()
{
    sne::base::Future::Ref decidebidderTask = decidebidderTask_.lock();
    if (decidebidderTask.get()) {
        return;
    }

    const msec_t interval = 1000 * 60 * 5; // 5분
    decidebidderTask_ = TASK_SCHEDULER->schedule(std::make_unique<DecidebidderTask>(*this),
        interval, interval);
}


void AuctionHouse::stopDecidebidderTask()
{
    sne::base::Future::Ref decidebidderTask = decidebidderTask_.lock();
    if (! decidebidderTask.get()) {
        return;
    }

    decidebidderTask->cancel();
    decidebidderTask_.reset();
}

}} // namespace gideon { namespace zoneserver {
