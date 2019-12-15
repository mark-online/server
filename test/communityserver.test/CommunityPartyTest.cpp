#include "CommunityServerTestPCH.h"
#include "CommunityServiceTestFixture.h"
#include "MockCommunityLoginServerProxy.h"
#include "MockCommunityServerSideProxy.h"
#include "MockCommunityUser.h"
#include "CommunityServer/channel/WorldMapChannelManager.h"
#include "CommunityServer/user/CommunityUserManager.h"
#include "CommunityServer/party/CommunityPartyManager.h"

using namespace communityserver;

const MapCode worldMapCode = makeMapCode(mtGlobalWorld, 1);
const ObjectId mapId = 1;
const ObjectId playerId1 = 1;
const ObjectId playerId2 = 2;
const ObjectId playerId3 = 3;

/**
* @class CommunityPartyTest
*
* CommunityParty 관련 테스트
*/
class CommunityPartyTest : public CommunityServiceTestFixture
{ 
private:
    virtual void SetUp() {
        CommunityServiceTestFixture::SetUp();

        loginUser(accountId1_);
        ASSERT_EQ(1, COMMUNITYUSER_MANAGER->getUserCount());

        loginUser(accountId2_);
        ASSERT_EQ(2, COMMUNITYUSER_MANAGER->getUserCount());

        loginUser(accountId3_);
        ASSERT_EQ(3, COMMUNITYUSER_MANAGER->getUserCount());

        zoneServer1_->z2m_worldMapOpened(worldMapCode, mapId);
        ASSERT_EQ(1, WORLDMAP_CHANNEL_MANAGER->getChannelCount());

        zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId1_, playerId1, L"test1", 10000));
        zoneServer1_->z2m_worldMapEntered(mapId,
            accountId1_, playerId1, Position());

        zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId2_, playerId2, L"test2", 10000));
        zoneServer1_->z2m_worldMapEntered(mapId,
            accountId2_, playerId2, Position());

        zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId3_, playerId3, L"test3", 10000));
        zoneServer1_->z2m_worldMapEntered(mapId,
            accountId3_, playerId3, Position());

        user1_ = static_cast<MockCommunityUser*>(COMMUNITYUSER_MANAGER->getUser(accountId1_));
        user2_ = static_cast<MockCommunityUser*>(COMMUNITYUSER_MANAGER->getUser(accountId2_));
        user3_ = static_cast<MockCommunityUser*>(COMMUNITYUSER_MANAGER->getUser(accountId3_));
    }
protected:
    MockCommunityUser* user1_;
    MockCommunityUser* user2_;
    MockCommunityUser* user3_;
};

