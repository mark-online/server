#pragma once

#include "Dungeon.h"
#include "../../world/WorldMapHelper.h"
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gdt {
class random_dungeon_t;
} // namespace gdt {

namespace gideon { namespace randun {
class RandomDungeonConstructor;
}} // namespace gideon { namespace randun {

namespace gideon { namespace zoneserver {
class WorldMap;
class WorldMapHelper;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class RandomDungeon
 * 던전 (입구)
 */
class ZoneServer_Export RandomDungeon :
    public Dungeon,
    private WorldMapHelper,
    public sne::core::ThreadSafeMemoryPoolMixin<RandomDungeon>
{
    typedef Dungeon Parent;

public:
    RandomDungeon(std::unique_ptr<gc::EntityController> controller);
	virtual ~RandomDungeon();

public:
    bool initialize(ObjectId dungeonId, MapCode dungeonMapCode,
        const gdt::random_dungeon_t& dungeonTemplate,
        WorldMapHelper& worldMapHelper);

    void setPartyId(PartyId partyId);

    PartyId getPartyId() const;

private:
    virtual void finalize();

private:
    // = Entity overriding

    ErrorCode respawn(WorldMap& worldMap);
    virtual ErrorCode spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition);
    virtual ErrorCode despawn();

    virtual WorldMap* getWorldMap() {
        return worldMap_.get();
    }

private:
    // = Dungeon overriding
    virtual ErrorCode enter(Entity& entity);

    virtual std::string getMapData() const {
        return mapData_;
    }

private:
	bool initWorldMap();
    bool generateRandomDungeonMap();

private:
    // = WorldMapHelper overriding
    virtual ObjectId generateMapId() const {
        assert(false);
        return invalidObjectId;
    }
    virtual const datatable::RegionTable* getRegionTable(MapCode mapCode) const;
    virtual const datatable::RegionCoordinates* getRegionCoordinates(MapCode mapCode) const;
    virtual const datatable::RegionSpawnTable* getRegionSpawnTable(MapCode mapCode) const;
    virtual const datatable::PositionSpawnTable* getPositionSpawnTable(MapCode /*mapCode*/) const { return nullptr; }
    virtual const datatable::EntityPathTable* getEntityPathTable(MapCode /*mapCode*/) const {
        return nullptr;
    }
private:
    void createPublicParty();
    void destoryParty();
    
private:
    const gdt::random_dungeon_t* dungeonTemplate_;
    MapRegion* mapRegion_;
    WorldMapHelper* worldMapHelper_;

    std::unique_ptr<randun::RandomDungeonConstructor> randunConstructor_;
    std::unique_ptr<WorldMap> worldMap_;

    std::string mapData_;
    PartyId partyId_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
