#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ItemInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {


class ZoneServer_Export ItemCostService : public boost::noncopyable
{
public:
	static ErrorCode getSellItemCost(CostType& costType, uint32_t& cost, DataCode itemCode);
	static ErrorCode getDefaultBuyItemCost(CostType& costType, uint32_t& cost, DataCode itemCode);
	static ErrorCode getBuyItemCost(CostType& costType, uint32_t& cost, DataCode itemCode);

};

}} // namespace gideon { namespace zoneserver {