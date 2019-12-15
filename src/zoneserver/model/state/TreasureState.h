#pragma once

#include <gideon/cs/shared/data/TreasureInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class TreasureState
 **/
class TreasureState
{
public:
	virtual ~TreasureState() {}

	virtual bool canTreasureOpen() const = 0;

    virtual bool isTreasureOpening() const = 0;
};


}} // namespace gideon { namespace zoneserver {
