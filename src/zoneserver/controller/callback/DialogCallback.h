#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class DialogCallback
 */
class DialogCallback
{
public:
    virtual ~DialogCallback() {}

    virtual void dialogOpened(const GameObjectInfo& npc, const GameObjectInfo& requester) = 0;
    virtual void dialogClosed(const GameObjectInfo& npc, const GameObjectInfo& requester) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
