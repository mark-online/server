#pragma once

#include "ZoneServer/controller/player/PlayerTeleportController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerTeleportController
 *
 * 테스트 용 mock MockPlayerTeleportController
 */
class MockPlayerTeleportController:
    public zoneserver::gc::PlayerTeleportController,
    public sne::test::CallCounter
{
public:
    MockPlayerTeleportController(zoneserver::go::Entity* owner);

    // = rpc::TeleportEvent overriding
	OVERRIDE_SRPC_METHOD_5(onTeleportTo,
		ErrorCode, errorCode, MapCode, worldMapCode, ObjectPosition, spawnPosition,
		MigrationTicket, migrationTicket, GameMoney, currentGameMoney);

	OVERRIDE_SRPC_METHOD_2(onAddBindRecall,
		ErrorCode, errorCode, BindRecallInfo, bindRecallInfo);
	OVERRIDE_SRPC_METHOD_2(onRemoveBindRecall,
		ErrorCode, errorCode, ObjectId, linkId);

	OVERRIDE_SRPC_METHOD_2(evBindRecallTicket,
		ErrorCode, errorCode, MigrationTicket, migrationTicket);
public:
    ErrorCode lastErrorCode_;
};
