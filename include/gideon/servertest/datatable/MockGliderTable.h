#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/GliderTable.h>

namespace gideon { namespace servertest {

/**
* @class MockGliderTable
*/
class GIDEON_SERVER_API MockGliderTable : public gideon::datatable::GliderTable
{
public:
	MockGliderTable();
	virtual ~MockGliderTable() {
		destroyGliders();
	}
	virtual const gdt::glider_t* getGlider(GliderCode code) const;

private:
	void destroyGliders();
	void fillGlider();

private:

	virtual const GliderMap& getGliderMap() const {
		return gliderMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	GliderMap gliderMap_;
};

}} // namespace gideon { namespace servertest {