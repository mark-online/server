#include "ZoneServerPCH.h"
#include "Entity.h"
#include "EntityEvent.h"
#include "ability/Knowable.h"
#include "ability/Factionable.h"
#include "ability/TargetSelectable.h"
#include "skilleffect/EntityEffectScriptApplier.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/EntityActionCallback.h"
#include "../../world/WorldMap.h"
#include "../../world/region/SpawnMapRegion.h"
#include "../../service/spawn/SpawnService.h"
#include "../../service/skill/helper/EffectHelper.h"
#include "../../helper/CastChecker.h"
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/template/entity_path_table.hxx>
#include <sne/base/utility/Assert.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class EntityDestroyTask
 */
class EntityDestroyTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<EntityDestroyTask>
{
public:
    EntityDestroyTask(go::Entity& entity) :
        entity_(entity) {}

private:
    virtual void run() {
        if (isValid(entity_.getDestoryObjectType())) {
            entity_.finalize();
            SPAWN_SERVICE->destroyEntity(entity_.getDestoryObjectType(), entity_);
        }
        else {
            assert(false && "invalid ot");
        }
    }

private:
    go::Entity& entity_;
};


/**
 * @class PlayActionEvent
 */
class PlayActionEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayActionEvent>
{
public:
    PlayActionEvent(const GameObjectInfo& entityInfo, uint32_t actionCode) :
        entityInfo_(entityInfo),
        actionCode_(actionCode) {}

private:
    virtual void call(go::Entity& entity) {
        gc::EntityActionCallback* actionCallback =
            entity.getController().queryEntityActionCallback();
        if (actionCallback != nullptr) {
            actionCallback->actionPlayed(entityInfo_, actionCode_);
        }
    }

private:
    const GameObjectInfo entityInfo_;
    const uint32_t actionCode_;
};


inline void scheduleEventCallTask(Entity& entity, go::EntityEvent::Ref event)
{
    (void)TASK_SCHEDULER->schedule(std::make_unique<go::EventCallTask>(entity, event));
}

} // namespace

Entity::Entity(std::unique_ptr<gc::EntityController> controller) :
    controller_(std::move(controller)),
    currentWorldMap_(nullptr),
    currentMapRegion_(nullptr),
    regionSequence_(0),
    entityPath_(nullptr),
    isPathForward_(true),
    currentPathIndex_(0),
    entityDestoryObjectType_(otInvalid)
{
    SNE_ASSERT(controller_.get() != nullptr);
    castChecker_= std::make_unique<CastChecker>(*this);
    controller_->setOwner(*this);
}


Entity::~Entity()
{
}


bool Entity::initialize(ObjectType objectType, ObjectId objectId)
{
    std::unique_lock<LockType> lock(lockThis_);
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    currentWorldMap_ = nullptr;
    currentMapRegion_ = nullptr;

    if (! Parent::initialize(objectType, objectId)) {
        return false;
    }

    castChecker_= std::make_unique<CastChecker>(*this);
    effectHelper_.reset(createEffectHelper().release());

    entityDestoryObjectType_ = objectType;

    if (! effectApplier_) {
        effectApplier_.reset(createEffectScriptApplier().release());
        effectApplier_->initialize();
    }
    else {
        if (effectApplier_->isEmptyCommand()) {
            effectApplier_->initialize();
        }
    }

    unionEntityInfo_.objectType_ = objectType;
    return true;
}


void Entity::finalize()
{
    //assert(! isEnteredAtWorld());

    despawned(); // 재확인


    sne::base::Future::Ref respawnTask = respawnTask_.lock();
    if (respawnTask.get() != nullptr) {
        respawnTask->cancel();
        respawnTask_.reset();
    }

    sne::base::Future::Ref despawnTask = despawnTask_.lock();
    if (despawnTask.get() != nullptr) {
        despawnTask->cancel();
        despawnTask_.reset();
    }

    std::unique_lock<LockType> lock(lockThis_);
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    if (effectApplier_.get() != nullptr) {
        effectApplier_->destroy();
    }

    unionEntityInfo_.reset();
    effectHelper_.reset();
    currentWorldMap_ = nullptr;
    currentMapRegion_ = nullptr;
    entityPath_ = nullptr;

    Parent::finalize();
}


