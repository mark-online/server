#pragma once

#include <gideon/cs/shared/data/Coordinate.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class AnchorAbillityCallback
 */
class AnchorAbillityCallback
{
public:
    virtual ~AnchorAbillityCallback() {}

public:
    virtual void reconnaissanced(const Positions& positins) = 0;
    virtual void anchorTargetFired(const GameObjectInfo& anchorInfo, uint8_t index, const GameObjectInfo& target) = 0;
    virtual void anchorPositionFired(const GameObjectInfo& anchorInfo, uint8_t index, const Position& position) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
