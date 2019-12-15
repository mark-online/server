#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerHarvestController.h"
#include "MockPlayerCastController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/HarvestTable.h>

/**
* @class HarvestTest
*
* 채집 관련 테스트
*/
class HarvestTest : public GameTestFixture
{ 
};


TEST_F(HarvestTest, testAcquireHarvest)
{
	ASSERT_EQ(0, playerController1_->getCallCount("evEntityDisappeared"));
	
    StartCastInfo castInfo;
    castInfo.set(uctHavest, GameObjectInfo(otHarvest, 1));
    playerCastController1_->startCasting(castInfo);
    ASSERT_EQ(ecOk, playerCastController1_->lastErrorCode_);

    // 즉시 시전 이기때문에
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    ASSERT_EQ(1, playerHarvestController1_->getCallCount("evHarvestRewarded"));
	ASSERT_TRUE(1 < playerHarvestController1_->lastHarvestItems_.size());
	
    int i = 0;
	for (const BaseItemInfo& info : playerHarvestController1_->lastHarvestItems_) {
		playerHarvestController1_->acquireHarvest(info.itemCode_);
		ASSERT_EQ(++i, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
		ASSERT_EQ(0, playerHarvestController1_->getCallCount("onStartHarvestonAcquireHarvest"));
	}

	ASSERT_EQ(1, playerController1_->getCallCount("evEntityDisappeared"));
}


TEST_F(HarvestTest, testCancelHarvest)
{
 //   StartCastInfo castInfo;
 //   castInfo.set(uctHavest, GameObjectInfo(otHarvest, 2));
 //   playerCastController1_->startCasting(castInfo);
 //   
 //   // 즉시 시전이 아니기때문에
 //   ASSERT_EQ(1, playerCastController1_->getCallCount("evCasted"));

 //   ASSERT_EQ(ecOk, playerCastController1_->lastErrorCode_);

 //   ObjectPosition position = player1_->getPosition();
 //   position.x_ = position.x_ + 1.0f;

 //   playerMoveController1_->move(position);
 //   // 테스트에서는 타이머가 실제랑 다르게 돌아가기 때문에
	////ASSERT_EQ(1, playerCastController1_->getCallCount("evCastCanceled"));
 //   ASSERT_EQ(0, playerHarvestController1_->getCallCount("evTreasureOpened"));
}
