#include "ZoneServerPCH.h"
#include "EntityEvent.h"
#include "Entity.h"

namespace gideon { namespace zoneserver { namespace go {

void EventCallTask::run()
{
    if (entity_.isValid()) {
        event_->call(entity_);
    }
}

}}} // namespace gideon { namespace zoneserver { namespace go {
