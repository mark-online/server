#pragma once

#include "StaticObject.h"
#include "ability/SOInventoryable.h"
#include "ability/Tickable.h"
#include <gideon/cs/shared/data/GraveStoneInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver { 
class LootItemInventory;
}} // namespace gideon { namespace zoneserver { 

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class GraveStone
 * 비석
 */
class GraveStone : public StaticObject,
	public SOInventoryable,
	public Tickable
{
    typedef StaticObject Parent;

public:        
    GraveStone(std::unique_ptr<gc::EntityController> controller);
    virtual ~GraveStone();

	bool initialize(ObjectId objectId, const UnionGraveStoneName& graveStoneName,
        const ObjectPosition& spawnPosition, const LootInvenItemInfoMap& graveStoneItemMap,
        HitPoint hp = hpMin);
    virtual void finalize();

    bool isBroken() const;

public:
    GraveStoneInfo getBaseGraveStoneInfo() const;
	
    bool shouldDespawn() const;
    void restartGraveStoneDespawnTask();

private:
	// SOInventoryable
	virtual LootInvenItemInfoMap getLootInvenItemInfoMap() const;

	virtual ErrorCode startLooting(Entity& player, LootInvenId id);
	virtual ErrorCode completeLooting(Entity& player, LootInvenId id);

private:
	// = Tickable overriding
	virtual void tick(GameTime diff);

private:
	virtual SOInventoryable* querySOInventoryable() {
		return this;
	}
	virtual Tickable* queryTickable() {return this;}

private:
    // = Entity overriding
    std::unique_ptr<EffectScriptApplier> createEffectScriptApplier();

private:
    // = Positionable overriding
    virtual void setPosition(const ObjectPosition& position);
    virtual void setHeading(Heading heading);

    virtual WorldPosition getWorldPosition() const;

private:
    virtual ErrorCode despawn();
	
private:
    void reset();
    
    void startGraveStoneDespawnTask(bool isCreate);
    void cancelGraveStoneDespawnTask();

private:
    GraveStoneInfo graveStoneInfo_;
    std::unique_ptr<LootItemInventory> lootInventory_;
    
    sne::base::Future::Ref despawnTask_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
