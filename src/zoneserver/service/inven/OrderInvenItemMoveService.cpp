#include "ZoneServerPCH.h"
#include "OrderInvenItemMoveService.h"
#include "../../model/item/Inventory.h"
#include "../../helper/InventoryHelper.h"

namespace gideon { namespace zoneserver {

namespace {

ErrorCode moveItem(Inventory& src, Inventory& target, ObjectId itemId, SlotId slotId)
{
    ItemInfo invenItem = src.getItemInfo(itemId);
    if (! invenItem.isValid()) {
        return ecInventoryItemNotFound;
    }

    if (invenItem.isEquipped()) {
        return ecInventoryCannotChangeEquippedItemSlot;
    }

    ErrorCode errorCode = target.checkAddSlot(slotId);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    if (! target.canAddItemByBaseItemInfo(invenItem)) {
        return ecInventoryIsFull;
    }

    src.removeItem(itemId);
    return target.addItem(createAddItemInfoByItemInfo(invenItem), slotId);
}

} // namespace {

SNE_DEFINE_SINGLETON(OrderInvenItemMoveService)

ErrorCode OrderInvenItemMoveService::playerInvenToOutsideInven(Inventory& playerInven,
    Inventory& buildingInven, ObjectId playerItemId, SlotId buildingSlotId)
{
    return moveItem(playerInven, buildingInven, playerItemId, buildingSlotId);
}


ErrorCode OrderInvenItemMoveService::outsideInvenToPlayerInven(Inventory& playerInven,
    Inventory& buildingInven, ObjectId buildingItemId, SlotId playerSlotId)
{
    return moveItem(buildingInven, playerInven, buildingItemId, playerSlotId);
}


ErrorCode OrderInvenItemMoveService::switchStaticObjectAndPlayerInven(Inventory& playerInven,
    Inventory& buildingInven, ObjectId invenItemId, ObjectId buildingItemId)
{
    // 메모리 복사 이유는 프로시져를 공요으로 쓰기때문에 먼저 지우고 인서트를 하기때문에~

    ItemInfo buildingItem = buildingInven.getItemInfo(buildingItemId);
    if (! buildingItem.isValid()) {
        return ecInventoryItemNotFound;
    }
    ItemInfo invenItem = playerInven.getItemInfo(invenItemId);
    if (! invenItem.isValid()) {
        return ecInventoryItemNotFound;
    }

    if (invenItem.isEquipped()) {
        return ecInventoryCannotChangeEquippedItemSlot;
    }

    playerInven.removeItem(invenItemId);
    buildingInven.removeItem(buildingItemId);

    playerInven.addItem(createAddItemInfoByItemInfo(buildingItem), invenItem.slotId_);    
    buildingInven.addItem(createAddItemInfoByItemInfo(invenItem), buildingItem.slotId_);
    

    return ecOk;
}

//
//ErrorCode OrderInvenItemMoveService::depositMoney(go::Player& owner, GameMoney money)
//{
//    ACE_GUARD_RETURN(go::Entity::LockType, lock, owner.getLock(), ecServerInternalError);
//
//    if (owner.getGameMoney() < money) {
//        return ecBankDipositNotEnoughGameMoney;
//    }
//
//    owner.downGameMoney(money);
//    BankAccount& bankAccount = owner.getBankAccount();	
//    bankAccount.depositMoney(money);
//
//    return ecOk;
//}
//
//
//ErrorCode OrderInvenItemMoveService::withdrawMoney(go::Player& owner, GameMoney money)
//{
//    ACE_GUARD_RETURN(go::Entity::LockType, lock, owner.getLock(), ecServerInternalError);
//
//    BankAccount& bankAccount = owner.getBankAccount();	
//    if (bankAccount.getGameMoney() < money) {
//        return ecBankWithdrawNotEnoughGameMoney;
//    }
//
//    bankAccount.withdrawMoney(money);
//    owner.upGameMoney(money);
//    return ecOk;
//}

}} // namespace gideon { namespace zoneserver {
