#pragma once


namespace gideon { namespace zoneserver { namespace gc {
class SkillController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Skillable
 * 스킬을 사용할수 있다
 */
class Skillable
{
public:
    virtual ~Skillable() {}
	
public:
	virtual std::unique_ptr<gc::SkillController> createSkillController() = 0;

    virtual msec_t getCalcCastingTime(msec_t castingTime) const = 0;

public:
	virtual gc::SkillController& getSkillController() = 0;
	virtual const gc::SkillController& getSkillController() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
