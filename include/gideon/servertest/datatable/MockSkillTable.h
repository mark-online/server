#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/SkillInfo.h>


namespace gideon { namespace servertest {
//
//const SkillCode defaultMeleeAttackSkillCode = makeSkillCode(1, 1); // 베어넘기기 1Lv
//const SkillCode defaultRangeAttackSkillCode = makeSkillCode(2, 1); // 관통 화살 1Lv
//const SkillCode defaultMagicAttackSkillCode = makeSkillCode(3, 1); // 마법탄 1Lv
//
//
//const SkillCode learnSkill_1 = makeSkillCode(4, 1); // 압도 스킬인덱스 4 level_1
//const SkillCode learnSkill_2 = makeSkillCode(4, 2); // 압도 스킬인덱스 4 level_2
//const SkillCode learnSkill_3 = makeSkillCode(4, 3); // 압도 스킬인덱스 4 level_3
//
//const SkillCode defaultTargetAreaSkillCode = makeSkillCode(5, 1); // 파멸 1Lv(Target Area 아님!!!)
//const SkillCode defaultSelfAreaSkillCode = makeSkillCode(6, 1); // 칼날 폭풍 1Lv
//
//
//const SkillCode testObserverSkillCode = makeSkillCode(7, 1); // 임의
//
//
//const SkillCode defaultLongGlobalCooldownTimeSkillCode = makeSkillCode(8, 1); // 얼음 방벽 1Lv
//
//const SkillCode defaultLongCastTimeSkillCode = makeSkillCode(9, 1); // 영원한 화염 1Lv
//
//const SkillCode hpRecoverySkillCode = makeSkillCode(10, 1); //자연의 결속 1lv
//const SkillCode mpRecoverySkillCode = makeSkillCode(11, 1); // 마나재생 1lv
//
//const SkillCode graveStoneSkillCode = makeSkillCode(12, 1);	// 집행 1Lv
//
//
//const SkillCode stunSkillCode = makeSkillCode(13, 1); // 스턴
//
//// charater stat chage 데이타는(3개) 다 틀림.
//const SkillCode peaceUseTypeSkillCode = makeSkillCode(14, 1); // 강타 1Lv
//const SkillCode combatUseTypeSkillCode = makeSkillCode(15, 2); // 회오리 바람 2Lv 
//const SkillCode changePeaceSkillCode = makeSkillCode(16, 1);
//const SkillCode absolutenessDefence = makeSkillCode(17, 1); // 절대 방어
//
//// 패시브 스킬
//const SkillCode spellCraft_1 = makeSkillCode(18, 1); // 주문학 level 1
//const SkillCode spellCraft_2 = makeSkillCode(18, 2); // 주문학 level 2
//
//const SkillCode defaultAreaAndCastee2SkillCode = makeSkillCode(19, 1); // 폭풍 1Lv
//


/**
* @class MockSkillTable
*/
//class GIDEON_SERVER_API MockSkillTable : public gideon::datatable::SkillTable
//{
//public:
   /* MockSkillTable() {
        fillSkills();
    }
    virtual ~MockSkillTable() {
        destroySkills();
    }

    virtual const gideon::datatable::PlayerSkillTemplate* getSkill(SkillCode code) const;

private:
    virtual const SkillMap& getSkillMap() const {
        return skillMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillSkills() {
        fillObserverSkillCode();
        fillStunSkill();
        fillGraveStoneSkill();
        fillMpRecoverySkill();
        fillHpRecoverySkill();
        fillMeleeAttackSkill();
        fillRangeAttackSkill();
        fillMagicAttackSkill();
        fillAreaSkill();
        fillSelfAreaSkill();
        fillTargetAreaSkill();
        fillLongGlobalCooldownTimeSkill();
        fillLongCastTimeSkill();
        fillCombatUseTypeSkill();
        fillChangePeaceSkill();
        fillPeaceUseTypeSkill();
        fillLearnSkill_1();
        fillLearnSkill_2();
        fillLearnSkill_3();
        fillAbsolutenessDefence();
        fillSpellCraft_1();
        fillSpellCraft_2();
    }

    void fillObserverSkillCode();
    void fillStunSkill();
    void fillGraveStoneSkill();
    void fillMpRecoverySkill();
    void fillHpRecoverySkill();
    void fillMeleeAttackSkill();
    void fillRangeAttackSkill();
    void fillMagicAttackSkill();
    void fillAreaSkill();
    void fillSelfAreaSkill();
    void fillTargetAreaSkill();
    void fillLongGlobalCooldownTimeSkill();
    void fillLongCastTimeSkill();
    void fillPeaceUseTypeSkill();
    void fillCombatUseTypeSkill();
    void fillChangePeaceSkill();
    void fillLearnSkill_1();
    void fillLearnSkill_2();
    void fillLearnSkill_3();
    void fillAbsolutenessDefence();
    void fillSpellCraft_1();
    void fillSpellCraft_2();

    void destroySkills();

private:
    SkillMap skillMap_;*/
//};

}} // namespace gideon { namespace servertest {
