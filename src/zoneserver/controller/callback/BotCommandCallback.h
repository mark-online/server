#pragma once

#include <gideon/cs/shared/data/Coordinate.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class BotCommandCallback
 */
class BotCommandCallback
{
public:
    virtual ~BotCommandCallback() {}

public:
    virtual void commandMoved(const Position& position, bool shouldStopDestination) = 0;
    virtual void commandSkillCasted() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
