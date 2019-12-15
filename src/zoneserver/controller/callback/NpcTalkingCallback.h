#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/NpcTalkingInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class NpcTalkingCallback
 */
class NpcTalkingCallback
{
public:
    virtual ~NpcTalkingCallback() {}

    virtual void talkedFrom(const GameObjectInfo& talker, NpcTalkingCode talkingCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
