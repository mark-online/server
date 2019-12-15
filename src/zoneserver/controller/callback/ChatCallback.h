#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/Chatting.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class ChatCallback
 */
class ChatCallback
{
public:
    virtual ~ChatCallback() {}

public:
    virtual void creatureSaid(const Nickname& nickname,
        const ChatMessage& message) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
