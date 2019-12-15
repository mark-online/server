#pragma once

#include "StaticObject.h"
#include <gideon/cs/shared/data/HarvestInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/base/concurrent/Future.h>


namespace gdt {
class harvest_t;
} // namespace gdt {

namespace gideon { namespace zoneserver { namespace go {

namespace {

struct UserHarvestInfo
{
    GameTime completeTime_;
    BaseItemInfos items_;
};

} // namespace {

class Entity;

/**
 * @class Harvest
 * 채집
 */
class ZoneServer_Export Harvest : public StaticObject
{
    typedef StaticObject Parent;

public:
    static void setNotUseTimer();

public:
    Harvest(std::unique_ptr<gc::EntityController> controller);
    virtual ~Harvest();

    bool initialize(ObjectId harvestId, HarvestCode harvestCode);

    ErrorCode startHarvest(Entity& entity);
    ErrorCode rewarded(Entity& entity);

public:
    bool shouldDespawn() const;

private:
    void reset();

    void cancelDespawn();
	
    // = StaticObject overriding
private:
    virtual void finalize();

    virtual ErrorCode respawn(WorldMap& worldMap);
    virtual void reserveDespawn();

    virtual DataCode getEntityCode() const;

    virtual HitPoints& getCurrentPoints() {
        static HitPoints points;
        return points;
    }
    virtual const HitPoints& getCurrentPoints() const {
        static const HitPoints points;
        return points;
    };

private:
    // = Positionable overriding
    virtual void setPosition(const ObjectPosition& position);
    virtual void setHeading(Heading heading);

    virtual WorldPosition getWorldPosition() const;
private:
    static bool notUseTimer_;
    const gdt::harvest_t* harvestTemplate_;

    FullHarvestInfo harvestInfo_;
    sne::base::Future::WeakRef despawnTask_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
