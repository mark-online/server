#include "ZoneServerPCH.h"
#include "Treasure.h"
#include "ability/Castable.h"
#include "ability/Inventoryable.h"
#include "ability/Castnotificationable.h"
#include "ability/Achievementable.h"
#include "status/StaticObjectStatus.h"
#include "../item/LootItemInventory.h"
#include "../item/Inventory.h"
#include "../../controller/NpcController.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../world/WorldMap.h"
#include "../../world/region/MapRegion.h"
#include "../../service/time/GameTimer.h"
#include "../../service/distance/DistanceChecker.h"
#include "../../service/item/ItemDropService.h"
#include "../../service/spawn/SpawnService.h"
#include <gideon/cs/datatable/TreasureTable.h>
#include <gideon/cs/datatable/RecipeTable.h>

namespace gideon { namespace zoneserver { namespace go {

namespace {


/**
 * @class OpeningTask
 */
class OpeningTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<OpeningTask>
{
public:
    OpeningTask(Entity& entity, Treasure& treasure) :
        entity_(entity),
        treasure_(treasure) {}

private:
    virtual void run() {
        const ErrorCode errorCode = treasure_.open(entity_);
        if (isFailed(errorCode)) {
            // TODO: 에러 리턴??
        }
    }

private:
    Entity& entity_;
    Treasure& treasure_;
};

} // namespace {

// = Treasure

bool Treasure::notUseTimer_ = false;

void Treasure::setNotUseTimer()
{
    notUseTimer_ = true;
}


Treasure::Treasure(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller)),
    ownerId_(invalidObjectId)
{
}


Treasure::~Treasure()
{
}


bool Treasure::initialize(ObjectId treasureId, TreasureCode treasureCode)
{
    treasureTemplate_ = TREASURE_TABLE->getTreasure(treasureCode);
    if (! treasureTemplate_) {
        return false;
    }

    {
        std::lock_guard<LockType> lock(getLock());

        if (! Parent::initialize(otTreasure, treasureId)) {
            return false;
        }

        treasureInfo_.treasureCode_ = treasureCode;
        treasureInfo_.objectType_ = otTreasure;
        treasureInfo_.objectId_ = treasureId;
        treasureInfo_.isOpen_ = false;

        UnionEntityInfo& info = getUnionEntityInfo_i();
        MoreTreasureInfo& moreTreasureInfo = info.asTreasureInfo();
        static_cast<TreasureInfo&>(moreTreasureInfo) = treasureInfo_;
    }
    return true;
}


void Treasure::finalize()
{
    cancelDespawn();

    reset();

    Parent::finalize();
}


ErrorCode Treasure::respawn(WorldMap& worldMap)
{
    reset();

    return Parent::respawn(worldMap);
}


void Treasure::reserveDespawn()
{
    reserveDespawn(treasureTemplate_->live_mil_sec());

}


void Treasure::reset()
{
    std::lock_guard<LockType> lock(getLock());

    treasureInfo_.isOpen_ = false;

    UnionEntityInfo& info = getUnionEntityInfo_i();
    MoreTreasureInfo& moreTreasureInfo = info.asTreasureInfo();
    moreTreasureInfo.isOpen_ = false;

    ownerId_ = invalidObjectId; 
}


void Treasure::reserveDespawn(GameTime tick)
{
    if (notUseTimer_) {
        return;
    }

    if (getCurrentWorldMap()->isDungeon()) {
        return;
    }

    sne::base::Future::Ref despawnHarvestTask = despawnTask_.lock();
    if (despawnHarvestTask.get()) {
        return;
    }

    despawnTask_ = SPAWN_SERVICE->scheduleDespawn(*this, tick);
}


ErrorCode Treasure::startOpenTreasure(Entity& entity)
{
    if (isValidObjectId(ownerId_)) {
        if (entity.getObjectId() != ownerId_) {
            return ecTreasureOrderOwnerTreasure;
        }
    }

    if (! DISTANCE_CHECKER->checkTreasureDistance(getPosition(), entity.getPosition())) {
        return ecTreasureTargetTooFar;
    }

    if (treasureInfo_.isOpen_) {
        return ecTreasureCannotOpenState;
    }

    ownerId_ = entity.getObjectId();

    cancelDespawn();
    const GameTime oneMin = 60 * 1000;
    reserveDespawn(oneMin);

    const GameTime openInterval = treasureTemplate_->open_mil_sec();    

    if (openInterval <= 0) {
        return open(entity);
    }

    Castable* castable = entity.queryCastable();
    CastNotificationable* notificationable = entity.queryCastNotificationable();
    if (! castable & ! notificationable) {
        return ecServerInternalError;
    }

    auto task = std::make_unique<OpeningTask>(entity, *this);
    castable->startCasting(treasureInfo_.treasureCode_, false, std::move(task), openInterval);
    StartCastResultInfo startCastInfo;
    startCastInfo.set(uctTreasure, entity.getGameObjectInfo(), getGameObjectInfo());
    notificationable->notifyStartCasting(startCastInfo);
    return ecOk;
}


ErrorCode Treasure::open(Entity& entity)
{
    Castable* castable = entity.queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

    CastNotificationable* notificationable = entity.queryCastNotificationable();
    if (notificationable) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(uctTreasure, entity.getGameObjectInfo(), getGameObjectInfo());
        notificationable->notifyCompleteCasting(comleteInfo);
    }

    
    {
        std::lock_guard<LockType> lock(getLock());

        LootInvenItemInfoMap treasureItemInfoMap;

        if (ownerId_ != entity.getObjectId()) {
            return ecTreasureOrderOwnerTreasure;
        }

        ItemDropService::fillItemDropInfo(treasureItemInfoMap, treasureTemplate_->item_drop_id());
        ItemDropService::fillWorldDropInfo(treasureItemInfoMap, treasureTemplate_->world_drop_id());

        lootInventory_ = std::make_unique<LootItemInventory>(*this, treasureItemInfoMap);
        UnionEntityInfo& info = getUnionEntityInfo_i();
        MoreTreasureInfo& moreTreasureInfo = info.asTreasureInfo();
        moreTreasureInfo.isOpen_ = true;
        treasureInfo_.isOpen_ = true;
    }