ErrorCode Entity::spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition)
{
    sne::base::Future::Ref respawnTask = respawnTask_.lock();
    if (respawnTask.get() != nullptr) {
        respawnTask->cancel();
        respawnTask_.reset();
    }

    const ErrorCode errorCode = worldMap.spawn(*this, spawnPosition);
    if (isFailed(errorCode)) {
        assert(false);
        return errorCode;
    }

    {
        std::unique_lock<LockType> lock(lockThis_);

        homePosition_ = spawnPosition;
    }
    return ecOk;
}


ErrorCode Entity::respawn(WorldMap& worldMap)
{
    return spawn(worldMap, getNextSpawnPosition());
}


ErrorCode Entity::despawn()
{
    WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return ecWorldMapNotEntered;
    }

    const ErrorCode errorCode = worldMap->despawn(*this);
    if (isSucceeded(errorCode)) {
        despawned();
    }
    return errorCode;
}


void Entity::reserveRespawn(WorldMap& worldMap)
{
    const msec_t spawnDelay = getSpawnTemplate().getSpawnInterval();
    if (spawnDelay <= 0) {
        if (! isGraveStone()) {
            TASK_SCHEDULER->schedule(std::make_unique<EntityDestroyTask>(*this), 10000);
        }
        return;
    }

    sne::base::Future::Ref respawnTask = respawnTask_.lock();
    if (respawnTask.get() != nullptr) {
        respawnTask->cancel();
        respawnTask_.reset();
    }

    respawnTask_ = SPAWN_SERVICE->scheduleRespawn(*this, worldMap);
}


void Entity::reserveDespawn()
{
    if (spawnTemplate_.shouldDespawn()) {
        sne::base::Future::Ref despawnTask = despawnTask_.lock();
        if (despawnTask.get() != nullptr) {
            despawnTask->cancel();
            despawnTask_.reset();
        }

        despawnTask_ = SPAWN_SERVICE->scheduleDespawn(*this, spawnTemplate_.getDespawnDelay());
    }
}


ObjectPosition Entity::getNextSpawnPosition() const
{
    {
        std::unique_lock<LockType> lockPosition(lockPositionable_);

        if ((currentWorldMap_ != nullptr) && (currentMapRegion_ != nullptr)) {
            if (currentMapRegion_ != &currentWorldMap_->getGlobalMapRegion()) {
                return currentMapRegion_->getRandomPositionBy(*this, regionSequence_);
            }
        }
    }
    return getHomePosition();
}


std::unique_ptr<EffectScriptApplier> Entity::createEffectScriptApplier()
{
    return std::make_unique<EntityEffectScriptApplier>(*this);
}


std::unique_ptr<EffectHelper> Entity::createEffectHelper()
{
    return std::make_unique<EntityEffectHelper>(*this);
}


const UnionEntityInfo& Entity::getUnionEntityInfo() const
{
    static const UnionEntityInfo nullUninonEntityInfo;

    std::unique_lock<LockType> lock(lockThis_);

    return getUnionEntityInfo_i();
}


void Entity::setMapRegion(MapRegion& mapRegion, int regionSequence)
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    currentMapRegion_ = &mapRegion;
    regionSequence_ = regionSequence;
}


const MapRegion& Entity::getMapRegion() const
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    assert(currentMapRegion_ != nullptr);
    return *currentMapRegion_;
}


MapRegion& Entity::getMapRegion()
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    assert(currentMapRegion_ != nullptr);
    return *currentMapRegion_;
}


void Entity::setHomePosition(const ObjectPosition& position)
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    homePosition_ = position;
}


ObjectPosition Entity::getHomePosition() const
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    return homePosition_;
}


bool Entity::isEffectInfoable() const
{
    return controller_->queryEffectCallback() != nullptr;
}


float32_t Entity::getSquaredLength(const Position& position) const
{
    const Vector2 source = asVector2(getPosition());
    const Vector2& destin = asVector2(position);
    return squaredLength(destin - source);
}


float32_t Entity::getAngle(float32_t x, float32_t y) const
{
    return getYaw(Position(x, y, 0.0f), getPosition());
}


bool Entity::hasPath() const
{
    if (entityPath_ != nullptr) {
        assert(entityPath_->path_node().size() > 1);
        return true;
    }
    return false;
}


