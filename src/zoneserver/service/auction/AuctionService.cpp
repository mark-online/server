#include "ZoneServerPCH.h"
#include "AuctionService.h"
#include "AuctionHouse.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/item/Inventory.h"
#include <sne/base/utility/Logger.h>


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

    //if (! auctionNpc->hasAuction()) {
    //    return ecAuctionCannotAuctionable;
    //}

    //npcCode = auctionNpc->getNpcCode();
    npcCode = 100728892;
    return ecOk;
}

}

SNE_DEFINE_SINGLETON(AuctionService)


AuctionService::AuctionService(ZoneId zoneId) :
    zoneId_(zoneId)
{
}


AuctionService::~AuctionService()
{
}


bool AuctionService::initialize(const ServerAuctionInfoMap& auctionMap)
{
    AuctionId maxAuctionId = 0;
    for (const ServerAuctionInfoMap::value_type& saValue : auctionMap) {
        AuctionHouse* auctionHouse = new AuctionHouse(zoneId_, saValue.first);
        auctionHouseMap_.emplace(saValue.first, auctionHouse);
        const FullAuctionInfoMap& infos = saValue.second;
        for (const FullAuctionInfoMap::value_type& faValue : infos) {
            const FullAuctionInfo& info = faValue.second;
            if (isFailed(auctionHouse->createAuction(info, false))) {
                // TODO 로그
                return false;
            }
            if (info.auctionId_ > maxAuctionId) {
                maxAuctionId = info.auctionId_;
            }
        }
    }
    generatorAuctionId_ = maxAuctionId;

    AuctionService::instance(this);
    return true;
}


ErrorCode AuctionService::createAuction(go::Entity& seller, ObjectId npcId,
    const CreateAuctionInfo& auctionInfo)
{
    if (! auctionInfo.isValid()) {
        return ecAuctionInvalidAuctionInfo;
    }

    ItemInfo itemInfo = seller.queryInventoryable()->getInventory().getItemInfo(auctionInfo.itemId_);
    if (! itemInfo.isValid()) {
        return ecAuctionInvalidItem;
    }   
    
    if (itemInfo.isEquipped()) {
        return ecAuctionCannotSellEquippedItem;
    }
    NpcCode npcCode = invalidNpcCode;
    ErrorCode errorCode = getNpcCode(npcCode, seller, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    
    GameMoney depositFee = calculateDepositFree(minimumDepositFree, auctionInfo.duration_);
    if (seller.queryMoneyable()->getGameMoney() < depositFee) {
        return ecAuctionNotEnoughGameMoney;
    }

    FullAuctionInfo fullAuctionInfo;
    fullAuctionInfo.itemInfo_ = itemInfo;
    fullAuctionInfo.auctionId_ = ++generatorAuctionId_;
    fullAuctionInfo.expireTime_ = getTime() + getAuctionDurationSec(auctionInfo.duration_);
    fullAuctionInfo.sellerId_ = seller.getObjectId();
    fullAuctionInfo.buyoutMoney_ = auctionInfo.buyoutPrice_;
    fullAuctionInfo.currentBidMoney_ = auctionInfo.bidPrice_;
    fullAuctionInfo.depositFee_ = calculateDepositFree(minimumDepositFree, auctionInfo.duration_);


    AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (! auctionHouse) {
        auctionHouse = new AuctionHouse(zoneId_, npcCode);
        auctionHouseMap_.emplace(npcCode, auctionHouse);
    }
    
    errorCode = auctionHouse->createAuction(fullAuctionInfo);
    if (isSucceeded(errorCode)) {
        seller.queryMoneyable()->downGameMoney(depositFee);
        seller.queryInventoryable()->getInventory().removeItem(itemInfo.itemId_);
    }
    return errorCode;
}


ErrorCode AuctionService::bid(go::Entity& owner, ObjectId npcId, AuctionId auctionId, GameMoney gameMoney)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    
    AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        return auctionHouse->bid(owner, auctionId, gameMoney);
    }

    return ecAuctionInvalidAuction;
}


ErrorCode AuctionService::buyOut(go::Entity& owner, ObjectId npcId, AuctionId auctionId)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    
    AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        return auctionHouse->buyOut(owner, auctionId);
    }
    return ecAuctionInvalidAuction;
}


ErrorCode AuctionService::cancelAuction(go::Entity& owner, ObjectId npcId, AuctionId auctionId)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        return auctionHouse->cancelAuction(owner, auctionId);
    }
    return ecAuctionInvalidAuction;
}


ErrorCode AuctionService::searchEquipmentByEquipType(AuctionInfos& auctionInfos,
    go::Entity& owner, ObjectId npcId, EquipType equipType)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        auctionHouse->searchEquipmentByEquipType(auctionInfos, equipType);
    }
    
    return ecOk;
}


ErrorCode AuctionService::searchEquipmentByEquipPart(AuctionInfos& auctionInfos,
    go::Entity& owner, ObjectId npcId, EquipPart equipPart)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        auctionHouse->searchEquipmentByEquipPart(auctionInfos, equipPart);
    }
    
    return ecOk;
}


ErrorCode AuctionService::searchWeapon(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        auctionHouse->searchWeapon(auctionInfos);
    }

    return ecOk;
}


ErrorCode AuctionService::searchArmor(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        auctionHouse->searchArmor(auctionInfos);
    }

    return ecOk;
}


ErrorCode AuctionService::searchItemsByCodeType(AuctionInfos& auctionInfos,
    go::Entity& owner, ObjectId npcId, CodeType codeType)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        auctionHouse->searchItemsByCodeType(auctionInfos, codeType);
    }

    return ecOk;
}


ErrorCode AuctionService::querySellingItems(AuctionInfos& auctionInfos,
    go::Entity& owner, ObjectId npcId)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        auctionHouse->querySellingItems(auctionInfos, owner.getObjectId());
    }

    return ecOk;
}


ErrorCode AuctionService::queryBiddingItems(AuctionInfos& auctionInfos, go::Entity& owner, 
    ObjectId npcId)
{
    NpcCode npcCode = invalidNpcCode;
    const ErrorCode errorCode = getNpcCode(npcCode, owner, npcId);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    const AuctionHouse* auctionHouse = getAuctionHouse(npcCode);
    if (auctionHouse) {
        auctionHouse->queryBiddingItems(auctionInfos, owner.getObjectId());
    }

    return ecOk;
}


AuctionHouse* AuctionService::getAuctionHouse(NpcCode npcCode)
{
    AuctionHouseMap::iterator pos =  auctionHouseMap_.find(npcCode);
    if (pos != auctionHouseMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


const AuctionHouse* AuctionService::getAuctionHouse(NpcCode npcCode) const
{
    AuctionHouseMap::const_iterator pos =  auctionHouseMap_.find(npcCode);
    if (pos != auctionHouseMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


}} // namespace gideon { namespace zoneserver {