    Achievementable* achievementable = entity.queryAchievementable();
    if (achievementable) {
        achievementable->updateAchievement(acmtTreasure, this);
    }

    return ecOk;
}


bool Treasure::canClose(ObjectId playerId) const
{
    return ownerId_ == playerId;
}


void Treasure::cancelOpenTreasure(ObjectId ownerId)
{
    std::lock_guard<LockType> lock(getLock());

    if (ownerId_ == ownerId) {
        ownerId_ = invalidObjectId;
    }
}


void Treasure::cancelDespawn()
{
    if (notUseTimer_) {
        return;
    }
    sne::base::Future::Ref despawnHarvestTask = despawnTask_.lock();
    if (! despawnHarvestTask.get()) {
        return;
    }

    despawnHarvestTask->cancel();
    despawnHarvestTask.reset();
    despawnTask_.reset();
}

// = Positionable overriding

void Treasure::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);
    treasureInfo_.position_ = position;
}


void Treasure::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);
    treasureInfo_.position_.heading_ = heading;

}


WorldPosition Treasure::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(treasureInfo_.position_, worldMap->getMapCode());
}


LootInvenItemInfoMap Treasure::getLootInvenItemInfoMap() const
{
    std::lock_guard<LockType> lock(getLock());

    if (! lootInventory_.get()) {
        return LootInvenItemInfoMap();
    }
    return lootInventory_->getLootInvenItemInfoMap();
}


ErrorCode Treasure::startLooting(Entity& player, LootInvenId id)
{
    std::lock_guard<LockType> lock(getLock());

    if (ownerId_ != player.getObjectId()) {
        return ecTreasureOrderOwnerTreasure;
    }

    if (! treasureInfo_.isOpen_) {
        return ecTreasureIsNotOpen;
    }

    if (! lootInventory_.get()) {
        return ecServerInternalError;
    }

    return lootInventory_->startLooting(player, id);
}


ErrorCode Treasure::completeLooting(Entity& player, LootInvenId id)
{
    if (! lootInventory_.get()) {
        return ecServerInternalError;
    }
    return lootInventory_->completeLooting(player, id);
}



}}} // namespace gideon { namespace zoneserver { namespace go {
