#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerCastController.h"
#include "MockPlayerMoveController.h"
#include "ZoneServer/model/gameobject/Player.h"

/**
* @class VehicleTest
*
* 탈 것 테스트
*/
class VehicleTest : public GameTestFixture
{ 
};


TEST_F(VehicleTest, testMountVehicle)
{
	StartCastInfo castInfo;
	castInfo.set(uctVehicle);
	playerCastController1_->startCasting(castInfo);
	ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));
	ASSERT_EQ(ecOk, playerCastController1_->lastErrorCode_);
	ASSERT_EQ(1, playerCastController1_->getCallCount("evCastCompleted"));
	ASSERT_EQ(1, playerCastController2_->getCallCount("evCastCompleted"));


	const UnionEntityInfo& creatureInfo = player1_->getUnionEntityInfo();

	ASSERT_EQ(true, creatureInfo.asCreatureInfo().stateInfo_.hasState(cstVehicleMount));
	playerCastController1_->startCasting(castInfo);
	ASSERT_EQ(ecVehicleCannotMountState, playerCastController1_->lastErrorCode_);

	ASSERT_EQ(2, playerCastController1_->getCallCount("onStartCasting"));
	ASSERT_EQ(1, playerCastController1_->getCallCount("evCastCompleted"));
	ASSERT_EQ(1, playerCastController2_->getCallCount("evCastCompleted"));
}


TEST_F(VehicleTest, testDismountVehicle)
{
    playerMoveController1_->dismountVehicle(false);
    ASSERT_EQ(0, playerMoveController1_->getCallCount("evVehicleDismounted"));
    ASSERT_EQ(0, playerMoveController2_->getCallCount("evVehicleDismounted"));

	StartCastInfo castInfo;
	castInfo.set(uctVehicle);
	playerCastController1_->startCasting(castInfo);
    playerMoveController1_->dismountVehicle(false);

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evVehicleDismounted"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evVehicleDismounted"));

    const UnionEntityInfo& creatureInfo = player1_->getUnionEntityInfo();
    ASSERT_EQ(false, creatureInfo.asCreatureInfo().stateInfo_.hasState(cstVehicleMount));


	playerMoveController1_->dismountGlider();
	ASSERT_EQ(0, playerMoveController2_->getCallCount("evGliderDismounted"));
}
