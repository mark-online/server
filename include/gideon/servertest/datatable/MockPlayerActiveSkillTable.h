#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/PlayerActiveSkillTable.h>

namespace gideon { namespace servertest {

const SkillCode defaultMeleeAttackSkillCode = makeSkillCode(sttActivePlayer, 1, 1); // 베어넘기기 1Lv
const SkillCode defaultRangeAttackSkillCode = makeSkillCode(sttActivePlayer, 2, 1); // 관통 화살 1Lv
const SkillCode defaultMagicAttackSkillCode = makeSkillCode(sttActivePlayer, 3, 1); // 마법탄 1Lv


const SkillCode learnSkill_1 = makeSkillCode(sttActivePlayer, 4, 1); // 압도 스킬인덱스 4 level_1
const SkillCode learnSkill_2 = makeSkillCode(sttActivePlayer, 4, 2); // 압도 스킬인덱스 4 level_2
const SkillCode learnSkill_3 = makeSkillCode(sttActivePlayer, 4, 3); // 압도 스킬인덱스 4 level_3

const SkillCode defaultTargetAreaSkillCode = makeSkillCode(sttActivePlayer, 5, 1); // 파멸 1Lv(Target Area 아님!!!)
const SkillCode defaultSelfAreaSkillCode = makeSkillCode(sttActivePlayer, 6, 1); // 칼날 폭풍 1Lv


const SkillCode testObserverSkillCode = makeSkillCode(sttActivePlayer, 7, 1); // 임의


const SkillCode defaultLongGlobalCooldownTimeSkillCode = makeSkillCode(sttActivePlayer, 8, 1); // 얼음 방벽 1Lv

const SkillCode defaultLongCastTimeSkillCode = makeSkillCode(sttActivePlayer, 9, 1); // 영원한 화염 1Lv

const SkillCode hpRecoverySkillCode = makeSkillCode(sttActivePlayer, 10, 1); //자연의 결속 1lv
const SkillCode mpRecoverySkillCode = makeSkillCode(sttActivePlayer, 11, 1); // 마나재생 1lv

const SkillCode graveStoneSkillCode = makeSkillCode(sttActivePlayer, 12, 1);	// 집행 1Lv


const SkillCode stunSkillCode = makeSkillCode(sttActivePlayer, 13, 1); // 스턴

// character stat change 데이타는(3개) 다 틀림.
const SkillCode peaceUseTypeSkillCode = makeSkillCode(sttActivePlayer, 14, 1); // 강타 1Lv
const SkillCode combatUseTypeSkillCode = makeSkillCode(sttActivePlayer, 15, 2); // 회오리 바람 2Lv 
const SkillCode changePeaceSkillCode = makeSkillCode(sttActivePlayer, 16, 1);
const SkillCode absolutenessDefence = makeSkillCode(sttActivePlayer, 17, 1); // 절대 방어

const SkillCode defaultAreaAndCastee2SkillCode = makeSkillCode(sttActivePlayer, 19, 1); // 폭풍 1Lv


/**
* @class MockPlayerActiveSkillTable
*/
class GIDEON_SERVER_API MockPlayerActiveSkillTable :
    public gideon::datatable::PlayerActiveSkillTable
{
public:
    MockPlayerActiveSkillTable() {
        fillSkills();
    }

    virtual const gideon::datatable::PlayerActiveSkillTemplate*
        getPlayerSkill(SkillCode code) const override;

    virtual const datatable::SkillClientEffectTemplate*
        getClientEffect(SkillCode code) const override {
        code;
        return nullptr;
    }

private:
    virtual const PlayerSkillMap& getPlayerSkillMap() const override {
        return skillMap_;
    }

    virtual const std::string& getLastError() const override {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const override {
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

private:
    PlayerSkillMap skillMap_;
};

}} // namespace gideon { namespace servertest {
