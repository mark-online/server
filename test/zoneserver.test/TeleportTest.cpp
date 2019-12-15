#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockZoneLoginServerProxy.h"
#include "MockZoneCommunityServerProxy.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerTeleportController.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/world/region/MapRegion.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/ZoneService.h"
#include <gideon/servertest/datatable/MockRegionTable.h>
#include <gideon/servertest/datatable/MockWorldMapTable.h>
/**
* @class TeleportTest
*
* 단일 존 서버 내에서의 텔레포트 테스트
*/
class TeleportTest : public GameTestFixture
{ 
};


TEST_F(TeleportTest, testTeleportToInvalidRegion)
{
    ASSERT_TRUE(ZONE_SERVICE != 0);

    playerTeleportController1_->teleportToRegion(WORLD->getWorldMapCode(), invalidRegionCode);

    ASSERT_EQ(1, playerTeleportController1_->getCallCount("onTeleportTo"));
    ASSERT_EQ(ecTeleportNotAvailable, playerTeleportController1_->lastErrorCode_);
}


TEST_F(TeleportTest, testTeleportToCurrentWorldMap)
{
    ASSERT_TRUE(ZONE_SERVICE != 0);

    const ObjectPosition position = player1_->getPosition();

    playerTeleportController1_->teleportToRegion(player1_->getCurrentWorldMap()->getMapCode(),
        servertest::portalEntryRegionCode);

    ASSERT_EQ(1, playerTeleportController1_->getCallCount("onTeleportTo"));
    ASSERT_EQ(ecOk, playerTeleportController1_->lastErrorCode_);

    ASSERT_NE(position.x_, player1_->getPosition().x_);
}


TEST_F(TeleportTest, testTeleportToOtherWorldMapInOtherPosition)
{
    ASSERT_TRUE(ZONE_SERVICE != 0);

    playerTeleportController1_->teleportToRegion(servertest::otherWorldMapCode,
        servertest::portalExitRegionCode);

    ASSERT_EQ(1, playerTeleportController1_->getCallCount("onTeleportTo"));
    ASSERT_EQ(ecTeleportNotAvailable, playerTeleportController1_->lastErrorCode_);
}


TEST_F(TeleportTest, testTeleportToOtherWorldMap)
{
    ASSERT_TRUE(ZONE_SERVICE != 0);

    WorldMap& globalWorldMap = WORLD->getGlobalWorldMap();

    const MapRegion* teleportMapRegion =
        globalWorldMap.getTeleportMapRegion(servertest::otherWorldMapCode,
        servertest::portalExitRegionCode);
    ASSERT_TRUE(teleportMapRegion != nullptr);
    const ObjectPosition targetPosition = ObjectPosition(teleportMapRegion->getCenterPosition(), 0);

    playerMoveController1_->move(targetPosition);

    playerTeleportController1_->teleportToRegion(servertest::otherWorldMapCode,
        servertest::portalExitRegionCode);

    ASSERT_EQ(1, playerTeleportController1_->getCallCount("onTeleportTo"));
    ASSERT_EQ(ecOk, playerTeleportController1_->lastErrorCode_);

    ASSERT_EQ(1, getLoginServerProxy().getCallCount("z2l_logoutUser"));
    ASSERT_EQ(1, getLoginServerProxy().getCallCount("z2l_reserveMigration"));
    ASSERT_EQ(1, ZONEUSER_MANAGER->getUserCount());

    ASSERT_EQ(1, getCommunityServerProxy().getCallCount("z2m_worldMapLeft"));
}
