#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <sne/common/StringTypes.h>


namespace gideon { namespace zoneserver {

namespace go {
class Cheatable;
}
/**
 * @class CheatService
 *
 */
class CheatService
{
public:
    static void execute(go::Cheatable& cheatable, ObjectId ownerId, 
        const wstring_t& command, const wstring_t& params);
};



}} // namespace gideon { namespace zoneserver {