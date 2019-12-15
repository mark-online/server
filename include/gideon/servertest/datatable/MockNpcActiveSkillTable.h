#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/NpcActiveSkillTable.h>

namespace gideon { namespace servertest {

/**
* @class MockNpcActiveSkillTable
*/
class GIDEON_SERVER_API MockNpcActiveSkillTable :
    public gideon::datatable::NpcActiveSkillTable
{
public:
    MockNpcActiveSkillTable() {
        //fillSkills();
    }

    virtual const gideon::datatable::NpcActiveSkillTemplate*
        getNpcSkill(SkillCode code) const override;

    virtual const datatable::SkillClientEffectTemplate*
        getClientEffect(SkillCode code) const override {
        code;
        return nullptr;
    }

private:
    virtual const NpcSkillMap& getNpcSkillMap() const override {
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
    NpcSkillMap skillMap_;
};

}} // namespace gideon { namespace servertest {
