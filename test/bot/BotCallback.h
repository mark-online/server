#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/ExpelReason.h>

namespace gideon { namespace bot {

/**
 * @class BotAccountCallback
 */
class BotAccountCallback
{
public:
    virtual ~BotAccountCallback() {}

    virtual void expelledFromServer(ExpelReason reason) = 0;
};

}} // namespace gideon { namespace bot {