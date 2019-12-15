#include "CommunityServerTestPCH.h"
#include "CommunityServiceTestFixture.h"
#include "MockCommunityServerSideProxy.h"
#include "CommunityServer/channel/WorldMapChannelManager.h"

using namespace communityserver;

const MapCode worldMapCode = makeMapCode(mtGlobalWorld, 1);
const ObjectId mapId = 1;

/**
* @class WorldMapChannelTest
*
* WorldMapChannel 관련 테스트
*/
class WorldMapChannelTest : public CommunityServiceTestFixture
{ 
private:
    virtual void SetUp() {
        CommunityServiceTestFixture::SetUp();

        ASSERT_EQ(0, WORLDMAP_CHANNEL_MANAGER->getChannelCount());

        zoneServer1_->z2m_worldMapOpened(worldMapCode, mapId);
    }
};


TEST_F(WorldMapChannelTest, testWorldMapOpened)
{
    ASSERT_EQ(1, WORLDMAP_CHANNEL_MANAGER->getChannelCount());

    WorldMapChannel::Ref channel =
        WORLDMAP_CHANNEL_MANAGER->getChannel(zoneServer1_->getZoneId(), mapId);
    ASSERT_TRUE(channel.get() != nullptr);
}


TEST_F(WorldMapChannelTest, testZoneServerDisconnected)
{
    zoneServer1_->unbridge();

    ASSERT_EQ(0, WORLDMAP_CHANNEL_MANAGER->getChannelCount());

    WorldMapChannel::Ref channel =
        WORLDMAP_CHANNEL_MANAGER->getChannel(zoneServer1_->getZoneId(), mapId);
    ASSERT_TRUE(channel.get() == nullptr);
}
