#include "ZoneServerPCH.h"
#include "GraveStoneService.h"
#include "ItemDropService.h"
#include "../spawn/EntityPool.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../model/gameobject/allocator/EntityAllocator.h"
#include "../../model/gameobject/GraveStone.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/item/Inventory.h"
#include "../../controller/GraveStoneController.h"
#include "../../controller/PlayerController.h"
#include "esut/Random.h"
#include <gideon/cs/shared/data/ItemDropInfo.h>
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/cs/datatable/ItemDropTable.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/GemTable.h>
#include <gideon/cs/datatable/WorldDropTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <sne/base/utility/Logger.h>


namespace gideon { namespace zoneserver {

namespace {
     
/**
 * @class GraveStoneDespawnTask
 */
class GraveStoneDespawnTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<GraveStoneDespawnTask>
{
public:
    GraveStoneDespawnTask(go::GraveStone& graveStone) :
        graveStone_(graveStone) {}

private:
    virtual void run() {
        GRAVE_STONE_SERVICE->unregisterGraveStone(graveStone_);
    }

private:
    go::GraveStone& graveStone_;
};


/**
 * @class GraveStoneAllocator
 */
class GraveStoneAllocator : public go::EntityAllocator
{
public:
    static GraveStoneAllocator& instance() {
        static GraveStoneAllocator s_graveStoneAllocator;
        return s_graveStoneAllocator;
    }

public:
    virtual go::Entity* malloc() {
        return new go::GraveStone(std::make_unique<gc::GraveStoneController>());
    }