int Entity::getNearestPathNodeIndex() const
{
    if (! entityPath_) {
        return -1;
    }

    const Position currentPosition = getPosition();
    int index = 0;
    float32_t minDistanceSq = (std::numeric_limits<float32_t>::max)();
    const gdt::entity_path_t::path_node_sequence& pathNodes = entityPath_->path_node();
    for (size_t i = 0; i < pathNodes.size(); ++i) {
        const gdt::path_node_t& pathNode = pathNodes[i];
        const Position position(pathNode.x(), pathNode.y(), pathNode.z());
        const float32_t distanceSq =
            squaredLength(asVector2(position) - asVector2(currentPosition));
        if (distanceSq < minDistanceSq) {
            minDistanceSq = distanceSq;
            index = int(i);
        }
    }
    return index;
}


int Entity::getLastPathIndex() const
{
    if (! entityPath_) {
        return -1;
    }

    return int(entityPath_->path_node().size()) - 1;
}


void Entity::playAction(uint32_t actionCode)
{
    assert(queryKnowable() != nullptr);

    auto event = std::make_shared<PlayActionEvent>(getGameObjectInfo(), actionCode);
    queryKnowable()->broadcast(event);
}


void Entity::registerObserver(const GameObjectInfo& entityInfo)
{
    std::unique_lock<LockType> lock(lockThis_);

    observerEntitySet_.insert(entityInfo);
}


void Entity::unregisterObserver(const GameObjectInfo& entityInfo)
{
    std::unique_lock<LockType> lock(lockThis_);

    observerEntitySet_.erase(entityInfo);
}


void Entity::notifyToOberversOfObservers(go::EntityEvent::Ref event, bool exceptSelf)
{
    if (! exceptSelf) {
        scheduleEventCallTask(*this, event);
    }

    WorldMap* currentWorldMap = nullptr;
    EntitySet fillEntitySet;
    EntitySet notifyEntitySet;
    {
        std::unique_lock<LockType> lock(lockThis_);

        currentWorldMap = currentWorldMap_;
        if (! currentWorldMap) {
            return;
        }

        fillEntitySet = observerEntitySet_;
        notifyEntitySet = observerEntitySet_;
    }
    GameObjectInfo exceptTarget = exceptSelf ? getGameObjectInfo() : GameObjectInfo();
    for (const GameObjectInfo& observerInfo : fillEntitySet) {
        go::Entity* observer = currentWorldMap->getEntity(observerInfo);
        if (observer != nullptr) {
            observer->fillObserverSet(notifyEntitySet, exceptTarget);
        }
    }

    for (const GameObjectInfo& observerInfo : notifyEntitySet) {
        go::Entity* observer = currentWorldMap->getEntity(observerInfo);
        if (observer != nullptr) {
            scheduleEventCallTask(*observer, event);
        }
    }
}


void Entity::notifyToObervers(go::EntityEvent::Ref event, bool exceptSelf)
{
    if (! exceptSelf) {
        scheduleEventCallTask(*this, event);
    }

    WorldMap* currentWorldMap = nullptr;
    EntitySet observerEntitySet;

    {
        std::unique_lock<LockType> lock(lockThis_);

        currentWorldMap = currentWorldMap_;
        if (! currentWorldMap) {
            return;
        }
        observerEntitySet = observerEntitySet_;
    }

    for (const GameObjectInfo& observerInfo : observerEntitySet) {
        go::Entity* observer = currentWorldMap->getEntity(observerInfo);
        if (observer != nullptr) {
            scheduleEventCallTask(*observer, event);
        }
    }
}


void Entity::fillObserverSet(EntitySet& entitySet, const GameObjectInfo& exceptTarget) const
{
    EntitySet observerEntitySet;
    WorldMap* currentWorldMap = nullptr;

    {
        std::unique_lock<LockType> lock(lockThis_);

        currentWorldMap = currentWorldMap_;
        if (! currentWorldMap) {
            return;
        }
        observerEntitySet = observerEntitySet_;
    }
    
    for (const GameObjectInfo& observerInfo : observerEntitySet) {
        if (exceptTarget == observerInfo) {
            continue;;
        }
        if (currentWorldMap->isEntered(observerInfo)) {
            entitySet.insert(observerInfo);
        }
    }
}


void Entity::despawned()
{
    GameObjectInfo summoner;
    EntitySet summons;

    {
        std::unique_lock<LockType> lock(lockThis_);

        summoner = summoner_;
        summons = summons_;
        summoner_.reset();
        summons_.clear();
    }

    resetObservers();

    Knowable* knowable = queryKnowable();
    if (knowable != nullptr) {
        if (summoner.isValid()) {
            Entity* entity = knowable->getEntity(summoner);
            if (entity != nullptr) {
                entity->removeSummon(getGameObjectInfo());
            }
        }

        for (const GameObjectInfo summonInfo : summons) {
            Entity* entity = knowable->getEntity(summonInfo);
            if (entity != nullptr) {
                entity->unsetSummoner();
            }
        }
    }
}


