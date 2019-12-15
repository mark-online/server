#pragma once

#include <gideon/cs/shared/data/LootInvenItemInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class SOInventoryable
 * 아이템을 루팅할수 있다
 */
class SOInventoryable
{
public:
    virtual ~SOInventoryable() {}

public:
    virtual LootInvenItemInfoMap getLootInvenItemInfoMap() const = 0;

	virtual ErrorCode startLooting(Entity& player, LootInvenId id) = 0;
	virtual ErrorCode completeLooting(Entity& player, LootInvenId id) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {