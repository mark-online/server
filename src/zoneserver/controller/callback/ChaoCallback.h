#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class ChaoCallback
 */
class ChaoCallback
{
public:
    virtual ~ChaoCallback() {}

public:
    virtual void changeChao(bool isChaoPlayer) = 0;
    virtual void changeTempChao(bool isChaoPlayer) = 0;

    virtual void chaoChanged(ObjectId playerId, bool isChaoPlayer) = 0;
    virtual void tempChaoChanged(ObjectId playerId, bool isChaoPlayer) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
