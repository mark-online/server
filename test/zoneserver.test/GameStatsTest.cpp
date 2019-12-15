#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockPlayerSkillController.h"
#include "MockPlayerInventoryController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon\servertest\datatable\DataCodes.h>
#include <gideon/servertest/datatable/MockSkillTable.h>
#include <gideon/cs/datatable/EquipTable.h>

/**
* @class CreatureStatusInfoTest
*
* 스텟 관련 테스트
*/
class CreatureStatusInfoTest : public GameTestFixture
{ 
private:
    virtual void SetUp() {
        GameTestFixture::SetUp();
        //validSkillCode_ = servertest::defaultMeleeAttackSkillCode;

        //skillTemplate_ = SKILL_TABLE->getSkill(validSkillCode_);
        //ASSERT_TRUE(skillTemplate_ != nullptr);
        /*const gdt::equip_t* equipTemplate_ = EQUIP_TABLE->getEquip(servertest::otherHelmetEquipCode);
        ASSERT_TRUE(equipTemplate_ != nullptr);
        criticalBlockRate_ = static_cast<float32_t>(equipTemplate_->critical_block_apply());
        criticalDamage_ = static_cast<float32_t>(equipTemplate_->critical_damage_apply());        
        registRate_ = static_cast<float32_t>(equipTemplate_->critical_block_apply());
        skillDamage_ = static_cast<float32_t>(skillTemplate_->getSkillInfo()->castee_effect_value_1() +
            equipTemplate_->attack_apply());*/
    }

protected:
    SkillCode validSkillCode_;
    //const datatable::SkillTemplate* skillTemplate_;
    float32_t skillDamage_;
    float32_t criticalDamage_;
    float32_t criticalBlockRate_;
    float32_t registRate_;

};


TEST_F(CreatureStatusInfoTest, testCreatureStatusInfo)
{
    const InventoryInfo p1InvenBefore = player1_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* p1Helmet = getItemInfo(p1InvenBefore,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(p1Helmet != nullptr);

    playerInventoryController1_->equipItem(p1Helmet->itemId_);

    const InventoryInfo p2InvenBefore = player2_->queryInventoryable()->getInventoryInfo();
    const ItemInfo* p2Helmet = getItemInfo(p2InvenBefore,
        servertest::otherHelmetEquipCode);
    ASSERT_TRUE(p2Helmet != nullptr);

    //playerInventoryController2_->equipItem(p2Helmet->itemId_);

    //const HitPoint hp2 = player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_;

    //playerCastController1_->startCasting(GameObjectInfo(otPc, characterId2_),
    //    validSkillCode_);

    //criticalDamage_ = gideon::getEffectReducePoint(criticalDamage_, criticalBlockRate_);
    //float32_t damage = skillDamage_ + criticalDamage_;
    //damage = getEffectReducePoint(damage, registRate_);
    //uint32_t realDamage = static_cast<uint32_t>(damage);
    //ASSERT_EQ(player2_->getUnionEntityInfo().asCreatureInfo().currentPoints_.hp_,
    //    toHitPoint(hp2 - realDamage));
}