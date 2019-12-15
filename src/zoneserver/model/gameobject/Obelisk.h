#pragma once

#include "StaticObject.h"
#include <gideon/cs/shared/data/ObeliskInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Obelisk
 * 오벨리스크
 */
class ZoneServer_Export Obelisk : public StaticObject
{
    typedef StaticObject Parent;

public:
    Obelisk(std::unique_ptr<gc::EntityController> controller);
    virtual ~Obelisk();

    bool initialize(ObjectId obeliskId);

    // = StaticObject overriding    
private:
    virtual void finalize();

    virtual ErrorCode respawn(WorldMap& worldMap);

    virtual DataCode getEntityCode() const {
        return uniqueObeliskCode;
    };

    virtual ObjectPosition getNextSpawnPosition() const;

private:
	// = Positionable overriding
	virtual void setPosition(const ObjectPosition& position);
	virtual void setHeading(Heading heading);

	virtual WorldPosition getWorldPosition() const;

private:
    void reset();

private:
	ObeliskInfo obeliskInfo_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
