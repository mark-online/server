#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerSkillController.h"
#include "MockPlayerAnchorController.h"
#include "MockPlayerController.h"
#include "MockPlayerMoveController.h"
#include "MockZoneCommunityServerProxy.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/service/anchor/AnchorService.h"
#include "ZoneServer/service/skill/Skill.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/servertest/datatable/DataCodes.h>

const GameTime defaultLootingDelay = 5000;

/**
* @class BuildingTest
*
* 설치물 관련 테스트
*/
class BuildingTest : public GameTestFixture
{
    virtual void SetUp() {
        GameTestFixture::SetUp();
        const uint8_t stackCount1 = 1;
        const SlotId slotId1 = 17;
        BaseItemInfo anchorItem(servertest::castleWall, stackCount1);


        const uint8_t stackCount2 = 100;
        const SlotId slotId2 = 18;
        BaseItemInfo buildMaterialItem(servertest::normalNpcElementCode1, stackCount2);


        ASSERT_TRUE(playerInventoryController1_->addInventoryItem(anchorItem, slotId1));
        buildingId_ = playerInventoryController1_->lastAddItemId_;
        ASSERT_TRUE(playerInventoryController1_->addInventoryItem(buildMaterialItem, slotId2));
        buildMeterialId_ = playerInventoryController1_->lastAddItemId_;
    }
protected:
    ObjectId buildingId_;
    ObjectId buildMeterialId_;
};



TEST_F(BuildingTest, testCreateBuilding)
{
    ASSERT_EQ(1, getMockPlayerController(*user1_).getCallCount("evEntityAppeared"));
    ASSERT_EQ(0, getMockPlayerController(*user2_).getCallCount("evEntityAppeared"));

    const ObjectPosition position1 = playerController1_->getOwner().getPosition();

    playerAnchorController1_->createBuilding(buildingId_, position1);
    ASSERT_EQ(0, playerAnchorController1_->getCallCount("onCreateBuilding"));

    ASSERT_EQ(1, playerInventoryController1_->getCallCount("evInventoryItemRemoved"));
    ASSERT_EQ(1 + 1, getMockPlayerController(*user1_).getCallCount("evEntityAppeared"));
    ASSERT_EQ(1, getMockPlayerController(*user2_).getCallCount("evEntityAppeared"));
}



TEST_F(BuildingTest, testBuildBuilding)
{
    playerAnchorController1_->createBuilding(buildingId_, position1);
    ASSERT_EQ(0, playerAnchorController1_->getCallCount("onCreateBuilding"));

    playerAnchorController1_->build()
}