#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockGliderTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockGliderTable::MockGliderTable()
{
	fillGlider();
}


const gdt::glider_t* MockGliderTable::getGlider(GliderCode code) const
{
	const GliderMap::const_iterator pos = gliderMap_.find(code);
	if (pos != gliderMap_.end()) {
		return (*pos).second;
	}
	return nullptr;
}


void MockGliderTable::fillGlider()
{
	gdt::glider_t* glider = new gdt::glider_t;
	glider->glider_code(gliderCode1);
	glider->cast_time(0);
	gliderMap_.emplace(gliderCode1, glider);
}


void MockGliderTable::destroyGliders()
{
	for (GliderMap::value_type& value : gliderMap_) {
		delete value.second;
	}
}



}} //namespace gideon { namespace servertest {