TEST_F(CommunityPartyTest, testInvite)
{
    user1_->inviteParty(L"test1");
    ASSERT_EQ(1, user1_->getCallCount("onInviteParty"));
    ASSERT_EQ(ecPartyNotInviteSelf, user1_->lastErrorCode_);

    user1_->inviteParty(L"test2");
    ASSERT_EQ(2, user1_->getCallCount("onInviteParty"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_EQ(1, user2_->getCallCount("evPartyInvited"));

    ASSERT_EQ(0, zoneServer1_->getCallCount("z2m_evPartyMemberJoined"));
}


TEST_F(CommunityPartyTest, testInviteRespond)
{
    user1_->inviteParty(L"test2");
    ASSERT_EQ(1, user1_->getCallCount("onInviteParty"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_EQ(1, user2_->getCallCount("evPartyInvited"));

    user2_->respondPartyInvitation(L"test1", false);
    ASSERT_EQ(1, user2_->getCallCount("onRespondPartyInvitation"));
    ASSERT_EQ(1, user1_->getCallCount("evPartyInvitationReponded"));
    ASSERT_EQ(false, user1_->lastPartyAnswer_);

    user2_->respondPartyInvitation(L"test1", true);
    ASSERT_EQ(1, user2_->getCallCount("evPartyMemberInfos"));
    ASSERT_EQ(1, user1_->getCallCount("evPartyMeberAdded"));

    ASSERT_EQ(2, user1_->lastMemberInfo_.playerId_);
    ASSERT_EQ(1, user1_->lastMemberInfos_.size());
    ASSERT_EQ(2, user2_->lastMemberInfos_.size());

    ASSERT_EQ(2, user2_->getCallCount("onRespondPartyInvitation"));
    ASSERT_EQ(2, user1_->getCallCount("evPartyInvitationReponded"));

    ASSERT_EQ(true, user1_->lastPartyAnswer_);

    ASSERT_EQ(2, zoneServer1_->getCallCount("z2m_evPartyMemberJoined"));
    
    ASSERT_TRUE(nullptr != COMMUNITYPARTY_MANAGER->getParty(1));
}


TEST_F(CommunityPartyTest, testReInvite)
{
    user1_->inviteParty(L"test2");
    ASSERT_EQ(1, user1_->getCallCount("onInviteParty"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_EQ(1, user2_->getCallCount("evPartyInvited"));

    user2_->respondPartyInvitation(L"test1", true);
    ASSERT_EQ(1, user2_->getCallCount("evPartyMemberInfos"));
    ASSERT_EQ(1, user1_->getCallCount("evPartyMeberAdded"));

    user1_->inviteParty(L"test2");
    ASSERT_EQ(ecPartyAlreadyMember, user1_->lastErrorCode_);

    user2_->respondPartyInvitation(L"test1", true);    
    ASSERT_EQ(ecPartySelfJoined, user2_->lastErrorCode_);

    user3_->respondPartyInvitation(L"test1", true);    
    ASSERT_EQ(ecOk, user3_->lastErrorCode_);
}


TEST_F(CommunityPartyTest, testKick)
{
    user2_->respondPartyInvitation(L"test1", true);
    user3_->respondPartyInvitation(L"test1", true);

    user1_->kickPartyMember(2);
    ASSERT_EQ(0, user1_->getCallCount("onKickPartyMember"));
    ASSERT_EQ(1, user1_->getCallCount("evPartyMemberKicked"));
    ASSERT_EQ(1, user2_->getCallCount("evPartyMemberKicked"));
    ASSERT_EQ(1, user3_->getCallCount("evPartyMemberKicked"));
    ASSERT_EQ(1, zoneServer1_->getCallCount("z2m_evPartyMemberLeft"));

    user1_->kickPartyMember(3);
    ASSERT_EQ(0, user1_->getCallCount("onKickPartyMember"));
    ASSERT_EQ(2, user1_->getCallCount("evPartyMemberKicked"));
    ASSERT_EQ(1, user2_->getCallCount("evPartyMemberKicked"));
    ASSERT_EQ(2, user3_->getCallCount("evPartyMemberKicked"));
    ASSERT_EQ(2 + 1, zoneServer1_->getCallCount("z2m_evPartyMemberLeft"));

    ASSERT_TRUE(nullptr == COMMUNITYPARTY_MANAGER->getParty(1));
}

TEST_F(CommunityPartyTest, testLeave)
{
    user2_->respondPartyInvitation(L"test1", true);
    user3_->respondPartyInvitation(L"test1", true);

    user1_->leaveParty();
    ASSERT_EQ(1, user1_->getCallCount("onLeaveParty"));
    ASSERT_EQ(0, user1_->getCallCount("evPartyMemberLeft"));
    ASSERT_EQ(1, user2_->getCallCount("evPartyMemberLeft"));
    ASSERT_EQ(1, user3_->getCallCount("evPartyMemberLeft"));
    ASSERT_EQ(0, user1_->getCallCount("evPartyMasterChanged"));
    ASSERT_EQ(1, user2_->getCallCount("evPartyMasterChanged"));
    ASSERT_EQ(1, user3_->getCallCount("evPartyMasterChanged"));
    ASSERT_EQ(1, zoneServer1_->getCallCount("z2m_evPartyMemberLeft"));

    user3_->leaveParty();
    ASSERT_EQ(1, user3_->getCallCount("onLeaveParty"));
    ASSERT_EQ(0, user1_->getCallCount("onKickPartyMember"));
    ASSERT_EQ(1, user2_->getCallCount("evPartyMasterChanged"));
    ASSERT_EQ(1, user3_->getCallCount("evPartyMasterChanged"));

    ASSERT_EQ(0, user1_->getCallCount("evPartyMemberLeft"));
    ASSERT_EQ(2, user2_->getCallCount("evPartyMemberLeft"));
    ASSERT_EQ(1, user3_->getCallCount("evPartyMemberLeft"));
    // 자동 탈퇴
    ASSERT_EQ(2 + 1, zoneServer1_->getCallCount("z2m_evPartyMemberLeft"));

    ASSERT_TRUE(nullptr == COMMUNITYPARTY_MANAGER->getParty(1));

    user2_->respondPartyInvitation(L"test1", true);
    ASSERT_EQ(ecOk, user2_->lastErrorCode_);
}
