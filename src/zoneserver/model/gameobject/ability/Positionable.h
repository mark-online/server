#pragma once

#include "../../../world/SectorAddress.h"
#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver {
class WorldMap;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Positionable
 * 맵 상에 위치할 수 있다
 */
class Positionable
{
public:
    virtual ~Positionable() {}

public:
    /// for test only
    virtual void setWorldMap(WorldMap& worldMap) = 0;

    virtual WorldMap* getCurrentWorldMap() = 0;
    virtual const WorldMap* getCurrentWorldMap() const = 0;

    virtual bool isEnteredAtWorld() const = 0;

public:
    virtual void setSectorAddress(const SectorAddress& address) = 0;

    virtual SectorAddress getSectorAddress() const = 0;

	virtual void setGlobalSectorAddress(const SectorAddress& address) = 0;

	virtual SectorAddress getGlobalSectorAddress() const = 0;

public:
    virtual void setPosition(const ObjectPosition& position) = 0;
    virtual void setHeading(Heading heading) = 0;

    virtual ObjectPosition getPosition() const = 0;

    /// 월드에서의 위치(던전은 해당 없음)
    virtual WorldPosition getWorldPosition() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
