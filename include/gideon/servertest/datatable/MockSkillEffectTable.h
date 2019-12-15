#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/SkillEffectTable.h>

namespace gideon { namespace servertest {

/**
* @class MockSkillEffectTable
*/
class GIDEON_SERVER_API MockSkillEffectTable : public gideon::datatable::SkillEffectTable
{
public:
	MockSkillEffectTable();

	virtual const gideon::datatable::SkillEffectTemplate*
        getSkillEffectTemplate(EffectIndex effectIndex) const;

private:
	virtual const SkillEffectMap& getSkillEffectMap() const {
		return effectMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	SkillEffectMap effectMap_;
};

}} // namespace gideon { namespace servertest {
