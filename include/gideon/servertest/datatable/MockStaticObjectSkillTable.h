#pragma once

#include <gideon/Common.h>
//#include <gideon/cs/datatable/StaticObjectSkillTable.h>


namespace gideon { namespace servertest {
//
///**
//* @class MockStaticObjectSkillTable
//*/
//class GIDEON_SERVER_API MockStaticObjectSkillTable : public datatable::StaticObjectSkillTable
//{
//public:
//    MockStaticObjectSkillTable() {
//        fillSkills();
//    }
//    virtual ~MockStaticObjectSkillTable() {
//        destroySkills();
//    }
//
//    virtual const datatable::StaticObjectSkillTemplate* getSkill(StaticObjectSkillCode code) const;
//    virtual const datatable::SkillTemplate* getClientSkill(StaticObjectSkillCode /*code*/) const {
//        return nullptr;
//    }
//private:
//    virtual const StaticObjectSkillMap& getStaticObjectSkillMap() const {
//        return skillMap_;
//    }
//
//    virtual const std::string& getLastError() const {
//        static std::string empty;
//        return empty;
//    }
//
//    virtual bool isLoaded() const {
//        return true;
//    }
//
//private:
//    void fillSkills() {
//        fillSelfAreaEntityHpDamageSkill();
//        fillSelfHpDamageSkill();
//        fillTargetHpDamageSkill();
//    }
//
//    void fillSelfAreaEntityHpDamageSkill();
//    void fillSelfHpDamageSkill();
//    void fillTargetHpDamageSkill();
//
//    void destroySkills();
//
//private:
//    StaticObjectSkillMap skillMap_;
//};
//
}} // namespace gideon { namespace servertest {
