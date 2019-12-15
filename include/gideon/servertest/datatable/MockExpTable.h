#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/ExpTable.h>

namespace gideon { namespace servertest {

/**
* @class MockExpTable
*/
class GIDEON_SERVER_API MockExpTable : public gideon::datatable::ExpTable
{
public:
	MockExpTable();

	virtual ExpPoint getExpPoint(CreatureLevel level, LevelStep levelStep) const {
        assert(isValidPlayerLevel(level) && isValid(levelStep));
		return expPoints_[level][levelStep]; 
	}

private:
	virtual const ExpPoints& getExpPoints() const {
		return expPoints_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	ExpPoints expPoints_;
};

}} // namespace gideon { namespace servertest {