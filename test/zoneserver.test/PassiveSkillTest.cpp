#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerController.h"
#include "MockPlayerItemController.h"
#include "MockPlayerSkillController.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include "ZoneServer/model/gameobject/status/CreatureStatus.h"
#include <gideon/servertest/datatable/DataCodes.h>

using namespace gideon::zoneserver;

/**
* @class PassiveSkillTest
*
* 스킬 테스트
*/
class PassiveSkillTest : public GameTestFixture
{
public:
   //ObjectId addItem(SkillPackageCode packageCode) {
   //     const ObjectId stackCount = 4;
   //     const SlotId slotId = 15;
   //     BaseItemInfo skillPackageItem(packageCode, stackCount);

   //     playerInventoryController1_->addInventoryItem( skillPackageItem, slotId);
   //     return playerInventoryController1_->lastAddItemId_;
   // }

    void learnSkill(ObjectId itemId, SkillIndex index) {
        // 스킬 포인터를 얻는다.       
        itemId, index;
        //playerItemController1_->useSkillPackageItem(itemId, index);
        ASSERT_EQ(ecOk, playerItemController1_->lastErrorCode_);
    }
};


TEST_F(PassiveSkillTest, testSpellCraft)
{
    //playerController1_->rewardExp(toExpPoint(1000));
    //ObjectId itemId = addItem(servertest::skillPackage_2);
    //const FullCreatureStatusInfo prevStats = player1_->getCreatureStatus().getFullCreatureStatusInfo();
    //learnSkill(itemId, 18);
    //const FullCreatureStatusInfo oneStepStats = player1_->getCreatureStatus().getFullCreatureStatusInfo();
    //ASSERT_TRUE(prevStats.currentGameStatus_.spellPower_ < oneStepStats.currentGameStatus_.spellPower_);
    //ASSERT_TRUE(prevStats.currentGameStatus_.maxPoints_.mp_ < oneStepStats.currentGameStatus_.maxPoints_.mp_);
    //learnSkill(itemId, 18);
    //const FullCreatureStatusInfo twoStepStats = player1_->getCreatureStatus().getFullCreatureStatusInfo();
    //ASSERT_EQ(twoStepStats.currentGameStatus_.spellPower_, toManaPoint(oneStepStats.currentGameStatus_.spellPower_ + 10));
    //ASSERT_EQ(twoStepStats.currentGameStatus_.maxPoints_.mp_, toManaPoint(oneStepStats.currentGameStatus_.maxPoints_.mp_ + 10));

}