    void free(go::Entity* stone) {
        boost::checked_delete(static_cast<go::GraveStone*>(stone));
    }
};


inline HitPoint getGraveStoneMaxHp(HitPoint minPoint, HitPoint maxPoint)
{
	// TODO 나중에 저항 넣기
    return static_cast<HitPoint>(esut::random(minPoint, maxPoint));
}


inline ObjectPosition getNpcGraveStoneSpawnPosition(const go::Npc& npc)
{
    ObjectPosition position = npc.getPosition();
    position.x_ += esut::random(-1.0f, 1.0f);
    position.y_ += esut::random(-1.0f, 1.0f);
    return position;
}

} // namespace 

// = GraveStoneService

SNE_DEFINE_SINGLETON(GraveStoneService);

GraveStoneService::GraveStoneService() :
    createDespawnTime_(
        GIDEON_PROPERTY_TABLE->getPropertyValue<GameTime>(L"gravestone_created_despawn_time")),
	clickDespawnTime_(
		GIDEON_PROPERTY_TABLE->getPropertyValue<GameTime>(L"gravestone_click_despawn_time")),
	minHitPoint_(toHitPoint(GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"gravestone_min_life_point"))),
	maxHitPoint_(toHitPoint(GIDEON_PROPERTY_TABLE->getPropertyValue<int>(L"gravestone_max_life_point"))),
    resultSkip_(false)
{
    graveStonePool_ = std::make_unique<EntityPool>(GraveStoneAllocator::instance(), 1000);
}


GraveStoneService::~GraveStoneService()
{
}


bool GraveStoneService::initialize(bool resultSkip)
{
    resultSkip_ = resultSkip;
    GraveStoneService::instance(this);
    return true;
}


go::GraveStone* GraveStoneService::registerGraveStone(const go::Npc& npc, WorldMap& worldMap,
    go::Entity* from)
{
    if (from != nullptr) {
        if (from->isNpc()) {
            if (nullptr == static_cast<go::Npc&>(*from).getTopScorePlayer()) {
                return nullptr;
            }
        }
        else if (! canStandGraveStone(from->getObjectType())) {
            return nullptr;
        }
    }
    const datatable::NpcTemplate& npcTemplate = npc.getNpcTemplate();

    LootInvenItemInfoMap lootInvenItemInfoMap;
    ItemDropService::fillItemDropInfo(lootInvenItemInfoMap, npcTemplate.getItemDropId1());
    ItemDropService::fillWorldDropInfo(lootInvenItemInfoMap, npcTemplate.getWorldDropSuffixId1());
    ItemDropService::fillItemDropInfo(lootInvenItemInfoMap, npcTemplate.getItemDropId2());
    ItemDropService::fillWorldDropInfo(lootInvenItemInfoMap, npcTemplate.getWorldDropSuffixId2()); 
    if (lootInvenItemInfoMap.empty()) {
        return nullptr;
    }

    go::GraveStone* graveStone = graveStonePool_->acquireEntity<go::GraveStone>();
    if (! graveStone) {
        assert(false);
        return nullptr;
    }

    const ObjectPosition spawnPosition = getNpcGraveStoneSpawnPosition(npc);

    const ObjectId objectId = graveStonePool_->generateObjectId();
    const UnionGraveStoneName graveStoneName(otNpc, npcTemplate.getNpcCode());
    if (! graveStone->initialize(objectId, graveStoneName, spawnPosition,
        lootInvenItemInfoMap)) {
        graveStone->finalize();
        return nullptr;
    }

    graveStone->setWorldMap(worldMap);
    graveStone->setMapRegion(worldMap.getGlobalMapRegion());

    const ErrorCode errorCode = graveStone->spawn(worldMap, spawnPosition);
    if (isFailed(errorCode)) {
        graveStonePool_->releaseEntity(graveStone);
        return nullptr;
    }

    addGraveStone(graveStone);
    return graveStone;
}


go::GraveStone* GraveStoneService::registerGraveStone(go::Entity& player, WorldMap& worldMap)
{
    LootInvenItemInfoMap itemInfoMap;
    const ObjectPosition position = player.getPosition();
    player.queryInventoryable()->robbed(itemInfoMap);

    if (itemInfoMap.empty()) {
        return nullptr;
    }

    go::GraveStone* graveStone = graveStonePool_->acquireEntity<go::GraveStone>();
    if (! graveStone) {
        assert(false);
        return nullptr;
    }

    const ObjectId objectId = graveStonePool_->generateObjectId();
    const UnionGraveStoneName graveStoneName(otPc, player.getNickname());
    if (! graveStone->initialize(objectId, graveStoneName, position, itemInfoMap, 
		getGraveStoneMaxHp(minHitPoint_, maxHitPoint_))) {
        graveStone->finalize();
        graveStonePool_->releaseEntity(graveStone);
        return nullptr;
    }

    graveStone->setWorldMap(worldMap);
	graveStone->setMapRegion(worldMap.getGlobalMapRegion());
    const ErrorCode errorCode = graveStone->spawn(worldMap, position);
    if (isFailed(errorCode)) {
        graveStonePool_->releaseEntity(graveStone);
        return nullptr;
    }

    addGraveStone(graveStone);
    return graveStone;
}


void GraveStoneService::unregisterGraveStone(go::GraveStone& graveStone)
{
    static_cast<go::Entity&>(graveStone).despawn();

    removeGraveStone(&graveStone);

    graveStone.finalize();
}


sne::base::Future::Ref GraveStoneService::scheduleGraveStoneDespawnTask(go::GraveStone& graveStone, bool isCreated)
{
    if (resultSkip_) {
        return sne::base::Future::Ref();
    }

	return TASK_SCHEDULER->schedule(std::make_unique<GraveStoneDespawnTask>(graveStone),
        isCreated ? createDespawnTime_ : clickDespawnTime_);
}


bool GraveStoneService::addGraveStone(go::GraveStone* graveStone)
{
    std::lock_guard<LockType> lock(lock_);

    return graveStoneMap_.insert(GraveStoneMap::value_type(
        graveStone->getObjectId(), graveStone)).second;
}


void GraveStoneService::removeGraveStone(go::GraveStone* graveStone)
{
    {
        std::lock_guard<LockType> lock(lock_);

        graveStoneMap_.erase(graveStone->getObjectId());
    }

    graveStonePool_->releaseEntity(graveStone);
}

}} // namespace gideon { namespace zoneserver {
