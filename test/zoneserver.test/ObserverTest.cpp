#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerController.h"
#include "MockPlayerCastController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/MockSkillTable.h>

/**
* @class ObserverTest
*
* 관찰자 관련 테스트
*/
class ObserverTest : public GameTestFixture
{
};


TEST_F(ObserverTest, testNotResistOberver)
{
    //ASSERT_EQ(0, playerController1_->getCallCount("evPointChanged"));
    //ASSERT_EQ(0, playerController2_->getCallCount("evPointChanged"));

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), 
    //    servertest::testObserverSkillCode));

    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));

    //ASSERT_EQ(0, playerController1_->getCallCount("evPointChanged"));
    //ASSERT_EQ(1, playerController2_->getCallCount("evPointChanged"));
}


TEST_F(ObserverTest, testResistOberver)
{
    //playerController1_->selectTarget(GameObjectInfo(otPc, characterId2_));

    //ASSERT_EQ(0, playerController1_->getCallCount("evPointChanged"));
    //ASSERT_EQ(0, playerController2_->getCallCount("evPointChanged"));

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), 
    //    servertest::testObserverSkillCode));

    //ASSERT_EQ(1, playerController1_->getCallCount("evPointChanged"));
    //ASSERT_EQ(1, playerController2_->getCallCount("evPointChanged"));
}


TEST_F(ObserverTest, testUnresistOberver)
{
    //playerController1_->selectTarget(GameObjectInfo(otPc, characterId2_));
    //playerController1_->selectTarget(GameObjectInfo());

    //playerCastController1_->startCasting(createUnionSkillCastToInfo(GameObjectInfo(otPc, characterId2_), 
    //    servertest::testObserverSkillCode));

    //ASSERT_EQ(1, playerCastController1_->getCallCount("onStartCasting"));

    //ASSERT_EQ(0, playerController1_->getCallCount("evPointChanged"));
    //ASSERT_EQ(1, playerController2_->getCallCount("evPointChanged"));
}