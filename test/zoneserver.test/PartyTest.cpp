#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockNpcController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerPartyController.h"
#include "MockPlayerController.h"
#include "MockPlayerMoveController.h"
#include "MockZoneCommunityServerProxy.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/world/World.h"
#include "ZoneServer/world/WorldMap.h"
#include "ZoneServer/model/gameobject/Npc.h"
#include "ZoneServer/model/gameobject/status/CreatureStatus.h"
#include "ZoneServer/model/state/CreatureState.h"
#include "ZoneServer/service/skill/Skill.h"
#include "ZoneServer/service/party/PartyService.h"
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/servertest/datatable/MockSkillTable.h>


const GameTime defaultLootingDelay = 5000;

/**
* @class PartyTest
*
* 파티 관련 테스트
*/
class PartyTest : public GameTestFixture
{
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();

        monsterInfo1_.objectType_ = otMonster;
        monsterInfo1_.objectId_ = 1;
        monster1_ =
            static_cast<go::Npc*>(WORLD->getGlobalWorldMap().getEntity(monsterInfo1_));
        ASSERT_TRUE(monster1_ != nullptr);
        monsterController1_ = &monster1_->getControllerAs<MockNpcController>();

        monsterInfo2_.objectType_ = otMonster;
        monsterInfo2_.objectId_ = 2;
        monster2_ =
            static_cast<go::Npc*>(WORLD->getGlobalWorldMap().getEntity(monsterInfo2_));
        ASSERT_TRUE(monster2_ != nullptr);
        monsterController2_ = &monster2_->getControllerAs<MockNpcController>();

        //skillCode_ = servertest::defaultMeleeAttackSkillCode;
        //skillTemplate_ = SKILL_TABLE->getSkill(skillCode_);
        //ASSERT_TRUE(skillTemplate_ != nullptr);


        // 비석이 호출될때 evEntityAppeared 호출이 되면 안된다. 
        //TODO: 비석생성시 evEntityAppeared 호출 스킵~
        //ASSERT_EQ(2, playerController1_->getCallCount("evEntityAppeared"));
        //ASSERT_EQ(2, playerController1_->getCallCount("evEntityAppeared"));
    }

protected:
    void killMonster1();

    MockNpcController& getMockNpcController(go::Npc& monster) {
        return monster.getControllerAs<MockNpcController>();
    }

protected:
    GameObjectInfo monsterInfo1_;
    go::Npc* monster1_;
    MockNpcController* monsterController1_;

    GameObjectInfo monsterInfo2_;
    go::Npc* monster2_;
    MockNpcController* monsterController2_;

    SkillCode skillCode_;
    //const datatable::SkillTemplate* skillTemplate_;
    ObjectId lastGraveStoneId_;
};


void PartyTest::killMonster1()
{
    ASSERT_EQ(monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
        hpDefault);

    //const int prevDiedCount = playerController1_->getCallCount("playerController1_");
    //monster1_->queryCreatureState()->invincible(false);

    //for (;;) {
    //    playerCastController1_->startCasting(createUnionSkillCastToInfo(monsterInfo1_, skillCode_));

    //    const datatable::SkillTemplate* skillTemplate = SKILL_TABLE->getSkill(skillCode_);
    //    ASSERT_TRUE(skillTemplate != nullptr);
    //    mockGameTimerSource_->set(GAME_TIMER->msec() +
    //        skillTemplate->getSkillInfo()->cool_time() + 1);

    //    if (playerController1_->getCallCount("evNpcDied") > prevDiedCount) {
    //        break;
    //    }
    //    // FYI: MockTaskScheduler에 의해 부활되어 버리므로 아래 코드를 사용하면 안됨!
    //    //if (monster1_->getUnionEntityInfo().asCreatureInfo().currentPoints_.isDied()) {
    //    //    break;
    //    //}
    //}
}

TEST_F(PartyTest, testPartyAddMember)
{
    getCommunityServerProxy().z2m_evPartyMemberJoined(1, player1_->getObjectId());
    ASSERT_TRUE(nullptr != player1_->queryPartyable()->getParty());
    getCommunityServerProxy().z2m_evPartyMemberJoined(1, player2_->getObjectId());
    ASSERT_TRUE(nullptr != player2_->queryPartyable()->getParty());
}


TEST_F(PartyTest, testPartyLeave)
{
    getCommunityServerProxy().z2m_evPartyMemberJoined(1, player1_->getObjectId());
    getCommunityServerProxy().z2m_evPartyMemberLeft(1, player1_->getObjectId());
    ASSERT_TRUE(nullptr == player1_->queryPartyable()->getParty());
    ASSERT_TRUE(nullptr == PARTY_SERVICE->getPartyByPartyId(1));
}

TEST_F(PartyTest, testPartyRewardExp)
{
    getCommunityServerProxy().z2m_evPartyMemberJoined(1, player1_->getObjectId());
    getCommunityServerProxy().z2m_evPartyMemberJoined(1, player2_->getObjectId());
    killMonster1();
    ASSERT_EQ(1, playerController1_->getCallCount("evExpPointUpdated"));
    ASSERT_EQ(1, playerController2_->getCallCount("evExpPointUpdated"));

}

TEST_F(PartyTest, testPartyMemberMove)
{
    ASSERT_EQ(0, playerController1_->getCallCount("evEntityStopped"));
    ASSERT_EQ(0, playerController2_->getCallCount("evEntityStopped"));

    getCommunityServerProxy().z2m_evPartyMemberJoined(1, player1_->getObjectId());
    getCommunityServerProxy().z2m_evPartyMemberJoined(1, player2_->getObjectId());
    ObjectPosition position = player2_->getPosition();
    //position.y_ = skillTemplate_->getSkillCommonInfo().minDistance_ + 1;
    playerMoveController1_->stop(position);

    ASSERT_EQ(1, playerMoveController1_->getCallCount("evEntityStopped"));
    ASSERT_EQ(1, playerMoveController2_->getCallCount("evEntityStopped"));
    ASSERT_EQ(0, playerPartyController1_->getCallCount("evPartyMemberMoved"));
    ASSERT_EQ(1, playerPartyController2_->getCallCount("evPartyMemberMoved"));
}
