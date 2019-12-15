#include "CommunityServerTestPCH.h"
#include "CommunityServiceTestFixture.h"
#include "MockCommunityLoginServerProxy.h"
#include "MockCommunityServerSideProxy.h"
#include "MockCommunityUser.h"
#include "CommunityServer/channel/WorldMapChannelManager.h"
#include "CommunityServer/user/CommunityUserManager.h"

using namespace communityserver;

const MapCode worldMapCode = makeMapCode(mtGlobalWorld, 1);
const ObjectId mapId = 1;
const ObjectId playerId1 = 1;

/**
* @class CommunityUserTest
*
* CommunityUser 관련 테스트
*/
class CommunityUserTest : public CommunityServiceTestFixture
{ 
private:
    virtual void SetUp() {
        CommunityServiceTestFixture::SetUp();

        loginUser(accountId1_);
        ASSERT_EQ(1, COMMUNITYUSER_MANAGER->getUserCount());

        zoneServer1_->z2m_worldMapOpened(worldMapCode, mapId);
        ASSERT_EQ(1, WORLDMAP_CHANNEL_MANAGER->getChannelCount());
    }
};


TEST_F(CommunityUserTest, testWorldMapEntered)
{
    zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId1_, playerId1, L"test1", 10000));
    zoneServer1_->z2m_worldMapEntered(mapId,
        accountId1_, playerId1, Position());

    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId1_);
    ASSERT_TRUE(user != nullptr);
    ASSERT_TRUE(user->isOnline());

    WorldMapChannel::Ref channel =
        WORLDMAP_CHANNEL_MANAGER->getChannel(zoneServer1_->getZoneId(), mapId);
    ASSERT_TRUE(channel.get() != nullptr);
    ASSERT_TRUE(channel->isEntered(accountId1_));

    ASSERT_EQ(1, static_cast<MockCommunityUser*>(user)->getCallCount("evWorldMapChannelEntered"));
}


TEST_F(CommunityUserTest, testWorldMapLeft)
{
    zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId1_, playerId1, L"test1", 10000));
    zoneServer1_->z2m_worldMapEntered(mapId,
        accountId1_, playerId1, Position());
    zoneServer1_->z2m_worldMapLeft(accountId1_, playerId1);

    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId1_);
    ASSERT_TRUE(user != nullptr);
    ASSERT_TRUE(user->isOnline());

    WorldMapChannel::Ref channel =
        WORLDMAP_CHANNEL_MANAGER->getChannel(zoneServer1_->getZoneId(), mapId);
    ASSERT_TRUE(channel.get() != nullptr);
    ASSERT_TRUE(! channel->isEntered(accountId1_));

    ASSERT_EQ(1, static_cast<MockCommunityUser*>(user)->getCallCount("evWorldMapChannelLeft"));
}


TEST_F(CommunityUserTest, testUserLoggedout)
{
    COMMUNITYUSER_MANAGER->logout(accountId1_);
    ASSERT_EQ(1, getLoginServerProxy().getCallCount("m2l_logoutUser"));
    ASSERT_EQ(0, COMMUNITYUSER_MANAGER->getUserCount());
}


TEST_F(CommunityUserTest, testUserLoggedoutAfterWorldMapEntered)
{
    zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId1_, playerId1, L"test1", 10000));
    zoneServer1_->z2m_worldMapEntered(mapId,
        accountId1_, playerId1, Position());

    COMMUNITYUSER_MANAGER->logout(accountId1_);
    ASSERT_EQ(1, getLoginServerProxy().getCallCount("m2l_logoutUser"));
    ASSERT_EQ(0, COMMUNITYUSER_MANAGER->getUserCount());

    WorldMapChannel::Ref channel =
        WORLDMAP_CHANNEL_MANAGER->getChannel(zoneServer1_->getZoneId(), mapId);
    ASSERT_TRUE(channel.get() != nullptr);
    ASSERT_TRUE(! channel->isEntered(accountId1_));
}


TEST_F(CommunityUserTest, testWorldMapChatting)
{
    const AccountId accountId2 = accountId1_ + 1;
    const ObjectId playerId2 = playerId1 + 1;

    setValidAccount(accountId2);

    loginUser(accountId2);
    ASSERT_EQ(2, COMMUNITYUSER_MANAGER->getUserCount());

    zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId1_, playerId1, L"test1", 10000));
    zoneServer1_->z2m_worldMapEntered(mapId,
        accountId1_, playerId1, Position());
    zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId2_, playerId2, L"test2", 10000));
    zoneServer1_->z2m_worldMapEntered(mapId,
        accountId2, playerId2, Position());

    CommunityUser* user1 = COMMUNITYUSER_MANAGER->getUser(accountId1_);
    ASSERT_TRUE(user1 != nullptr);
    CommunityUser* user2 = COMMUNITYUSER_MANAGER->getUser(accountId2);
    ASSERT_TRUE(user2 != nullptr);

    const ChatMessage msg(L"abc");
    user1->sayInWorldMap(msg);

    ASSERT_EQ(1, static_cast<MockCommunityUser*>(user1)->getCallCount("evWorldMapSaid"));
    ASSERT_EQ(1, static_cast<MockCommunityUser*>(user2)->getCallCount("evWorldMapSaid"));
}


TEST_F(CommunityUserTest, testNotice)
{
    const AccountId accountId2 = accountId1_ + 1;
    const ObjectId playerId2 = playerId1 + 1;

    setValidAccount(accountId2);

    loginUser(accountId2);
    ASSERT_EQ(2, COMMUNITYUSER_MANAGER->getUserCount());

    zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId1_, playerId1, L"test1", 10000));
    zoneServer1_->z2m_worldMapEntered(mapId,
        accountId1_, playerId1, Position());
    zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId2_, playerId2, L"test2", 10000));
    zoneServer1_->z2m_worldMapEntered(mapId,
        accountId2, playerId2, Position());

    CommunityUser* user1 = COMMUNITYUSER_MANAGER->getUser(accountId1_);
    ASSERT_TRUE(user1 != nullptr);
    CommunityUser* user2 = COMMUNITYUSER_MANAGER->getUser(accountId2);
    ASSERT_TRUE(user2 != nullptr);

    user2->noticeToShard(L"notice");
    ASSERT_EQ(0, static_cast<MockCommunityUser*>(user1)->getCallCount("evShardNoticed"));
    ASSERT_EQ(0, static_cast<MockCommunityUser*>(user2)->getCallCount("evShardNoticed"));

    user1->noticeToShard(L"notice");
    ASSERT_EQ(1, static_cast<MockCommunityUser*>(user1)->getCallCount("evShardNoticed"));
    ASSERT_EQ(1, static_cast<MockCommunityUser*>(user2)->getCallCount("evShardNoticed"));
}
