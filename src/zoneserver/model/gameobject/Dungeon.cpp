#include "ZoneServerPCH.h"
#include "Dungeon.h"
#include "status/StaticObjectStatus.h"
#include "../../controller/EntityController.h"
#include "../../world/WorldMap.h"
#include "../../service/distance/DistanceChecker.h"
#include <gideon/3d/3d.h>

namespace gideon { namespace zoneserver { namespace go {

Dungeon::Dungeon(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller)),
    dungeonMapCode_(invalidMapCode)
{
}


Dungeon::~Dungeon()
{
}


bool Dungeon::initialize(ObjectId dungeonId, MapCode dungeonMapCode)
{
    assert(isValidObjectId(dungeonId));

    std::lock_guard<LockType> lock(getLock());

    if (! Parent::initialize(otDungeon, dungeonId)) {
        return false;
    }

    dungeonMapCode_ = dungeonMapCode;
    dungeonInfo_.objectType_ = getObjectType();
    dungeonInfo_.objectId_ = dungeonId;
    //dungeonInfo_.position_ = spawnPosition;
    dungeonInfo_.mapCode_ = dungeonMapCode_;
    assert(dungeonInfo_.isValid());

    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    entityInfo.objectType_ = getObjectType();
    MoreDungeonInfo& moreDungeonInfo = entityInfo.asDungeonInfo();
    static_cast<DungeonInfo&>(moreDungeonInfo) = dungeonInfo_;
    return true;
}


void Dungeon::finalize()
{
    Parent::finalize();

    dungeonMapCode_ = invalidMapCode;
}


ErrorCode Dungeon::checkEnterable(Entity& entity) const
{
    if (! DISTANCE_CHECKER->checkDungeonDistance(entity.getPosition(), getPosition())) {
        return ecDungeonNoPermission;
    }

    if (! isEnteredAtWorld()) {
        return ecDungeonNotFound;
    }

    WorldMap* currentWorldMap = entity.getCurrentWorldMap();
    if (currentWorldMap != nullptr) {
        if (currentWorldMap->isDungeon()) {
            return ecWorldMapAlreadyEntered;
        }
    }

    return ecOk;
}

// = Positionable overriding

void Dungeon::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);

    dungeonInfo_.position_ = position;
}


void Dungeon::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);

    dungeonInfo_.position_.heading_ = heading;
}


WorldPosition Dungeon::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(dungeonInfo_.position_, worldMap->getMapCode());
}

}}} // namespace gideon { namespace zoneserver { namespace go {
