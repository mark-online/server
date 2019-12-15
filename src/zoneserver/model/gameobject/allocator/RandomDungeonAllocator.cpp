#include "ZoneServerPCH.h"
#include "RandomDungeonAllocator.h"
#include "../RandomDungeon.h"
#include "../../../controller/RandomDungeonController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* RandomDungeonAllocator::malloc()
{
    return new go::RandomDungeon(
        std::make_unique<gc::RandomDungeonController>());
}


void RandomDungeonAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::RandomDungeon*>(entity));
}


//std::unique_ptr<go::Dungeon> RandomDungeonAllocator::create(const RegionSpawnTemplate& spawnTemplate,
//    const gdt::random_dungeon_t& dungeonTemplate,
//    MapRegion& mapRegion, WorldMapHelper& worldMapHelper)
//{
//    const MapCode mapCode = dungeonTemplate.map_code();
//    const MapType mapType = getMapType(mapCode);
//
//    std::unique_ptr<gc::EntityController> controller(createController(mapType));
//    if (! controller.get()) {
//        return nullptr;
//    }
//
//    switch (mapType) {
//    case mtRandomDundeon:
//        return std::make_unique<go::RandomDungeon>(controller, spawnTemplate, dungeonTemplate,
//            mapRegion, worldMapHelper));
//    }
//
//    assert(false && "invalid Dungeon MapType");
//    return nullptr;
//}
//
//
//std::unique_ptr<gc::EntityController> RandomDungeonAllocator::createController(MapType mapType)
//{
//    switch (mapType) {
//    case mtRandomDundeon:
//        return std::make_unique<gc::RandomDungeonController>();
//    }
//
//    assert(false && "invalid Dungeon MapType");
//    return nullptr;
//}

}}} // namespace gideon { namespace zoneserver { namespace go {
