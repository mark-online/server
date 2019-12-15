#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include <gideon/cs/shared/rpc/player/AuctionRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
    class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {


/**
 * @class PlayerAuctionController
 * 경매 담당
 */
class ZoneServer_Export PlayerAuctionController : public Controller,
    public rpc::AuctionRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
public:
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerAuctionController);
public:
    PlayerAuctionController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

public:
    OVERRIDE_SRPC_METHOD_2(createAuction,
        ObjectId, npcId, CreateAuctionInfo, auctionInfo);
    OVERRIDE_SRPC_METHOD_2(searchEquipmentByEquipType,
        ObjectId, npcId, EquipType, equipType);
    OVERRIDE_SRPC_METHOD_2(searchEquipmentByEquipPart,
        ObjectId, npcId, EquipPart, equipPart);
    OVERRIDE_SRPC_METHOD_1(searchWeapon,
        ObjectId, npcId);
    OVERRIDE_SRPC_METHOD_1(searchArmor,
        ObjectId, npcId);
    OVERRIDE_SRPC_METHOD_2(searchItemsByCodeType,
        ObjectId, npcId, CodeType, codeType);
    OVERRIDE_SRPC_METHOD_1(querySellingItems,
        ObjectId, npcId);
    OVERRIDE_SRPC_METHOD_1(queryBiddingItems,
        ObjectId, npcId);
    OVERRIDE_SRPC_METHOD_3(bid,
        ObjectId, npcId, AuctionId, auctionId, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_2(buyOut,
        ObjectId, npcId, AuctionId, auctionId);
    OVERRIDE_SRPC_METHOD_2(cancelAuction,
        ObjectId, npcId, AuctionId, auctionId);

public:
    OVERRIDE_SRPC_METHOD_2(onCreateAuction,
        ErrorCode, errorCode, GameMoney, currentMoney);
    OVERRIDE_SRPC_METHOD_2(onSearchEquipmentByEquipType,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onSearchEquipmentByEquipPart,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onSearchWeapon,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onSearchArmor,
        ErrorCode, errorCode, AuctionInfos, infos);

    OVERRIDE_SRPC_METHOD_2(onSearchItemsByCodeType,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onSellingItems,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_2(onBiddingItems,
        ErrorCode, errorCode, AuctionInfos, infos);
    OVERRIDE_SRPC_METHOD_4(onBid,
        ErrorCode, errorCode, AuctionId, auctionId, GameMoney, bidMoney, GameMoney, currentMoney);
    OVERRIDE_SRPC_METHOD_3(onBuyOut,
        ErrorCode, errorCode, AuctionId, auctionId, GameMoney, currentMoney);
    OVERRIDE_SRPC_METHOD_2(onCancelAuction,
        ErrorCode, errorCode, AuctionId, auctionId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {

