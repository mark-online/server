#include "ZoneServerPCH.h"
#include "Obelisk.h"
#include "skilleffect/StaticObjectEffectScriptApplier.h"
#include "status/StaticObjectStatus.h"
#include "../../controller/NpcController.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../world/WorldMap.h"
#include "../../world/region/MapRegion.h"
#include "../../service/time/GameTimer.h"
#include "../../service/spawn/SpawnService.h"

namespace gideon { namespace zoneserver { namespace go {

Obelisk::Obelisk(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller))
{
}


Obelisk::~Obelisk()
{
}


bool Obelisk::initialize(ObjectId obeliskId)
{
    std::lock_guard<LockType> lock(getLock());

    if (! Parent::initialize(otObelisk, obeliskId)) {
        return false;
    }

    obeliskInfo_.objectType_ = otObelisk;
    obeliskInfo_.objectId_ = obeliskId;

    UnionEntityInfo& info = getUnionEntityInfo_i();
    MoreObeliskInfo& moreObeliskInfo = info.asObeliskInfo();
    static_cast<ObeliskInfo&>(moreObeliskInfo) = obeliskInfo_;
    return true;
}


void Obelisk::finalize()
{
    reset();

    Parent::finalize();
}


ErrorCode Obelisk::respawn(WorldMap& worldMap)
{
    reset();

    return Parent::respawn(worldMap);
}


void Obelisk::reset()
{
    //std::lock_guard<LockType> lock(getLock());
    //UnionEntityInfo& info = getUnionEntityInfo_i();
    //MoreObeliskInfo& moreObeliskInfo = info.asObeliskInfo();
}


ObjectPosition Obelisk::getNextSpawnPosition() const
{
    return getPosition();
}


// = Positionable overriding

void Obelisk::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);
    obeliskInfo_.position_ = position;
}


void Obelisk::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);
    obeliskInfo_.position_.heading_ = heading;
}


WorldPosition Obelisk::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(obeliskInfo_.position_, worldMap->getMapCode());
}

}}} // namespace gideon { namespace zoneserver { namespace go {
