#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerController.h"
#include "ZoneServer/service/time/GameTimer.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/model/gameobject/status/CreatureStatus.h"
#include <gideon/cs/datatable/PropertyTable.h>

using namespace gideon::zoneserver;

/**
* @class ReviveTest
*
* 캐릭터 소생 테스트
*/
class ReviveTest : public GameTestFixture
{ 
};


TEST_F(ReviveTest, testRevive)
{
    ASSERT_EQ(1, playerController2_->getCallCount("evEntitiesAppeared"));

    playerController2_->revive();
    ASSERT_EQ(1, playerController2_->getCallCount("onRevive"));
    ASSERT_EQ(ecCreatureCannotRevive, playerController2_->lastErrorCode_);

    killPlayer2();

    playerController2_->revive();
    ASSERT_EQ(2, playerController2_->getCallCount("onRevive"));
    ASSERT_EQ(ecCharacterNotEnoughReviveTime, playerController2_->lastErrorCode_);

    mockGameTimerSource_->set(GAME_TIMER->msec() +
        GIDEON_PROPERTY_TABLE->getPropertyValue<GameTime>(L"player_min_revive_time"));
    playerController2_->revive();
    ASSERT_EQ(3, playerController2_->getCallCount("onRevive"));
    ASSERT_EQ(ecOk, playerController2_->lastErrorCode_);

    ASSERT_EQ(0, playerController1_->getCallCount("evEntityDisappeared"));

    ASSERT_EQ(1, playerController2_->getCallCount("evEntitiesAppeared"));
    ASSERT_EQ(1, playerController1_->getCallCount("evEntityAppeared"));


    go::Creature& creature2 = playerController2_->getOwnerAs<go::Creature>();
    const CreatureInfo& creatureInfo = creature2.getUnionEntityInfo().asCreatureInfo();
    const Points maxPoints = creature2.getCreatureStatus().getMaxPoints();
    ASSERT_EQ(creatureInfo.currentPoints_.hp_, maxPoints.hp_);
    ASSERT_EQ(creatureInfo.currentPoints_.mp_, maxPoints.mp_);
}
