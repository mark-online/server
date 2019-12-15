#pragma once

#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/RegionInfo.h>
#include <gideon/cs/shared/data/QuestInfo.h>
#include <gideon/cs/shared/data/DeviceInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {

/***
 * @class Quest
 ***/
class Quest
{
public:
	virtual ~Quest() {}
};

}} // namespace gideon { namespace zoneserver {