#pragma once

#include <gideon/cs/shared/data/HarvestInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class HarvestState
 **/
class HarvestState
{
public:
	virtual ~HarvestState() {}

	virtual bool canHarvest() const = 0;

    virtual bool isHarvesting() const = 0;
};


}} // namespace gideon { namespace zoneserver {
