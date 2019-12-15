#pragma once

#include <gideon/cs/shared/data/AuctionInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/base/concurrent/Future.h>
#include <sne/core/memory/MemoryPoolMixin.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {

/***
 * class FullAuctionInfoIdPtr
 * 아이디 전용 정열
 ***/
class FullAuctionInfoIdPtr
{
public:
    FullAuctionInfoIdPtr(FullAuctionInfo* ptr) :
        ptr_(ptr) {}
     
    bool operator<(const FullAuctionInfoIdPtr& rhs) const {
        return ptr_->auctionId_ < rhs->auctionId_;
    }

    FullAuctionInfo* operator->() {
        return ptr_;
    }

    const FullAuctionInfo* operator->() const {
        return ptr_;
    }

    FullAuctionInfo& operator*() {
        return *ptr_;
    }
    
    const FullAuctionInfo& operator*() const {
        return *ptr_;
    }
    
private:
    FullAuctionInfo* ptr_;
};

/***
 * class FullAuctionInfoTimePtr
 * 아이디 전용 정열
 ***/
class FullAuctionInfoTimePtr
{
public:
    FullAuctionInfoTimePtr(FullAuctionInfo* ptr) :
        ptr_(ptr) {}
     
    bool operator<(const FullAuctionInfoTimePtr& rhs) const {    
        if (ptr_->expireTime_ == rhs->expireTime_) {
            return ptr_->auctionId_ < rhs->auctionId_;
        }
        return ptr_->expireTime_ < rhs->expireTime_;
    }

    FullAuctionInfo* operator->() {
        return ptr_;
    }

    const FullAuctionInfo* operator->() const {
        return ptr_;
    }

    FullAuctionInfo& operator*() {
        return *ptr_;
    }
    
    const FullAuctionInfo& operator*() const {
        return *ptr_;
    }
    
private:
    FullAuctionInfo* ptr_;
};


/***
 * @class AuctionHouse
 ***/
class AuctionHouse
{
private:
    typedef std::mutex LockType;
    typedef FullAuctionInfo* FullAuctionInfoPtr;

    typedef sne::core::HashMap<AuctionId, FullAuctionInfo*> ActionMap;
    typedef sne::core::Set<FullAuctionInfoIdPtr> FullAuctionSet;
    typedef sne::core::Set<FullAuctionInfoTimePtr> FullAuctionTimeSet;
    typedef sne::core::HashMap<ObjectId, FullAuctionSet> ObjectIdAuctionsMap;
    typedef sne::core::HashMap<CodeType, FullAuctionSet> CodeTypeAuctionsMap;
    typedef sne::core::HashMap<EquipType, FullAuctionSet> EquipTypeAuctionsMap;
    typedef sne::core::HashMap<EquipPart, FullAuctionSet> EquipPartAuctionsMap;

public:
    AuctionHouse(ZoneId zoneId, NpcCode npcCode);
    ~AuctionHouse();

    ErrorCode createAuction(const FullAuctionInfo& auctionInfo, bool isDbSave = true);
    ErrorCode bid(go::Entity& owner, AuctionId auctionId, GameMoney gameMoney);
    ErrorCode buyOut(go::Entity& owner, AuctionId auctionId);
    ErrorCode cancelAuction(go::Entity& owner, AuctionId auctionId);

    void searchEquipmentByEquipType(AuctionInfos& auctionInfos, EquipType equipType) const;
    void searchEquipmentByEquipPart(AuctionInfos& auctionInfos, EquipPart equipPart) const;
    void searchWeapon(AuctionInfos& auctionInfos) const;
    void searchArmor(AuctionInfos& auctionInfos) const;
    void searchItemsByCodeType(AuctionInfos& auctionInfos, CodeType codeType) const;   
    void querySellingItems(AuctionInfos& auctionInfos, ObjectId ownerId) const;
    void queryBiddingItems(AuctionInfos& auctionInfos, ObjectId ownerId) const;

    void decideBidder();
    
private:
    void insertSellerAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void insertBuyerAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void insertCodeTypeAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void insertEquipTypeAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void insertEquipPartAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void insertWeaponAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void insertArmorAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void insertTimeAuctionInfo(FullAuctionInfoTimePtr auctionPtr);

private:
    void deleteAuctionInfo(AuctionId auctionId);
    void deleteSellerAuctionInfo(ObjectId sellerId, FullAuctionInfoIdPtr auctionPtr);
    void deleteBuyerAuctionInfo(ObjectId buyerId, FullAuctionInfoIdPtr auctionPtr);
    void deleteCodeTypeAuctionInfo(CodeType codeType, FullAuctionInfoIdPtr auctionPtr);
    void deleteEquipTypeAuctionInfo(EquipType equipType, FullAuctionInfoIdPtr auctionPtr);
    void deleteEquipPartAuctionInfo(EquipPart equipPart, FullAuctionInfoIdPtr auctionPtr);
    void deleteWeaponAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void deleteArmorAuctionInfo(FullAuctionInfoIdPtr auctionPtr);
    void deleteTimeTypeAuctionInfo(FullAuctionInfoTimePtr  auctionPtr);


private:
    FullAuctionInfo* getAuctionInfo(AuctionId auctionId);
    const FullAuctionInfo* getAuctionInfo(AuctionId auctionId) const;

    void startDecidebidderTask();
    void stopDecidebidderTask();
private:
    mutable LockType lock_;

    ZoneId zoneId_;
    NpcCode npcCode_;
    ActionMap allActionInfoMap_;
    ObjectIdAuctionsMap sellerAuctions_;
    ObjectIdAuctionsMap buyerAuctions_;
    CodeTypeAuctionsMap codeTypeAuctions_;
    EquipTypeAuctionsMap equipTypeAuctions_;
    EquipPartAuctionsMap equipPartAuctions_;
    FullAuctionSet weaponAuctions_;
    FullAuctionSet armorAuctions_;
    FullAuctionTimeSet timeAuctionSet_;
    

    sne::base::Future::WeakRef decidebidderTask_;    
};

}} // namespace gideon { namespace zoneserver {
