#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "MockPlayerItemController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerMoveController.h"
#include "ZoneServer/model/gameobject/Player.h"

/**
* @class GliderTest
*
* 글라이더 테스트
*/
class GliderTest : public GameTestFixture
{ 
};


TEST_F(GliderTest, testUseGliderItem)
{

}


TEST_F(GliderTest, testMountGlider)
{
	StartCastInfo castInfo;
	castInfo.set(uctGlider);
	playerCastController1_->startCasting(castInfo);
	ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
    ASSERT_EQ(ecOk, playerCastController1_->lastErrorCode_);
	ASSERT_EQ(1, playerCastController1_->getCallCount("evCastCompleted"));
	ASSERT_EQ(1, playerCastController2_->getCallCount("evCastCompleted"));


    const UnionEntityInfo& creatureInfo = player1_->getUnionEntityInfo();

    ASSERT_EQ(true, creatureInfo.asCreatureInfo().stateInfo_.hasState(cstGliderMount));
	playerCastController1_->startCasting(castInfo);
    ASSERT_EQ(ecGliderCannotReadyMountState, playerCastController1_->lastErrorCode_);
	
	ASSERT_EQ(2, playerCastController1_->getCallCount("onStartCasting"));
	ASSERT_EQ(1, playerCastController1_->getCallCount("evCastCompleted"));
	ASSERT_EQ(1, playerCastController2_->getCallCount("evCastCompleted"));
}


TEST_F(GliderTest, testDismountGlider)
{
    playerMoveController1_->dismountGlider();
    ASSERT_EQ(0, playerMoveController2_->getCallCount("evGliderDismounted"));

	StartCastInfo castInfo;
	castInfo.set(uctGlider);
	playerCastController1_->startCasting(castInfo);

    playerMoveController1_->dismountGlider();

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evGliderDismounted"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evGliderDismounted"));

    const UnionEntityInfo& creatureInfo = player1_->getUnionEntityInfo();
    ASSERT_EQ(false, creatureInfo.asCreatureInfo().stateInfo_.hasState(cstGliderMount));
}


TEST_F(GliderTest, testCannotMountGliderWithVehicle)
{
	StartCastInfo castInfo;
	castInfo.set(uctVehicle);
	playerCastController1_->startCasting(castInfo);

	castInfo.set(uctGlider);
	playerCastController1_->startCasting(castInfo);
    ASSERT_EQ(ecGliderCannotReadyMountState, playerCastController1_->lastErrorCode_);

    const UnionEntityInfo& creatureInfo = player1_->getUnionEntityInfo();
    ASSERT_EQ(false, creatureInfo.asCreatureInfo().stateInfo_.hasState(cstGliderMount));
}
