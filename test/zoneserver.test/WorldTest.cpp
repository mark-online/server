#include "ZoneServerTestPCH.h"
#include "ZoneServiceTestFixture.h"
#include "MockZoneLoginServerProxy.h"
#include "MockZoneUser.h"
#include "MockPlayerController.h"
#include "MockZoneCommunityServerProxy.h"
#include "ZoneServer/user/ZoneUserManager.h"
#include "ZoneServer/user/ZoneUser.h"
#include <gideon/servertest/MockProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>

/**
* @class WorldTest
*
* World 테스트
*/
class WorldTest : public ZoneServiceTestFixture
{ 
private:
    virtual void SetUp() {
        ZoneServiceTestFixture::SetUp();

        characterId_ = createCharacter(accountId1_);
        loginUser(accountId1_);
        ASSERT_EQ(1, ZONEUSER_MANAGER->getUserCount());

        user1_ = getMockZoneUser(accountId1_);
        ASSERT_FALSE(user1_->isCharacterSelected());

    }

protected:
    ObjectId characterId_;
    MockZoneUser* user1_;
};


TEST_F(WorldTest, testNotifyToCommunityServer)
{
    // FYI: 랜덤 던전 포함
    user1_->selectCharacter(characterId_);
    user1_->enterWorld();
    MockPlayerController* playerController1 = &getMockPlayerController(*user1_);
    playerController1->readyToPlay();
    GameObjectInfo dungeonInfo_ = playerController1->getAnyEntity(otDungeon);
    ASSERT_TRUE(dungeonInfo_.isValid());
    playerController1->enterDungeon(dungeonInfo_.objectId_);
    ASSERT_EQ(1 + 1, getCommunityServerProxy().getCallCount("z2m_worldMapOpened"));
}


TEST_F(WorldTest, testEnterWorld)
{
    user1_->enterWorld();
    ASSERT_FALSE(user1_->isCharacterSelected());

    user1_->selectCharacter(characterId_);
    user1_->enterWorld();
    ASSERT_TRUE(user1_->isCharacterSelected());
    ASSERT_EQ(2, user1_->getCallCount("onEnterWorld"));

    MockPlayerController* playerController1 = &getMockPlayerController(*user1_);
    playerController1->readyToPlay();
    ASSERT_EQ(1, playerController1->getCallCount("onReadyToPlay"));

    /// already npc spawned
    ASSERT_EQ(1, getMockPlayerController(*user1_).getCallCount("evEntitiesAppeared"));
}


TEST_F(WorldTest, testLeaveWorld)
{
    ASSERT_FALSE(user1_->leaveFromWorld());

    user1_->selectCharacter(characterId_);
    user1_->enterWorld();
    ASSERT_EQ(1, user1_->getCallCount("onEnterWorld"));

    getMockPlayerController(*user1_).readyToPlay();

    ASSERT_TRUE(user1_->leaveFromWorld());
}


TEST_F(WorldTest, testEnterAndLeaveWorldWithOtherCharacter)
{
    user1_->selectCharacter(characterId_);
    user1_->enterWorld();
    getMockPlayerController(*user1_).readyToPlay();

    const AccountId accountId2 = accountId1_ + 1;
    setValidAccount(accountId2);
    const ObjectId characterId2 = createCharacter(accountId2);
    loginUser(accountId2);
    MockZoneUser* user2 = getMockZoneUser(accountId2);
    user2->selectCharacter(characterId2);
    user2->enterWorld();
    getMockPlayerController(*user2).readyToPlay();

    ASSERT_EQ(1, getMockPlayerController(*user1_).getCallCount("evEntitiesAppeared"));
    ASSERT_EQ(1, getMockPlayerController(*user1_).getCallCount("evEntityAppeared"));
    ASSERT_EQ(1, getMockPlayerController(*user2).getCallCount("evEntitiesAppeared"));
    ASSERT_EQ(0, getMockPlayerController(*user2).getCallCount("evEntityAppeared"));

    ASSERT_TRUE(user1_->leaveFromWorld());
    ASSERT_EQ(1, getMockPlayerController(*user2).getCallCount("evEntityDisappeared"));
}


TEST_F(WorldTest, testSaveCharacterStatsAfterLeaveWorld)
{
    user1_->selectCharacter(characterId_);
    user1_->enterWorld();
    getMockPlayerController(*user1_).readyToPlay();
    ASSERT_TRUE(user1_->leaveFromWorld());

    sne::database::Guard<servertest::MockProxyGameDatabase> db(SNE_DATABASE_MANAGER);
    ASSERT_EQ(1, db->getCallCount("saveCharacterStats"));
}


TEST_F(WorldTest, testNotifyToCommunityServerWhenWorldEntered)
{
    user1_->selectCharacter(characterId_);
    user1_->enterWorld();
    getMockPlayerController(*user1_).readyToPlay();
    ASSERT_TRUE(user1_->isCharacterSelected());
    ASSERT_EQ(1, user1_->getCallCount("onEnterWorld"));

    ASSERT_EQ(1, getCommunityServerProxy().getCallCount("z2m_worldMapEntered"));
}


TEST_F(WorldTest, testNotifyToCommunityServerWhenWorldLeft)
{
    user1_->selectCharacter(characterId_);
    user1_->enterWorld();
    getMockPlayerController(*user1_).readyToPlay();
    ASSERT_TRUE(user1_->leaveFromWorld());

    ASSERT_EQ(1, getCommunityServerProxy().getCallCount("z2m_worldMapLeft"));
}
