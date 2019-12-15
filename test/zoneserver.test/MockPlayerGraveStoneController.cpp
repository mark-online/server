#include "ZoneServerTestPCH.h"
#include "MockPlayerGraveStoneController.h"

using namespace gideon::zoneserver;

MockPlayerGraveStoneController::MockPlayerGraveStoneController(zoneserver::go::Entity* owner) :
    PlayerGraveStoneController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}
