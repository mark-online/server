#pragma once

#include "SectorDirection.h"
#include <sne/base/utility/Assert.h>
#include <functional>
#include <limits>

namespace gideon { namespace zoneserver {

/**
 * @class SectorAddress
 */
class SectorAddress
{
public:
    typedef int16_t Value;
    typedef int32_t SectorId;

    static const Value invalidPos = _I16_MAX;
    static const SectorId invalidSectorId = _I32_MAX;

public:
    explicit SectorAddress(Value column = invalidPos, Value row = invalidPos) :
        sectorId_((column << (sizeof(Value) * CHAR_BIT)) + row),
        column_(column),
        row_(row) {}

    void reset() {
        column_ = row_ = invalidPos;
        sectorId_ = invalidSectorId;
    }

    bool isValid() const {
        return sectorId_ != invalidSectorId;
    }

    SectorAddress getNeighbor(SectorDirection direction) const {
        switch (direction) {
        case dirEast:
            return SectorAddress(column_ + 1, row_);
        case dirNorthEast:
            return SectorAddress(column_ + 1, row_ + 1);
        case dirNorth:
            return SectorAddress(column_, row_ + 1);
        case dirNorthWest:
            return SectorAddress(column_ - 1, row_ + 1);
        case dirWest:
            return SectorAddress(column_ - 1, row_);
        case dirSouthWest:
            return SectorAddress(column_ - 1, row_ - 1);
        case dirSouth:
            return SectorAddress(column_, row_ - 1);
        case dirSouthEast:
            return SectorAddress(column_ + 1, row_ - 1);
        }
        SNE_ASSERT(false && "Invalid direction.");
        return SectorAddress();
    }

    bool isNeighbor(const SectorAddress& address) const {
        if (address == *this) {
            return true;
        }
        for (int i = 0; i < dirCount; ++i) {
            const SectorDirection direction = static_cast<SectorDirection>(i);
            if (getNeighbor(direction) == address) {
                return true;
            }
        }
        return false;
    }

    void operator=(const SectorAddress& rhs) {
        this->column_ = rhs.column_;
        this->row_ = rhs.row_;
        this->sectorId_ = rhs.sectorId_;
    }

    bool operator<(const SectorAddress& rhs) const {
        return sectorId_ < rhs.sectorId_;
    }

    bool operator==(const SectorAddress& rhs) const {
        return sectorId_ == rhs.sectorId_;
    }

    bool operator!=(const SectorAddress& rhs) const {
        return sectorId_ != rhs.sectorId_;
    }

    Value getColumn() const {
        return column_;
    }

    Value getRow() const {
        return row_;
    }

    SectorId getSectorId() const {
        return sectorId_;
    }

private:
    SectorId sectorId_;
    Value column_;
    Value row_;
};


inline std::size_t hash_value(const SectorAddress& val)
{
    return val.getSectorId();
}

}} // namespace gideon { namespace zoneserver {
