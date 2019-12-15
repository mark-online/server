#include "CommunityServerTestPCH.h"
#include "CommunityServiceTestFixture.h"
#include "MockCommunityLoginServerProxy.h"
#include "MockCommunityServerSideProxy.h"
#include "MockCommunityUser.h"
#include "CommunityServer/channel/WorldMapChannelManager.h"
#include "CommunityServer/user/CommunityUserManager.h"
#include "CommunityServer/Guild/CommunityGuildManager.h"

using namespace communityserver;

const MapCode worldMapCode = makeMapCode(mtGlobalWorld, 1);
const ObjectId mapId = 1;
const ObjectId playerId1 = 1;
const ObjectId playerId2 = 2;
const ObjectId playerId3 = 3;

/**
* @class CommunityGuildTest
*
* CommunityGuild 관련 테스트
*/
class CommunityGuildTest : public CommunityServiceTestFixture
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

        zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId1_, playerId1, L"test1"));
        zoneServer1_->z2m_worldMapEntered(mapId,
            accountId1_, playerId1, Position());
        
        zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId2_, playerId2, L"test2"));
        zoneServer1_->z2m_worldMapEntered(mapId,
            accountId2_, playerId2, Position());

        zoneServer1_->z2m_initPlayerInfo(PlayerInfo(accountId3_, playerId3, L"test3"));
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

TEST_F(CommunityGuildTest, testDuplicatedGuildName)
{
    user1_->createGuild(L"test", 11111);
    ASSERT_EQ(1, user1_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);

    user2_->createGuild(L"test", 11111);
    ASSERT_EQ(1, user2_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecGuildInvalidName, user2_->lastErrorCode_);
}

TEST_F(CommunityGuildTest, testInvite)
{
    user1_->createGuild(L"test", 11111);
    ASSERT_EQ(1, user1_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);

    user1_->inviteGuild(L"test1");
    ASSERT_EQ(1, user1_->getCallCount("onInviteGuild"));
    ASSERT_EQ(ecGuildNotInviteSelf, user1_->lastErrorCode_);

    user1_->inviteGuild(L"test2");
    ASSERT_EQ(2, user1_->getCallCount("onInviteGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_EQ(1, user2_->getCallCount("evGuildInvited"));
}


TEST_F(CommunityGuildTest, testInviteRespond)
{
    user1_->createGuild(L"test", 11111);
    user1_->inviteGuild(L"test2");
    ASSERT_EQ(1, user1_->getCallCount("onInviteGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_EQ(1, user2_->getCallCount("evGuildInvited"));


    user2_->respondGuildInvitation(L"test1", false);
    ASSERT_EQ(1, user2_->getCallCount("onRespondGuildInvitation"));
    ASSERT_EQ(1, user1_->getCallCount("evGuildInvitationResponded"));
    ASSERT_EQ(false, user1_->lastGuildAnswer_);

    user2_->respondGuildInvitation(L"test1", true);
    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));
}


TEST_F(CommunityGuildTest, testReInvite)
{
    user1_->createGuild(L"test", 11111);
    user1_->inviteGuild(L"test2");
    ASSERT_EQ(1, user1_->getCallCount("onInviteGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_EQ(1, user2_->getCallCount("evGuildInvited"));

    user2_->respondGuildInvitation(L"test1", true);

    user1_->inviteGuild(L"test2");
    ASSERT_EQ(ecGuildAlreadyMember, user1_->lastErrorCode_);

    user2_->respondGuildInvitation(L"test1", true);    
    ASSERT_EQ(ecGuildSelfJoined, user2_->lastErrorCode_);

    user3_->respondGuildInvitation(L"test1", true);    
    ASSERT_EQ(ecOk, user3_->lastErrorCode_);
}

TEST_F(CommunityGuildTest, testLeaveGuildMember)
{
    user1_->createGuild(L"test", 11111);
    ASSERT_EQ(1, user1_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));

    user2_->respondGuildInvitation(L"test1", true);
    ASSERT_EQ(ecOk, user2_->lastErrorCode_);
    user3_->respondGuildInvitation(L"test1", true);    
    ASSERT_EQ(ecOk, user3_->lastErrorCode_);

    user2_->leaveGuild();
    ASSERT_EQ(ecOk, user2_->lastErrorCode_);
    ASSERT_EQ(1, user2_->getCallCount("onLeaveGuild"));
    ASSERT_EQ(0, user2_->getCallCount("evGuildMemberLeft"));
    
    ASSERT_EQ(1, user1_->getCallCount("evGuildMemberLeft"));
    ASSERT_EQ(2, user1_->leftGuildMemberId_);

    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));
}

TEST_F(CommunityGuildTest, testLeaveGuildMaster)
{
    user1_->createGuild(L"test", 11111);
    ASSERT_EQ(1, user1_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));

    user2_->respondGuildInvitation(L"test1", true);
    ASSERT_EQ(ecOk, user2_->lastErrorCode_);
    user3_->respondGuildInvitation(L"test1", true);    
    ASSERT_EQ(ecOk, user3_->lastErrorCode_);

    user1_->leaveGuild();
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_EQ(1, user1_->getCallCount("onLeaveGuild"));
    ASSERT_EQ(0, user1_->getCallCount("evGuildMemberLeft"));
    ASSERT_EQ(1, user2_->getCallCount("evGuildMemberLeft"));
    ASSERT_EQ(1, user2_->leftGuildMemberId_);
    ASSERT_EQ(1, user3_->getCallCount("evGuildMemberLeft"));
    ASSERT_EQ(1, user3_->leftGuildMemberId_);
    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));

    // TODO: 길드장 변경 이벤트 테스트
}


