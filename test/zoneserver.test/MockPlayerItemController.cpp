#include "ZoneServerTestPCH.h"
#include "MockPlayerItemController.h"

using namespace gideon::zoneserver;

MockPlayerItemController::MockPlayerItemController(zoneserver::go::Entity* owner) :
    PlayerItemController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}
