#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ItemInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {


class ZoneServer_Export ItemOptionService : public boost::noncopyable
{
public:
	static ItemScriptInfos getItemOptions(ItemSuffixId id);
	
};

}} // namespace gideon { namespace zoneserver {