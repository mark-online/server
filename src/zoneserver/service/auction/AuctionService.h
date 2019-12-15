#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/AuctionInfo.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <sne/core/utility/Singleton.h>
#include <atomic>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go


class AuctionHouse;

/**
 * @class AuctionService
 *
 * 경매 서비스
 */
class ZoneServer_Export AuctionService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(AuctionService);
    typedef sne::core::HashMap<NpcCode, AuctionHouse*> AuctionHouseMap;
public:
    AuctionService(ZoneId zoneId);
    ~AuctionService();

    bool initialize(const ServerAuctionInfoMap& auctionMap);

    ErrorCode createAuction(go::Entity& seller, ObjectId npcId,
        const CreateAuctionInfo& auctionInfo);
    ErrorCode bid(go::Entity& owner, ObjectId npcId, AuctionId auctionId, GameMoney gameMoney);
    ErrorCode buyOut(go::Entity& owner, ObjectId npcId, AuctionId auctionId);
    ErrorCode cancelAuction(go::Entity& owner, ObjectId npcId, AuctionId auctionId);

    ErrorCode searchEquipmentByEquipType(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId, EquipType equipType);
    ErrorCode searchEquipmentByEquipPart(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId, EquipPart equipPart);
    ErrorCode searchWeapon(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId);
    ErrorCode searchArmor(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId);

    ErrorCode searchItemsByCodeType(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId, CodeType codeType);
    ErrorCode querySellingItems(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId);
    ErrorCode queryBiddingItems(AuctionInfos& auctionInfos, go::Entity& owner, ObjectId npcId);


private:
    AuctionHouse* getAuctionHouse(NpcCode npcCode);
    const AuctionHouse* getAuctionHouse(NpcCode npcCode) const;

private:
    ZoneId zoneId_;
    std::atomic<AuctionId> generatorAuctionId_;
    AuctionHouseMap auctionHouseMap_;
};

}} // namespace gideon { namespace zoneserver {

#define AUCTION_SERVICE gideon::zoneserver::AuctionService::instance()
