#pragma once

namespace gideon { namespace zoneserver {

template <typename T>
inline int getPercentValue(T current, T total)
{
    if ((total == 0) || (current == 0)) {
        return 0;
    }
    return static_cast<int>((static_cast<float64_t>(current) / static_cast<float64_t>(total))  * 1000);
}

}} // namespace gideon { namespace zoneserver {
