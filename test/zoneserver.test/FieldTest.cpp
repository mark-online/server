#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerController.h"
#include "MockPlayerMoveController.h"
#include "ZoneServer/model/gameobject/Player.h"

/**
* @class FieldTest
*
* 필드 상에서의 캐릭터 이동 및 채팅 테스트
*/
class FieldTest : public GameTestFixture
{ 
};



TEST_F(FieldTest, testGoToward)
{
    ObjectPosition position = player1_->getPosition();
    position.x_ = position.x_ + 1.0f;

    playerMoveController1_->goToward(position);

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evEntityGoneToward"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evEntityGoneToward"));

    const UnionEntityInfo& creatureInfo1 = player1_->getUnionEntityInfo();
    ASSERT_EQ(true, creatureInfo1.asEntityMovementInfo().isMoving_);
    ASSERT_FLOAT_EQ(position.x_, creatureInfo1.asEntityMovementInfo().destination_.x_);
}


TEST_F(FieldTest, testMove)
{
    ObjectPosition position = player1_->getPosition();
    position.x_ = position.x_ + 1.0f;

    playerMoveController1_->move(position);

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evEntityMoved"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evEntityMoved"));

    const UnionEntityInfo& creatureInfo1 = player1_->getUnionEntityInfo();
    ASSERT_EQ(false, creatureInfo1.asEntityMovementInfo().isMoving_);
    ASSERT_FLOAT_EQ(position.x_, creatureInfo1.asCharacterInfo().position_.x_);
}


TEST_F(FieldTest, testStop)
{
    ObjectPosition position = player1_->getPosition();
    position.x_ = position.x_ + 1.0f;

    playerMoveController1_->goToward(position);
    playerMoveController1_->stop(position);

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evEntityStopped"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evEntityStopped"));

    const UnionEntityInfo& creatureInfo1 = player1_->getUnionEntityInfo();
    ASSERT_EQ(false, creatureInfo1.asEntityMovementInfo().isMoving_);
    ASSERT_FLOAT_EQ(position.x_, creatureInfo1.asEntityMovementInfo().destination_.x_);
    ASSERT_FLOAT_EQ(position.x_, creatureInfo1.asCharacterInfo().position_.x_);
}


TEST_F(FieldTest, testTurn)
{
    playerMoveController1_->turn(12);

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evEntityTurned"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evEntityTurned"));

    const UnionEntityInfo& creatureInfo1 = player1_->getUnionEntityInfo();
    ASSERT_EQ(12, creatureInfo1.asCharacterInfo().position_.heading_);
}


TEST_F(FieldTest, testJump)
{
    ObjectPosition position = player1_->getPosition();
    position.x_ = position.x_ + 1.0f;

    playerMoveController1_->jump(position);

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evEntityJumped"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evEntityJumped"));

    const UnionEntityInfo& creatureInfo1 = player1_->getUnionEntityInfo();
    ASSERT_EQ(false, creatureInfo1.asEntityMovementInfo().isMoving_);
    ASSERT_FLOAT_EQ(position.x_, creatureInfo1.asCharacterInfo().position_.x_);
}


TEST_F(FieldTest, testSay)
{
    const ChatMessage msg(L"abc");
    playerController1_->say(msg);

    ASSERT_EQ(1, playerController1_->getCallCount("evCreatureSaid"));
    ASSERT_EQ(1, playerController2_->getCallCount("evCreatureSaid"));
}


TEST_F(FieldTest, testSelectTarget)
{
    playerController1_->selectTarget(GameObjectInfo(otPc, invalidObjectId));
    ASSERT_EQ(1, playerController1_->getCallCount("onSelectTarget"));
    ASSERT_EQ(ecEntityNotFound, playerController1_->lastErrorCode_);

    playerController1_->selectTarget(GameObjectInfo(otPc, characterId2_));
    ASSERT_EQ(2, playerController1_->getCallCount("onSelectTarget"));
    ASSERT_EQ(ecOk, playerController1_->lastErrorCode_);
    ASSERT_EQ(characterId2_,
        playerController1_->lastTargetInfo_.targetInfo_.objectId_);
}
