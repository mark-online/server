#include "ZoneServerPCH.h"
#include "PublicQuestSpawnMapRegion.h"
#include "../../model/gameobject/Npc.h"
#include <gideon/cs/datatable/NpcTable.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

PublicQuestSpawnMapRegion::PublicQuestSpawnMapRegion(WorldMap& worldMap,
    PublicQuest& publicQuest, const datatable::RegionTemplate& regionTemplate,
    const datatable::RegionCoordinates::CoordinatesList& coordinates,
    const datatable::RegionSpawnTable::Spawns& regionSpawns,
    const datatable::PositionSpawnTable::Spawns& positionSpawns,
    WorldMapHelper& worldMapHelper) :
    SpawnMapRegion(worldMap, regionTemplate, coordinates, &regionSpawns, &positionSpawns,
        worldMapHelper),
    publicQuest_(publicQuest),
    totalLevel_(0)
{
    assert(isSpawnable(getRegionType()));
}


PublicQuestSpawnMapRegion::~PublicQuestSpawnMapRegion()
{
}


bool PublicQuestSpawnMapRegion::initialize()
{
    if (! SpawnMapRegion::initialize()) {
        return false;
    }

    ACE_GUARD_RETURN(LockType, lock, getLock(), false);

    totalLevel_ = 0;
    goalInfo_.clear();

    BOOST_FOREACH(const RegionSpawnTemplate& spawnTemplate, getRegionSpawnTemplates()) {
        for (uint16_t i = 0; i < spawnTemplate.pool_; ++i) {
            if (! updateGoalInfo(spawnTemplate.entityCode_)) {
                return false;
            }
        }
    }

    BOOST_FOREACH(const PositionSpawnTemplate& spawnTemplate, getPositionSpawnTemplates()) {
        if (! updateGoalInfo(spawnTemplate.entityCode_)) {
            return false;
        }
    }

    return true;
}


void PublicQuestSpawnMapRegion::finalize()
{
    SpawnMapRegion::finalize();
}


void PublicQuestSpawnMapRegion::despawnNpcs()
{
    ObjectEntityMap allEntities = getObjectEntityMap();
    ObjectEntityMap::iterator pos1 = allEntities.find(otNpc);
    if (pos1 != allEntities.end()) {
        go::EntityMap& npcMap = (*pos1).second;
	    BOOST_FOREACH(const go::EntityMap::value_type& value, npcMap) {
            go::Entity* entity = value.second;
            const ErrorCode ec = entity->despawn();
            if (isFailed(ec)) {
                assert(false);
            }
	    }
    }

    ObjectEntityMap::iterator pos2 = allEntities.find(otMonster);
    if (pos2 != allEntities.end()) {
	    go::EntityMap& monsterMap = (*pos2).second;
        BOOST_FOREACH(const go::EntityMap::value_type& value, monsterMap) {
            go::Entity* entity = value.second;
            const ErrorCode ec = entity->despawn();
            if (isFailed(ec)) {
                assert(false);
            }
	    }
    }
}


bool PublicQuestSpawnMapRegion::updateGoalInfo(NpcCode npcCode)
{
    if (! isValidNpcCode(npcCode)) {
        assert(false);
        return false;
    }

    const datatable::NpcTemplate* npcTemplate = NPC_TABLE->getNpcTemplate(npcCode);
    if (! npcTemplate) {
        assert(false);
        return false;
    }

    ++goalInfo_[npcCode].goalKillCount_;
    totalLevel_ += npcTemplate->getLevel();
    return true;
}

// = SpawnMapRegion overriding

void PublicQuestSpawnMapRegion::entitySpawned(go::Entity& entity)
{
    SpawnMapRegion::entitySpawned(entity);

    if (entity.isNpcOrMonster()) {
        static_cast<go::Npc&>(entity).setPublicQuest(&publicQuest_);
    }
}

}} // namespace gideon { namespace zoneserver {
