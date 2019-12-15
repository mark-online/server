#pragma once

#include "AbstractAnchorController.h"

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class AnchorController
 * Controller for Anchor
 */
class ZoneServer_Export AnchorController : public AbstractAnchorController
{
private:
    // = StaticObjectEffectCallback override
//    virtual void damaged(HitPoint currentPoint, bool observerNotify, bool exceptSelf);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {