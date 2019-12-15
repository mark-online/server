#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerController.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerSkillController.h"

/**
* @class CheatTest
*
*  치트 관련 테스트
*/
class CheatTest : public GameTestFixture
{
};


TEST_F(CheatTest, testAddItem)
{
    playerController1_->cheat(L"/additem 150994949 255");
    
    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemAdded"));
}


TEST_F(CheatTest, testRefillFullHp)
{
    playerController1_->cheat(L"/fullpoints");

    ASSERT_EQ(1, playerController1_->getCallCount("evPointsRestored"));
}


TEST_F(CheatTest, testRewardExp)
{
    playerController1_->cheat(L"/rewardexp 10000");

    ASSERT_EQ(1, playerController1_->getCallCount("evPlayerLeveledUpInfo"));
    ASSERT_EQ(1, playerController2_->getCallCount("evPlayerLeveledUp"));
}


TEST_F(CheatTest, testAddskill)
{
    playerController1_->cheat(L"/addskill 16781058");
    ASSERT_EQ(1, playerSkillController1_->getCallCount("evSkillLearned"));
}
