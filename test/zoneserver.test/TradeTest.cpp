#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerTradeController.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/PropertyTable.h>

/**
* @class TradeTest
*
* 거래 테스트
*/
class TradeTest : public GameTestFixture
{ 
};


TEST_F(TradeTest, testRequestNotFindOrderPlayer)
{
	const ObjectId invalidPlayerId = 10;
	playerTradeController1_->requestTrade(invalidPlayerId);
	
	ASSERT_EQ(ecEntityNotFound, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(0, playerTradeController1_->getCallCount("evTradeRequested"));
	ASSERT_EQ(1, playerTradeController1_->getCallCount("onRequestTrade"));
}


TEST_F(TradeTest, testRequestFarDistance)
{
    const float32_t maxDistance =
        GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"trade_max_accept_distance");
    ObjectPosition position = player1_->getPosition();
    position.x_ += (maxDistance + 1.0f);
	playerMoveController1_->move(position);
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);

	ASSERT_EQ(ecTradeTargetFarDistance, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(0, playerTradeController1_->getCallCount("evTradeRequested"));
	ASSERT_EQ(1, playerTradeController1_->getCallCount("onRequestTrade"));
	ASSERT_EQ(0, playerTradeController2_->getCallCount("evTradeRequested"));
}


TEST_F(TradeTest, testDuplicateRequestTrade)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);

	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(0, playerTradeController1_->getCallCount("evTradeRequested"));
	ASSERT_EQ(1, playerTradeController1_->getCallCount("onRequestTrade"));
	ASSERT_EQ(1, playerTradeController2_->getCallCount("evTradeRequested"));

	playerTradeController1_->requestTrade(taragetId);

	ASSERT_EQ(ecTradeCannotSelfTradeState, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(0, playerTradeController1_->getCallCount("evTradeRequested"));
	ASSERT_EQ(2, playerTradeController1_->getCallCount("onRequestTrade"));
	ASSERT_EQ(1, playerTradeController2_->getCallCount("evTradeRequested"));

}


TEST_F(TradeTest, testRefusalTrade)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);

	playerTradeController2_->respondTrade(playerTradeController2_->lastTradePlayerId_, false);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);
	
	playerTradeController1_->requestTrade(taragetId);

	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
}


TEST_F(TradeTest, testAcceptTrade)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);

	playerTradeController2_->respondTrade(playerTradeController2_->lastTradePlayerId_, true);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	playerTradeController1_->requestTrade(taragetId);

	ASSERT_EQ(ecTradeCannotSelfTradeState, playerTradeController1_->lastErrorCode_);
}


TEST_F(TradeTest, testRequestAfterCancelTrade)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(1, playerTradeController2_->getCallCount("evTradeRequested"));

	playerTradeController1_->cancelTrade();
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(1, playerTradeController2_->getCallCount("evTradeCancelled"));

	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(2, playerTradeController2_->getCallCount("evTradeRequested"));
}


TEST_F(TradeTest, testEquipItem)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	playerTradeController2_->respondTrade(playerTradeController2_->lastTradePlayerId_, true);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);
	const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
	const ItemInfo* helmet = getItemInfo(invenBefore,
		servertest::defaultHelmetEquipCode);
	ASSERT_TRUE(helmet != nullptr);
	playerTradeController1_->addTradeItem(helmet->itemId_);
	ASSERT_EQ(ecTradeCannotTradeEquipped, playerTradeController1_->lastErrorCode_);
}


TEST_F(TradeTest, testRemoveItem)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	playerTradeController2_->respondTrade(playerTradeController2_->lastTradePlayerId_, true);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	const InventoryInfo invenBefore = player1_->queryInventoryable()->getInventoryInfo();
	const ItemInfo* helmet = getItemInfo(invenBefore,
		servertest::otherHelmetEquipCode);
	ASSERT_TRUE(helmet != nullptr);

	playerTradeController1_->addTradeItem(helmet->itemId_);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);

	playerTradeController1_->addTradeItem(helmet->itemId_);
	ASSERT_EQ(ecTradeAlreadyRegistItem, playerTradeController1_->lastErrorCode_);

	playerTradeController1_->removeTradeItem(helmet->itemId_);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(1, playerTradeController2_->getCallCount("evTradeItemRemoved"));

	playerTradeController1_->addTradeItem(helmet->itemId_);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
}


TEST_F(TradeTest, testNotEnoughGameMoney)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	playerTradeController2_->respondTrade(playerTradeController2_->lastTradePlayerId_, true);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);
	
	playerTradeController1_->toggleTradeReady(10001);
	ASSERT_EQ(ecTradeNotEnoughTradeGameMoney, playerTradeController1_->lastErrorCode_);

	playerTradeController1_->toggleTradeReady(10000);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
}


TEST_F(TradeTest, testFullInventory)
{
	/*const ObjectId stackCount = 255;
	BaseItemInfo skillPackageItem(servertest::skillPackage_1, stackCount);

	int i = 0;
	while (true) {
		++i;
		playerInventoryController1_->addInventoryItem(skillPackageItem, invalidSlotId);
		if (i != playerInventoryController1_->getCallCount("evInventoryItemAdded")) {
			break;
		}
	}
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	playerTradeController2_->respondTrade(playerTradeController2_->lastTradePlayerId_, true);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	const InventoryInfo invenBefore = player2_->queryInventoryable()->getInventoryInfo();
	const ItemInfo* helmet = getItemInfo(invenBefore,
		servertest::otherHelmetEquipCode);
	ASSERT_TRUE(helmet != nullptr);

	playerTradeController2_->addTradeItem(helmet->itemId_);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	playerTradeController1_->toggleTradeReady(0);
	playerTradeController2_->toggleTradeReady(0);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	playerTradeController2_->toggleTradeConform(0);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);
	
	playerTradeController1_->toggleTradeConform(0);
	ASSERT_EQ(ecTradeSelfNotEnoughInventory, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(1, playerTradeController1_->getCallCount("evTradeConformToggled"));
	ASSERT_EQ(0, playerTradeController2_->getCallCount("evTradeConformToggled"));

	playerTradeController2_->toggleTradeConform(0);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);
	playerTradeController1_->toggleTradeConform(0);
	playerTradeController2_->toggleTradeConform(0);
	ASSERT_EQ(ecTradeOrderNotEnoughInventory, playerTradeController2_->lastErrorCode_);*/
}


TEST_F(TradeTest, testTradeItem)
{
	const ObjectId taragetId = 2;
	playerTradeController1_->requestTrade(taragetId);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	playerTradeController2_->respondTrade(playerTradeController2_->lastTradePlayerId_, true);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	const InventoryInfo invenBefore = player2_->queryInventoryable()->getInventoryInfo();
	const ItemInfo* helmet = getItemInfo(invenBefore,
		servertest::otherHelmetEquipCode);
	ASSERT_TRUE(helmet != nullptr);

	playerTradeController2_->addTradeItem(helmet->itemId_);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	playerTradeController1_->toggleTradeReady(0);
	playerTradeController2_->toggleTradeReady(0);
	ASSERT_EQ(ecOk, playerTradeController1_->lastErrorCode_);
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	playerTradeController2_->toggleTradeConform();
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	playerTradeController1_->toggleTradeConform();
	ASSERT_EQ(ecOk, playerTradeController2_->lastErrorCode_);

	ASSERT_EQ(1, playerInventoryController2_->getCallCount("evInventoryItemRemoved"));
}
