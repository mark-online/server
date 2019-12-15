#pragma once

namespace gideon { namespace zoneserver {

/**
 * @enum SectorDirection
 */
enum SectorDirection
{
    dirEast = 0,
    dirNorthEast = 1,
    dirNorth = 2,
    dirNorthWest = 3,
    dirWest = 4,
    dirSouthWest = 5,
    dirSouth = 6,
    dirSouthEast = 7,

    dirCount
};


inline bool isValid(SectorDirection direction)
{
    return (dirEast <= direction) && (direction <= dirSouthEast);
}


inline SectorDirection getOpposite(SectorDirection direction)
{
    assert(isValid(direction));
#ifndef NDEBUG
    switch (direction) {
    case dirEast:
        return dirWest;
    case dirNorthEast:
        return dirSouthWest;
    case dirNorth:
        return dirSouth;
    case dirNorthWest:
        return dirSouthEast;
    case dirWest:
        return dirEast;
    case dirSouthWest:
        return dirNorthEast;
    case dirSouth:
        return dirNorth;
    case dirSouthEast:
        return dirNorthWest;
    }
    return direction;
#else
    return static_cast<SectorDirection>((direction + 4) & 7);
#endif
}

}} // namespace gideon { namespace zoneserver {
