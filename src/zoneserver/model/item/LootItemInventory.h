#pragma once

#include <gideon/cs/shared/data/LootInvenItemInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

/**
 * @class LootItemInventory
 * 아이템을 루팅할수 있다
 */
class LootItemInventory
{
public:
	LootItemInventory(go::Entity& owner, const LootInvenItemInfoMap& ItemInfoMap);
    virtual ~LootItemInventory() {}

public:
    const LootInvenItemInfoMap& getLootInvenItemInfoMap() const {
		return ItemInfoMap_;
	}

	ErrorCode startLooting(go::Entity& player, LootInvenId id);
	ErrorCode completeLooting(go::Entity& player, LootInvenId id);

private:
	const LootInvenItemInfo* getItemInfo(LootInvenId lootInvenId) const;

private:
	go::Entity& owner_;
	LootInvenItemInfoMap ItemInfoMap_;
};

}} // namespace gideon { namespace zoneserver {