void Entity::resetObservers()
{
    WorldMap* currentWorldMap = nullptr;
    EntitySet observerEntitySet;
    {
        std::unique_lock<LockType> lock(lockThis_);

        observerEntitySet = observerEntitySet_;
        observerEntitySet_.clear();

        currentWorldMap = currentWorldMap_;
    }

    if (currentWorldMap != nullptr) {
        const GameObjectInfo& goi = getGameObjectInfo();
        for (const GameObjectInfo& observerInfo : observerEntitySet) {
            go::Entity* observer = currentWorldMap->getEntity(observerInfo);
            if (observer != nullptr) {
                TargetSelectable* targetSelectable = observer->queryTargetSelectable();
                if (targetSelectable) {
                    targetSelectable->unselectTarget();
                }
                else {
                    observer->unregisterObserver(goi);
                }
            }
        }
    }
}

// = Positionable overriding

void Entity::setWorldMap(WorldMap& worldMap)
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    currentWorldMap_ = &worldMap;
}


WorldMap* Entity::getCurrentWorldMap()
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    return currentWorldMap_;
}


const WorldMap* Entity::getCurrentWorldMap() const
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    return currentWorldMap_;
}


bool Entity::isEnteredAtWorld() const
{
    WorldMap* worldMap = nullptr;
    {
        std::unique_lock<LockType> lockPosition(lockPositionable_);

        if (! isValid()) {
            return false;
        }

        worldMap = currentWorldMap_;
    }

    if (! worldMap) {
        return false;
    }

    return worldMap->isEntered(getGameObjectInfo());
}


void Entity::setSectorAddress(const SectorAddress& address)
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    sectorAddress_ = address;
}


SectorAddress Entity::getSectorAddress() const
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    return sectorAddress_;
}


void Entity::setGlobalSectorAddress(const SectorAddress& address)
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    globalSectorAddress_ = address;
}


SectorAddress Entity::getGlobalSectorAddress() const
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    return globalSectorAddress_;
}


void Entity::setPosition(const ObjectPosition& position)
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    if (! isValid()) {
        return;
    }

    //const Position oldPos = getUnionEntityInfo_i().asEntityInfo().position_;
    //SNE_LOG_DEBUG8("setPosition(E%" PRIu64 "): (%.2f,%.2f,%.2f)->(%.2f,%.2f,%.2f)",
    //    getObjectId(), oldPos.x_, oldPos.y_, oldPos.z_, position.x_, position.y_, position.z_);

    getUnionEntityInfo_i().asEntityInfo().position_ = position;
}


void Entity::setHeading(Heading heading)
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    if (! isValid()) {
        return;
    }

    getUnionEntityInfo_i().asEntityInfo().position_.heading_ = heading;
}


ObjectPosition Entity::getPosition() const
{
    std::unique_lock<LockType> lockPosition(lockPositionable_);

    if (! isValid()) {
        return ObjectPosition();
    }
    return getUnionEntityInfo_i().asEntityInfo().position_;
}

// = Summonable overriding

void Entity::setSummoner(go::Entity& summoner, SpawnType spawnType)
{
    std::unique_lock<LockType> lock(lockThis_);

    summoner_ = summoner.getGameObjectInfo();
    if (shouldInheritFaction(spawnType)) {
        go::Factionable* summonerFactionable = summoner.queryFactionable();
        if (summonerFactionable != nullptr) {
            go::Factionable* summonFactionable = queryFactionable();
            if (summonFactionable != nullptr) {
                summonFactionable->setFaction(summonerFactionable->getFactionCode());
            }
        }
    }
}


void Entity::unsetSummoner()
{
    std::unique_lock<LockType> lock(lockThis_);

    summoner_.reset();
}


void Entity::addSummon(const GameObjectInfo& summon)
{
    std::unique_lock<LockType> lock(lockThis_);

    summons_.insert(summon);
}


void Entity::removeSummon(const GameObjectInfo& summon)
{
    std::unique_lock<LockType> lock(lockThis_);

    summons_.erase(summon);
}


Entity::EntitySet Entity::getSummons() const
{
    std::lock_guard<LockType> lock(lockThis_);

    return summons_;
}


GameObjectInfo Entity::getSummoner() const
{
    std::unique_lock<LockType> lock(lockThis_);

    return summoner_;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
