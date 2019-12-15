#include "ZoneServerTestPCH.h"
#include "MockPlayerNpcController.h"

using namespace gideon::zoneserver;

MockPlayerNpcController::MockPlayerNpcController(zoneserver::go::Entity* owner) :
    PlayerNpcController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}


DEFINE_SRPC_METHOD_2(MockPlayerNpcController, evDialogOpened,
    GameObjectInfo, npc, GameObjectInfo, requester)
{
    addCallCount("evDialogOpened");
    npc, requester;
}


DEFINE_SRPC_METHOD_2(MockPlayerNpcController, evDialogClosed,
    GameObjectInfo, npc, GameObjectInfo, requester)
{
    addCallCount("evDialogClosed");
    npc, requester;
}
