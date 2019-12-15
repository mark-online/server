#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class AnchorCallback
 */
class AnchorCallback
{
public:
    virtual ~AnchorCallback() {}

public:
    virtual void startBuilt(ObjectId ownerId, sec_t startTime) = 0;
    virtual void broken() = 0;

    virtual void fired(uint8_t index, const GameObjectInfo& targetInfo) = 0;
    virtual void fired(uint8_t index, const Position& position) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
