#pragma once

#include <gideon/cs/shared/data/TradeInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Dialogable
 * 대화할 수 있다
 */
class Dialogable
{
public:
    virtual ~Dialogable() {}

public:
    virtual void openDialog(const Entity& requester) = 0;
    virtual void closeDialog(const GameObjectInfo& requester) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {