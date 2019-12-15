#pragma once

#include <gideon/cs/shared/data/LootInvenItemInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class BuildingProductionInventoryable
 * 빌딩 아이템을 루팅할수 있다
 */
class BuildingProductionInventoryable
{
public:
    virtual ~BuildingProductionInventoryable() {}

public:
	virtual ErrorCode startLooting(Entity& player, ObjectId itemId, InvenType invenType) = 0;
	virtual ErrorCode completeLooting(Entity& player, ObjectId itemId, InvenType invenType) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {