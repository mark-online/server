#pragma once

#include "StaticObject.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/DungeonInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Dungeon
 * 던전 (입구)
 */
class ZoneServer_Export Dungeon : public StaticObject
{
    typedef StaticObject Parent;

public:
    Dungeon(std::unique_ptr<gc::EntityController> controller);
	virtual ~Dungeon();

public:
    bool initialize(ObjectId dungeonId, MapCode dungeonMapCode);

protected:
    virtual void finalize();

public:
    /// entity 입장
    virtual ErrorCode enter(Entity& entity) = 0;

public:
    virtual std::string getMapData() const = 0;

    virtual WorldMap* getWorldMap() = 0;

protected:
    virtual DataCode getEntityCode() const {
        return dungeonMapCode_;
    }

protected:
    ErrorCode checkEnterable(Entity& entity) const;

private:
    // = Positionable overriding
    virtual void setPosition(const ObjectPosition& position);
    virtual void setHeading(Heading heading);

    virtual WorldPosition getWorldPosition() const;

private:
    MapCode dungeonMapCode_;
    DungeonInfo dungeonInfo_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
