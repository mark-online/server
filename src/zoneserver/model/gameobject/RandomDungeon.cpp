#include "ZoneServerPCH.h"
#include "RandomDungeon.h"
#include "../../controller/DungeonController.h"
#include "../../service/spawn/SpawnService.h"
#include "../../world/region/MapRegion.h"
#include "../../world/WorldMap.h"
#include "../../world/World.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "esut/Random.h"
#include <gideon/cs/randun/RandomDungeonConstructor.h>
#include <gideon/cs/datatable/template/random_dungeon_table.hxx>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/datatable/RegionSpawnTable.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <sne/server/utility/Profiler.h>

namespace gideon { namespace zoneserver { namespace go {

RandomDungeon::RandomDungeon(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller)),
    dungeonTemplate_(nullptr),
    worldMapHelper_(nullptr)
{
}


RandomDungeon::~RandomDungeon()
{
}


bool RandomDungeon::initialize(ObjectId dungeonId, MapCode dungeonMapCode,
    const gdt::random_dungeon_t& dungeonTemplate,
    WorldMapHelper& worldMapHelper)
{
    assert(isValidObjectId(dungeonId));

    {
        std::lock_guard<LockType> lock(getLock());

        if (! Parent::initialize(dungeonId, dungeonMapCode)) {
            return false;
        }

        dungeonTemplate_ = &dungeonTemplate;
        worldMapHelper_ = &worldMapHelper;

        randunConstructor_ = std::make_unique<randun::RandomDungeonConstructor>(
            dungeonTemplate_->column_count(), dungeonTemplate_->row_count(), dungeonTemplate_->base_z());

        // TODO: WorldMapHelper가 2종류가 있는데 왜?
        WorldMapHelper& thisWorldMapHelper = *this;
        worldMap_ = std::make_unique<WorldMap>(thisWorldMapHelper, worldMapHelper_->generateMapId());
    }
    return true;
}


void RandomDungeon::finalize()
{
    if (worldMap_.get() != nullptr) {
        WORLD->removeWorldMap(*worldMap_);
        worldMap_.reset();
    }

    Parent::finalize();
}



ErrorCode RandomDungeon::respawn(WorldMap& worldMap)
{
    createPublicParty();

    if (worldMap_.get() != nullptr) {
        worldMap_->finalizeForReinitialize();
    }
    
    return Parent::respawn(worldMap);
}


ErrorCode RandomDungeon::spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition)
{
    ErrorCode errorCode = Entity::spawn(worldMap, spawnPosition);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    // TODO: 처리해야함....
    createPublicParty();
    return errorCode;
}


ErrorCode RandomDungeon::despawn()
{
    if (! isEnteredAtWorld()) {
        return ecWorldMapNotEntered;
    }

    {
        std::lock_guard<LockType> lock(getLock());

        if (! worldMap_.get()) {
            //assert(false);
            return ecWorldMapNotEntered;
        }
    }

    if (worldMap_->getEntityCount(otPc) <= 0) {
        WORLD->removeWorldMap(*worldMap_);
        const ErrorCode errorCode = Parent::despawn();
        if (isSucceeded(errorCode)) {
            destoryParty();
            return ecOk;
        }

        SNE_LOG_ERROR("Failed to despawn RandomDungeon(%" PRIu64 ")[E%d]",
            getObjectId(), errorCode);
    }

    // 30초 후 재시도
    const GameTime delay = 30 * 1000;
    (void)SPAWN_SERVICE->scheduleDespawn(*this, delay);
    return ecOk;
}


ErrorCode RandomDungeon::enter(Entity& entity)
{
    std::lock_guard<LockType> lock(getLock());

    const ErrorCode errorCode = checkEnterable(entity);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    if (! worldMap_.get()) {
        assert(false);
        return ecServerInternalError;
    }

    if (worldMap_->isInitialized()) {
        return ecOk;
    }

    if (! initWorldMap()) {
        return ecWorldMapInvalidDungeon;
    }

    return ecOk;
}


bool RandomDungeon::initWorldMap()
{
    assert(NPC_TABLE != nullptr);

    if (! generateRandomDungeonMap()) {
        return false;
    }

    const MapInfo mapInfo =
        randunConstructor_->getMapInfo(getEntityCode(), dungeonTemplate_->sector_length());
    if (! mapInfo.isValid()) {
        SNE_LOG_ERROR("Failed to get MapInfo of RandunConstructor(%u).",
            uint32_t(dungeonTemplate_->map_code()));
        return false;
    }

    if (! worldMap_->initialize(mapInfo)) {
        worldMap_.reset();
        SNE_LOG_ERROR("Failed to initialize RandomDungeon WorldMap(%u).",
            uint32_t(dungeonTemplate_->map_code()));
        return false;
    }

    WORLD->addWorldMap(*worldMap_);
    return true;
}


bool RandomDungeon::generateRandomDungeonMap()
{
    std::lock_guard<LockType> lock(getLock());

    const datatable::RegionSpawnTable* spawnTable =
        worldMapHelper_->getRegionSpawnTable(dungeonTemplate_->map_code());
    if (! spawnTable) {
        SNE_LOG_ERROR("Failed to get RegionSpawnTable(%u).",
            uint32_t(dungeonTemplate_->map_code()));
        return false;
    }

    if (dungeonTemplate_->column_count() <= 0) {
        return false;
    }
    if (dungeonTemplate_->row_count() <= 0) {
        return false;
    }

    {
        sne::server::Profiler profiler("randun::RandomDungeonConstructor::generate()");

        randunConstructor_->generate(dungeonTemplate_->dck_length(), dungeonTemplate_->dck_steps());
    }

    {
        sne::server::Profiler profiler("randun::RandomDungeonConstructor::analyze()");

        if (! randunConstructor_->analyze(spawnTable, NPC_TABLE)) {
            SNE_LOG_ERROR("Failed to analyze RandunConstructor(%u).",
                uint32_t(dungeonTemplate_->map_code()));
            return false;
        }
    }

    mapData_ = randunConstructor_->exportDe();

    return true;
}

// = WorldMapHelper overriding

const datatable::RegionTable* RandomDungeon::getRegionTable(MapCode mapCode) const
{
    assert(mapCode == getEntityCode()); mapCode;
    return randunConstructor_->getRegionTable();
}


const datatable::RegionCoordinates* RandomDungeon::getRegionCoordinates(MapCode mapCode) const
{
    assert(mapCode == getEntityCode()); mapCode;
    return randunConstructor_->getRegionCoordinates();
}


const datatable::RegionSpawnTable* RandomDungeon::getRegionSpawnTable(MapCode mapCode) const
{
    assert(mapCode == getEntityCode()); mapCode;
    return randunConstructor_->getSpawnTable();
}


void RandomDungeon::createPublicParty()
{
    ZONE_SERVICE->getCommunityServerProxy().z2m_createRandumDungeonPublicParty(getGameObjectInfo());
}


void RandomDungeon::setPartyId(PartyId partyId)
{
    partyId_ = partyId;
}


void RandomDungeon::destoryParty()
{
    if (isValidPartyId(partyId_)) {
        ZONE_SERVICE->getCommunityServerProxy().z2m_destoryRandumDungeonPublicParty(partyId_);
        partyId_ = invalidPartyId;
    }
}


PartyId RandomDungeon::getPartyId() const
{
    return partyId_;
}


}}} // namespace gideon { namespace zoneserver { namespace go {
