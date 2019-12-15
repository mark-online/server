#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/SOActiveSkillTable.h>

namespace gideon { namespace servertest {

/**
* @class MockSOActiveSkillTable
*/
class GIDEON_SERVER_API MockSOActiveSkillTable :
    public gideon::datatable::SOActiveSkillTable
{
public:
    MockSOActiveSkillTable() {
        //fillSkills();
    }

    virtual const gideon::datatable::SOActiveSkillTemplate*
        getSOSkill(SkillCode code) const override;

    virtual const datatable::SkillClientEffectTemplate*
        getClientEffect(SkillCode code) const override {
        code;
        return nullptr;
    }

private:
    virtual const SOSkillMap& getSOSkillMap() const override {
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
    SOSkillMap skillMap_;
};

}} // namespace gideon { namespace servertest {
