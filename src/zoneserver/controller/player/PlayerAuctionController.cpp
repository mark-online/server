#include "ZoneServerPCH.h"
#include "PlayerAuctionController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../model/item/Inventory.h"
#include "../../service/auction/AuctionService.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>


namespace gideon { namespace zoneserver { namespace gc {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerAuctionController);

PlayerAuctionController::PlayerAuctionController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerAuctionController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerAuctionController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


RECEIVE_SRPC_METHOD_2(PlayerAuctionController, createAuction,
    ObjectId, npcId, CreateAuctionInfo, auctionInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = 
        AUCTION_SERVICE->createAuction(owner, npcId, auctionInfo);
    onCreateAuction(errorCode, owner.queryMoneyable()->getGameMoney());
}


RECEIVE_SRPC_METHOD_2(PlayerAuctionController, searchEquipmentByEquipType,
    ObjectId, npcId, EquipType, equipType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    AuctionInfos auctionInfos;
    const ErrorCode errorCode = 
        AUCTION_SERVICE->searchEquipmentByEquipType(auctionInfos, owner, npcId, equipType);
    onSearchEquipmentByEquipType(errorCode, auctionInfos);
}


RECEIVE_SRPC_METHOD_2(PlayerAuctionController, searchEquipmentByEquipPart,
    ObjectId, npcId, EquipPart, equipPart)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    AuctionInfos auctionInfos;
    const ErrorCode errorCode = 
        AUCTION_SERVICE->searchEquipmentByEquipPart(auctionInfos, owner, npcId, equipPart);
    onSearchEquipmentByEquipPart(errorCode, auctionInfos);
}


RECEIVE_SRPC_METHOD_1(PlayerAuctionController, searchWeapon,
    ObjectId, npcId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    AuctionInfos auctionInfos;
    const ErrorCode errorCode = 
        AUCTION_SERVICE->searchWeapon(auctionInfos, owner, npcId);
    onSearchWeapon(errorCode, auctionInfos);
}


RECEIVE_SRPC_METHOD_1(PlayerAuctionController, searchArmor,
    ObjectId, npcId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    AuctionInfos auctionInfos;
    const ErrorCode errorCode = 
        AUCTION_SERVICE->searchArmor(auctionInfos, owner, npcId);
    onSearchArmor(errorCode, auctionInfos);
}


RECEIVE_SRPC_METHOD_2(PlayerAuctionController, searchItemsByCodeType,
    ObjectId, npcId, CodeType, codeType)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    AuctionInfos auctionInfos;
    const ErrorCode errorCode = 
        AUCTION_SERVICE->searchItemsByCodeType(auctionInfos, owner, npcId, codeType);
    onSearchItemsByCodeType(errorCode, auctionInfos);
}


RECEIVE_SRPC_METHOD_1(PlayerAuctionController, querySellingItems,
    ObjectId, npcId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    AuctionInfos auctionInfos;
    const ErrorCode errorCode = 
        AUCTION_SERVICE->querySellingItems(auctionInfos, owner, npcId);
    onSellingItems(errorCode, auctionInfos);
}


RECEIVE_SRPC_METHOD_1(PlayerAuctionController, queryBiddingItems,
    ObjectId, npcId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    AuctionInfos auctionInfos;
    const ErrorCode errorCode = 
        AUCTION_SERVICE->queryBiddingItems(auctionInfos, owner, npcId);
    onBiddingItems(errorCode, auctionInfos);
}


RECEIVE_SRPC_METHOD_3(PlayerAuctionController, bid,
    ObjectId, npcId, AuctionId, auctionId, GameMoney, gameMoney)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    const ErrorCode errorCode = 
        AUCTION_SERVICE->bid(owner, npcId, auctionId, gameMoney);
    onBid(errorCode, auctionId, gameMoney, owner.queryMoneyable()->getGameMoney());
}


RECEIVE_SRPC_METHOD_2(PlayerAuctionController, buyOut,
    ObjectId, npcId, AuctionId, auctionId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    const ErrorCode errorCode = 
        AUCTION_SERVICE->buyOut(owner, npcId, auctionId);
    onBuyOut(errorCode, auctionId, owner.queryMoneyable()->getGameMoney());
}


RECEIVE_SRPC_METHOD_2(PlayerAuctionController, cancelAuction,
    ObjectId, npcId, AuctionId, auctionId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
    const ErrorCode errorCode = 
        AUCTION_SERVICE->cancelAuction(owner, npcId, auctionId);
    onCancelAuction(errorCode, auctionId);
}


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onCreateAuction,
    ErrorCode, errorCode, GameMoney, currentMoney);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onSearchEquipmentByEquipType,
    ErrorCode, errorCode, AuctionInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onSearchEquipmentByEquipPart,
    ErrorCode, errorCode, AuctionInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onSearchWeapon,
    ErrorCode, errorCode, AuctionInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onSearchArmor,
    ErrorCode, errorCode, AuctionInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onSearchItemsByCodeType,
    ErrorCode, errorCode, AuctionInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onSellingItems,
    ErrorCode, errorCode, AuctionInfos, infos);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onBiddingItems,
    ErrorCode, errorCode, AuctionInfos, infos);


FORWARD_SRPC_METHOD_4(PlayerAuctionController, onBid,
    ErrorCode, errorCode, AuctionId, auctionId, GameMoney, bidMoney, GameMoney, currentMoney);


FORWARD_SRPC_METHOD_3(PlayerAuctionController, onBuyOut,
    ErrorCode, errorCode, AuctionId, auctionId, GameMoney, currentMoney);


FORWARD_SRPC_METHOD_2(PlayerAuctionController, onCancelAuction,
    ErrorCode, errorCode, AuctionId, auctionId);


}}} // namespace gideon { namespace zoneserver { namespace gc {
