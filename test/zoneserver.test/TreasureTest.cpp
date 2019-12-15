#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerTreasureController.h"
#include "MockPlayerCastController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/cs/datatable/TreasureTable.h>

/**
* @class TreasureTest
*
* 보물 관련 테스트
*/
class TreasureTest : public GameTestFixture
{ 
};


TEST_F(TreasureTest, testAcquireTreasure)
{
	ASSERT_EQ(0, playerController1_->getCallCount("evEntityDisappeared"));
    StartCastInfo castInfo;
    castInfo.set(uctTreasure, GameObjectInfo(otTreasure, 1));
	playerCastController1_->startCasting(castInfo);
    ASSERT_EQ(ecOk, playerCastController1_->lastErrorCode_);

    // 즉시 시전이기 때문에
    ASSERT_EQ(0, playerCastController1_->getCallCount("evCasted"));
    ASSERT_EQ(1, playerTreasureController1_->getCallCount("evTreasureOpened"));

    //LootInvenItemInfoMap::const_iterator pos = playerTreasureController1_->lastTreasureItemMap_.begin();
    //LootInvenItemInfoMap::const_iterator end = playerTreasureController1_->lastTreasureItemMap_.end();
    //for (; pos != end; ++pos) {
    //    const LootInvenItemInfo& info = (*pos).second;
    //    if (info.baseItemInfo_.isValid()) {
    //        playerTreasureController1_->acquireTreasure(1, uint8_t((*pos).first));
    //        ASSERT_EQ(ecOk, playerTreasureController1_->lastErrorCode_);
    //        ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
    //        break;
    //    }
    //}
    playerTreasureController1_->closeTreasure(1);
    ASSERT_EQ(ecOk, playerTreasureController1_->lastErrorCode_);
    ASSERT_EQ(1, playerTreasureController1_->getCallCount("onCloseTreasure"));
	ASSERT_EQ(1, playerController1_->getCallCount("evEntityDisappeared"));
}


TEST_F(TreasureTest, testCancelTreasure)
{
    StartCastInfo castInfo;
    castInfo.set(uctTreasure, GameObjectInfo(otTreasure, 2));
    playerCastController1_->startCasting(castInfo);
    ASSERT_EQ(1, playerCastController1_->getCallCount("evCasted"));
    ASSERT_EQ(ecOk, playerCastController1_->lastErrorCode_);
    ObjectPosition position = player1_->getPosition();
    position.x_ = position.x_ + 1.0f;
    // TODO :Task 작업 볼것
    //playerMoveController1_->move(position);
    //ASSERT_EQ(1, playerCastController1_->getCallCount("evCastCanceled"));
    //ASSERT_EQ(0, playerTreasureController1_->getCallCount("evTreasureOpened"));
}