#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class GliderCallback
 */
class GliderCallback
{
public:
    virtual ~GliderCallback() {}

public:
	virtual void mountGlider() = 0;
    virtual void gliderDismounted(const GameObjectInfo& creatureInfo, float32_t speed) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
