#pragma once

#include <gideon/cs/shared/data/GliderInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class GliderState
 **/
class GliderState
{
public:
    virtual bool canReadyMonutGlider() const = 0;
    virtual bool isMonutGliding() const = 0;

    virtual void mountGlider(GliderCode gliderCode) = 0;
    virtual void dismountGlider() = 0;

	virtual bool isGliderMounting() const = 0;
};


}} // namespace gideon { namespace zoneserver {
