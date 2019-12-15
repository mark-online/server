#pragma once

#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/3d/3d.h>

namespace gideon { namespace zoneserver {
    
/**
 * class RotatedRect
 */
class RotatedRegionRect
{
public:
    RotatedRegionRect(const Vector2& minPos, const Vector2& maxPos, Heading heading);

    bool isInRect(const Vector2& pos) const;

    Vector2 getRandomPoint() const;

    Vector2 getClampPos(const Vector2& pos, const Vector2& des) const;

    
    const Vector2& getMaxPos() const{
        return maxPos_;
    }
    const Vector2& getMinPos() const{
        return minPos_;
    }

private:
    void computeRotatedRect(const Vector2& minPos, const Vector2& maxPos, Heading heading);

    Heading heading_;

    Vector2 maxPos_;
    Vector2 minPos_;

    Vector2 center_;
};

}} // namespace gideon { namespace zoneserver {
