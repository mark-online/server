#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/PlayerPassiveSkillTable.h>

namespace gideon { namespace servertest {

// 패시브 스킬
const SkillCode spellCraft_1 = makeSkillCode(sttPassivePlayer, 18, 1); // 주문학 level 1
const SkillCode spellCraft_2 = makeSkillCode(sttPassivePlayer, 18, 2); // 주문학 level 2

/**
* @class MockPlayerPassiveSkillTable
*/
class GIDEON_SERVER_API MockPlayerPassiveSkillTable :
    public gideon::datatable::PlayerPassiveSkillTable
{
public:
    MockPlayerPassiveSkillTable() {
        fillSkills();
    }

    virtual const datatable::PassiveSkillTemplate*
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
        fillSpellCraft_1();
        fillSpellCraft_2();
    }

    void fillSpellCraft_1();
    void fillSpellCraft_2();

private:
    PlayerSkillMap skillMap_;
};

}} // namespace gideon { namespace servertest {