TEST_F(CommunityGuildTest, testSearchGuild)
{
    user1_->createGuild(L"test", 11111);
    ASSERT_EQ(1, user1_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecOk, user1_->lastErrorCode_);
    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));

    user2_->createGuild(L"2ndguild", 11111);
    ASSERT_EQ(1, user2_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecOk, user2_->lastErrorCode_);
    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));

    user3_->createGuild(L"3rdguild", 11111);
    ASSERT_EQ(1, user3_->getCallCount("onCreateGuild"));
    ASSERT_EQ(ecOk, user3_->lastErrorCode_);
    ASSERT_TRUE(nullptr != COMMUNITYGUILD_MANAGER->getGuild(12));

    user2_->searchGuildInfo(L"tes1");
    ASSERT_EQ(0, user2_->lastSearchGuildInfos_.size());

    user2_->searchGuildInfo(L"te");
    ASSERT_EQ(1, user2_->lastSearchGuildInfos_.size());

    user1_->searchGuildInfo(L"guild");
    ASSERT_EQ(2, user1_->lastSearchGuildInfos_.size());

    user3_->searchGuildInfo(L"2nd");
    ASSERT_EQ(1, user3_->lastSearchGuildInfos_.size());
}


//
//TEST_F(CommunityGuildTest, testKick)
//{
//    user2_->respondGuildInvitation(L"test1", true);
//    user3_->respondGuildInvitation(L"test1", true);
//
//    user1_->kickGuildMember(2);
//    ASSERT_EQ(0, user1_->getCallCount("onKickGuildMember"));
//    ASSERT_EQ(1, user1_->getCallCount("evGuildMemberKicked"));
//    ASSERT_EQ(1, user2_->getCallCount("evGuildMemberKicked"));
//    ASSERT_EQ(1, user3_->getCallCount("evGuildMemberKicked"));
//    ASSERT_EQ(1, zoneServer1_->getCallCount("z2m_evGuildMemberLeft"));
//
//    user1_->kickGuildMember(3);
//    ASSERT_EQ(0, user1_->getCallCount("onKickGuildMember"));
//    ASSERT_EQ(2, user1_->getCallCount("evGuildMemberKicked"));
//    ASSERT_EQ(1, user2_->getCallCount("evGuildMemberKicked"));
//    ASSERT_EQ(2, user3_->getCallCount("evGuildMemberKicked"));
//    ASSERT_EQ(2 + 1, zoneServer1_->getCallCount("z2m_evGuildMemberLeft"));
//
//    ASSERT_TRUE(nullptr == COMMUNITYGUILD_MANAGER->getGuild(1));
//}
//
//TEST_F(CommunityGuildTest, testLeave)
//{
//    user2_->respondGuildInvitation(L"test1", true);
//    user3_->respondGuildInvitation(L"test1", true);
//
//    user1_->leaveGuild();
//    ASSERT_EQ(1, user1_->getCallCount("onLeaveGuild"));
//    ASSERT_EQ(0, user1_->getCallCount("evGuildMemberLeft"));
//    ASSERT_EQ(1, user2_->getCallCount("evGuildMemberLeft"));
//    ASSERT_EQ(1, user3_->getCallCount("evGuildMemberLeft"));
//    ASSERT_EQ(0, user1_->getCallCount("evGuildMasterChanged"));
//    ASSERT_EQ(1, user2_->getCallCount("evGuildMasterChanged"));
//    ASSERT_EQ(1, user3_->getCallCount("evGuildMasterChanged"));
//    ASSERT_EQ(1, zoneServer1_->getCallCount("z2m_evGuildMemberLeft"));
//
//    user3_->leaveGuild();
//    ASSERT_EQ(1, user3_->getCallCount("onLeaveGuild"));
//    ASSERT_EQ(0, user1_->getCallCount("onKickGuildMember"));
//    ASSERT_EQ(1, user2_->getCallCount("evGuildMasterChanged"));
//    ASSERT_EQ(1, user3_->getCallCount("evGuildMasterChanged"));
//
//    ASSERT_EQ(0, user1_->getCallCount("evGuildMemberLeft"));
//    ASSERT_EQ(2, user2_->getCallCount("evGuildMemberLeft"));
//    ASSERT_EQ(1, user3_->getCallCount("evGuildMemberLeft"));
//    // 자동 탈퇴
//    ASSERT_EQ(2 + 1, zoneServer1_->getCallCount("z2m_evGuildMemberLeft"));
//
//    ASSERT_TRUE(nullptr == COMMUNITYGUILD_MANAGER->getGuild(1));
//
//    user2_->respondGuildInvitation(L"test1", true);
//    ASSERT_EQ(ecOk, user2_->lastErrorCode_);
//}
