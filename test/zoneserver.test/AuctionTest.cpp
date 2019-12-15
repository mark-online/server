#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerAuctionController.h"
#include "MockPlayerMailController.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/model/gameobject/Npc.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>

using gideon::servertest::MockProxyGameDatabase;

using namespace gideon::zoneserver;
/**
* @class AuctionTest
*
* 옥션 관련 테스트
*/
class AuctionTest : public GameTestFixture
{
public:
    ObjectId getNpcId(go::Player* player, NpcCode npcCode);
};


ObjectId AuctionTest::getNpcId(go::Player* player, NpcCode npcCode)
{
    go::Knowable* knowable = player->queryKnowable();
    go::EntityMap entityMap = knowable->getKnownEntities();
    for (go::EntityMap::value_type& value : entityMap) {
        go::Entity* entity = value.second;
        if (entity->isNpc()) {
            if (entity->getEntityCode() == npcCode) {
                return entity->getObjectId();
            }
        }
    }
    return invalidObjectId;
}

TEST_F(AuctionTest, testCreateAuction)
{    
    const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* helmet = getItemInfo(invenBefore,
        servertest::otherHelmetEquipCode);
    
    CreateAuctionInfo auctionInfo;
    auctionInfo.itemId_ =  helmet->itemId_;
    auctionInfo.bidPrice_ =  500;
    auctionInfo.buyoutPrice_ =  1000;
    auctionInfo.duration_ = adt12Hour;

    playerAuctionController1_->createAuction(getNpcId(player1_, servertest::storeNpcCode),
        auctionInfo);
    ASSERT_EQ(ecOk, playerAuctionController1_->lastErrorCode_);

    ASSERT_EQ(1, playerAuctionController1_->getCallCount("onCreateAuction"));
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));

    sne::database::Guard<MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("createAuction"));
}


TEST_F(AuctionTest, testBid)
{
    ObjectId npcId = getNpcId(player2_, servertest::storeNpcCode);
    playerAuctionController2_->searchItemsByCodeType(npcId, ctEquipment);

    ASSERT_EQ(ecOk, playerAuctionController2_->lastErrorCode_);
    ASSERT_EQ(1, playerAuctionController2_->getCallCount("onSearchItemsByCodeType"));
    AuctionInfos equipInfos = playerAuctionController2_->auctionInfos_;
    ASSERT_EQ(2, equipInfos.size());

    playerAuctionController2_->bid(npcId, 1, 50);
    ASSERT_EQ(ecAuctionBidHigherCurrentBid, playerAuctionController2_->lastErrorCode_);

    playerAuctionController2_->bid(npcId, 1, 150);
    ASSERT_EQ(ecOk, playerAuctionController2_->lastErrorCode_);

    ASSERT_EQ(0, playerMailController2_->getCallCount("evMailReceived"));

    playerAuctionController2_->bid(npcId, 1, 200);
    ASSERT_EQ(ecOk, playerAuctionController2_->lastErrorCode_);

    ASSERT_EQ(1, playerMailController2_->getCallCount("evMailReceived"));

    playerAuctionController1_->querySellingItems(npcId);
    ASSERT_EQ(3, playerAuctionController1_->auctionInfos_.size());
    playerAuctionController1_->queryBiddingItems(npcId);
    ASSERT_EQ(0, playerAuctionController1_->auctionInfos_.size());
    playerAuctionController2_->querySellingItems(npcId);
    ASSERT_EQ(0, playerAuctionController2_->auctionInfos_.size());
    playerAuctionController2_->queryBiddingItems(npcId);
    ASSERT_EQ(1, playerAuctionController2_->auctionInfos_.size());
}


TEST_F(AuctionTest, testBuyout)
{
    ObjectId npcId = getNpcId(player2_, servertest::storeNpcCode);
    playerAuctionController2_->searchItemsByCodeType(npcId, ctEquipment);
    ASSERT_EQ(ecOk, playerAuctionController2_->lastErrorCode_);
    ASSERT_EQ(1, playerAuctionController2_->getCallCount("onSearchItemsByCodeType"));
    AuctionInfos equipInfos = playerAuctionController2_->auctionInfos_;
    ASSERT_EQ(2, equipInfos.size());
    const AuctionInfo& auctionInfo = (*equipInfos.begin());
    playerAuctionController2_->bid(npcId, 1, 200);
    ASSERT_EQ(ecOk, playerAuctionController2_->lastErrorCode_);
    GameMoney oldGameMoney = player2_->queryMoneyable()->getGameMoney();
    playerAuctionController2_->buyOut(npcId, auctionInfo.auctionId_);
    ASSERT_EQ(ecOk, playerAuctionController2_->lastErrorCode_);  
    ASSERT_EQ(oldGameMoney, player2_->queryMoneyable()->getGameMoney() + auctionInfo.buyoutMoney_);

    playerAuctionController2_->searchItemsByCodeType(npcId, ctEquipment);
    ASSERT_EQ(1, playerAuctionController2_->auctionInfos_.size());
    ASSERT_EQ(2, playerMailController2_->getCallCount("evMailReceived"));

    playerAuctionController1_->querySellingItems(npcId);
    ASSERT_EQ(2, playerAuctionController1_->auctionInfos_.size());
    playerAuctionController1_->queryBiddingItems(npcId);
    ASSERT_EQ(0, playerAuctionController1_->auctionInfos_.size());
    playerAuctionController2_->querySellingItems(npcId);
    ASSERT_EQ(0, playerAuctionController2_->auctionInfos_.size());
    playerAuctionController2_->queryBiddingItems(npcId);
    ASSERT_EQ(0, playerAuctionController2_->auctionInfos_.size());
}
