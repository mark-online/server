#pragma once

#include "StaticObject.h"
#include "ability/SOInventoryable.h"
#include <gideon/cs/shared/data/TreasureInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/base/concurrent/Future.h>


namespace gdt {
class treasure_t;
} // namespace gdt {

namespace gideon { namespace zoneserver { 
class LootItemInventory;
}} // namespace gideon { namespace zoneserver { 

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Treasure
 * 보물상자
 */
class ZoneServer_Export Treasure : public StaticObject,
	public SOInventoryable
{
    typedef StaticObject Parent;

public:
    static void setNotUseTimer();

public:
    Treasure(std::unique_ptr<gc::EntityController> controller);
    virtual ~Treasure();

	bool initialize(ObjectId treasureId, TreasureCode treasureCode);

    ErrorCode startOpen(ObjectId playerId);

	ErrorCode startOpenTreasure(Entity& enity);
	ErrorCode open(Entity& entity);
	ErrorCode acquireTreasure(Entity& player, LootInvenId treasureItemId);

public:
	bool canClose(ObjectId playerId) const;

    void cancelOpenTreasure(ObjectId playerId);

private:
    void cancelDespawn();

    void reset();
    void reserveDespawn(GameTime tick);
private:
	virtual SOInventoryable* querySOInventoryable() {
		return this;
	}

private:
    virtual void finalize();

    virtual ErrorCode respawn(WorldMap& worldMap);
    virtual void reserveDespawn();

    virtual DataCode getEntityCode() const {
        return treasureInfo_.treasureCode_;
    }

private:
	// = Positionable overriding
	virtual void setPosition(const ObjectPosition& position);
	virtual void setHeading(Heading heading);

	virtual WorldPosition getWorldPosition() const;

private:
	virtual LootInvenItemInfoMap getLootInvenItemInfoMap() const;

	virtual ErrorCode startLooting(Entity& player, LootInvenId id);
	virtual ErrorCode completeLooting(Entity& player, LootInvenId id);

private:
    static bool notUseTimer_;
    const gdt::treasure_t* treasureTemplate_;
	ObjectId ownerId_;

	TreasureInfo treasureInfo_;
    std::unique_ptr<LootItemInventory> lootInventory_;
    
    sne::base::Future::WeakRef despawnTask_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
