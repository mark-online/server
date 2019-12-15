#include "ZoneServerPCH.h"
#include "GraveStone.h"
#include "EntityEvent.h"
#include "skilleffect/StaticObjectEffectScriptApplier.h"
#include "status/StaticObjectStatus.h"
#include "ability/Castable.h"
#include "ability/CastNotificationable.h"
#include "ability/Inventoryable.h"
#include "../item/LootItemInventory.h"
#include "../item/Inventory.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../controller/callback/GraveStoneCallback.h"
#include "../../service/item/ItemIdGenerator.h"
#include "../../service/item/GraveStoneService.h"
#include "../../service/time/GameTimer.h"
#include "../../world/WorldMap.h"
#include "../../world/World.h"
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver { namespace go {

// = GraveStone

GraveStone::GraveStone(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller))
{
}


GraveStone::~GraveStone()
{
}


bool GraveStone::initialize(ObjectId objectId,const UnionGraveStoneName& graveStoneName, 
    const ObjectPosition& spawnPosition, const LootInvenItemInfoMap& graveStoneItemMap, 
    HitPoint hp)
{
    assert(isValidObjectId(objectId));

    std::lock_guard<LockType> lock(getLock());

    if (! Parent::initialize(otGraveStone, objectId)) {
        return false;
    }

    graveStoneInfo_.objectType_ = getObjectType();
    graveStoneInfo_.objectId_ = objectId;
    graveStoneInfo_.position_ = spawnPosition;
    graveStoneInfo_.graveStoneName_ = graveStoneName;
    lootInventory_ = std::make_unique<LootItemInventory>(*this, graveStoneItemMap);
    assert(graveStoneInfo_.isValid());

    initStaticObjectStatusInfo(StaticObjectStatusInfo(HitPoints(hp, hp)));


    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    entityInfo.objectType_ = getObjectType();
    MoreGraveStoneInfo& moreGraveStoneInfo = entityInfo.asGraveStoneInfo();
    static_cast<GraveStoneInfo&>(moreGraveStoneInfo) = graveStoneInfo_;

    startGraveStoneDespawnTask(true);
    return true;
}


void GraveStone::finalize()
{
    {
        std::lock_guard<LockType> lock(getLock());

        reset();
    }

    Parent::finalize();
}


bool GraveStone::isBroken() const
{
    // TODO: Anchor
    return false;
}


bool GraveStone::shouldDespawn() const
{
    std::lock_guard<LockType> lock(getLock());

    return lootInventory_->getLootInvenItemInfoMap().empty();
}


void GraveStone::restartGraveStoneDespawnTask()
{
    cancelGraveStoneDespawnTask();
    startGraveStoneDespawnTask(false);
}


LootInvenItemInfoMap GraveStone::getLootInvenItemInfoMap() const
{
    std::lock_guard<Entity::LockType> lock(getLock());

    return lootInventory_->getLootInvenItemInfoMap();
}


ErrorCode GraveStone::startLooting(Entity& player, LootInvenId id)
{
    // TODO: Anchor
    //if (! graveStoneInfo_.isMinHp()) {
    //	return ecGraveStoneNotMinHp;
    //}
    //
    const ErrorCode ec = lootInventory_->startLooting(player, id);
    if (isSucceeded(ec)) {
        restartGraveStoneDespawnTask();
    }
    return ec;
}


ErrorCode GraveStone::completeLooting(Entity& player, LootInvenId id)
{
    ErrorCode ec = lootInventory_->completeLooting(player, id);

    if (isFailed(ec)) {
        return ec;
    }

    if (shouldDespawn()) {
        gc::GraveStoneCallback* graveStoneCallback = player.getController().queryGraveStoneCallback();
        if (graveStoneCallback) {
            graveStoneCallback->destoryed(*this);
        }
    }

    return ec;
}

// = Tickable overriding

void GraveStone::tick(GameTime /*diff*/)
{
    getEffectScriptApplier().tick();
    
}


std::unique_ptr<EffectScriptApplier> GraveStone::createEffectScriptApplier()
{
    return std::make_unique<StaticObjectEffectScriptApplier>(*this);
}


GraveStoneInfo GraveStone::getBaseGraveStoneInfo() const
{
    std::lock_guard<LockType> lock(getLock());

    return graveStoneInfo_;
}


void GraveStone::reset()
{
    cancelGraveStoneDespawnTask();

    graveStoneInfo_.reset();
}


void GraveStone::startGraveStoneDespawnTask(bool isCreate)
{
    despawnTask_ = GRAVE_STONE_SERVICE->scheduleGraveStoneDespawnTask(*this, isCreate);
}


void GraveStone::cancelGraveStoneDespawnTask()
{
    if (! despawnTask_.get()) {
        return;
    }

    despawnTask_->cancel();
    despawnTask_.reset();
}

// = Positionable overriding

void GraveStone::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);

    graveStoneInfo_.position_ = position;
}


void GraveStone::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);

    graveStoneInfo_.position_.heading_ = heading;
}


WorldPosition GraveStone::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(graveStoneInfo_.position_, worldMap->getMapCode());
}


ErrorCode GraveStone::despawn()
{
    if (graveStoneInfo_.graveStoneName_.ownerType_ == otPc) {
        Entity* entity = WORLD->getPlayer(graveStoneInfo_.graveStoneName_.nickname_);
        if (entity) {
            entity->getController().queryGraveStoneCallback()->graveStoneRemoved();
        }
    }
    return Entity::despawn();    
}

}}} // namespace gideon { namespace zoneserver { namespace go {
