#include "ZoneServerTestPCH.h"
#include "MockPlayerTeleportController.h"

using namespace gideon::zoneserver;

MockPlayerTeleportController::MockPlayerTeleportController(zoneserver::go::Entity* owner) :
    PlayerTeleportController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}

DEFINE_SRPC_METHOD_5(MockPlayerTeleportController, onTeleportTo,
	ErrorCode, errorCode, MapCode, worldMapCode, ObjectPosition, spawnPosition,
	MigrationTicket, migrationTicket, GameMoney, currentGameMoney)
{
	addCallCount("onTeleportTo");
	lastErrorCode_ = errorCode;
	worldMapCode, spawnPosition, migrationTicket, currentGameMoney;
}

DEFINE_SRPC_METHOD_2(MockPlayerTeleportController, onAddBindRecall,
	ErrorCode, errorCode, BindRecallInfo, bindRecallInfo)
{
	addCallCount("onAddBindRecall");	    
	lastErrorCode_ = errorCode;
	bindRecallInfo;
}


DEFINE_SRPC_METHOD_2(MockPlayerTeleportController, onRemoveBindRecall,
	ErrorCode, errorCode, ObjectId, linkId)
{
	addCallCount("onRemoveBindRecall");
	lastErrorCode_ = errorCode;
	linkId;
}


DEFINE_SRPC_METHOD_2(MockPlayerTeleportController, evBindRecallTicket,
	ErrorCode, errorCode, MigrationTicket, migrationTicket)
{
	addCallCount("evBindRecallTicket");
	lastErrorCode_ = errorCode;
	migrationTicket;
}

