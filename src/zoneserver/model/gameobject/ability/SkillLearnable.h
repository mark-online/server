#pragma once

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class SkillLearnable
 * 설치물을 생성 있다
 */
class SkillLearnable
{
public:
    virtual ~SkillLearnable() {}
	
public:
	virtual ErrorCode learnSkill(SkillTableType skillTableType, SkillIndex index) = 0;
	virtual ErrorCode unlearnSkill(SkillTableType skillTableType, SkillIndex index) = 0;
    virtual ErrorCode learnSkills(const SkillCodes& skillCodes) = 0;
    virtual ErrorCode resetLearnedSkills() = 0;

    virtual SkillPoint getSkillPoint() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
