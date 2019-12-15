#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockRandomDungeonTable.h>

namespace gideon { namespace servertest {

const datatable::RandomDungeonTemplate* MockRandomDungeonTable::getRandomDungeonTemplate(MapCode mapCode) const
{
    const RandomDungeonMap::const_iterator pos = randomDungeonMap_.find(mapCode);
    if (pos != randomDungeonMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockRandomDungeonTable::fillNormalRandomDungeon()
{
    gdt::random_dungeon_t* dungeon = new gdt::random_dungeon_t;
    dungeon->map_code(firstRandomDungeonMapCode);
    dungeon->random_dungeon_type(1);
    dungeon->column_count(24);
    dungeon->row_count(8);
    dungeon->dck_length(10);
    dungeon->dck_steps(400);
    dungeon->sector_length(30);
    randomDungeonMap_.insert(
        RandomDungeonMap::value_type(dungeon->map_code(), 
        datatable::RandomDungeonTemplate(*dungeon)));
}


void MockRandomDungeonTable::destroyRandomDungeons()
{
    for (RandomDungeonMap::value_type& value : randomDungeonMap_) {
        const datatable::RandomDungeonTemplate& dungeon = value.second;
        delete (dungeon.getInfo());
    }
}

}} // namespace gideon { namespace servertest {
