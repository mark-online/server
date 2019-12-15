#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver {

class ArenaServiceCallback
{
public:
    virtual void registDeserter(ObjectId playerId) = 0;
};

}} // namespace gideon { namespace zoneserver {
