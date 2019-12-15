#pragma once

#include "ZoneServer/controller/player/PlayerSkillController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerSkillController
 *
 * 테스트 용 mock MockPlayerSkillController
 */
class MockPlayerSkillController :
    public zoneserver::gc::PlayerSkillController,
    public sne::test::CallCounter
{
public:
    MockPlayerSkillController(zoneserver::go::Entity* owner);

    // = rpc::SkillRpc overriding
    OVERRIDE_SRPC_METHOD_2(evConcentrationSkillCancelled,
        GameObjectInfo, entityInfo, SkillCode, skillCode);
	OVERRIDE_SRPC_METHOD_2(evConcentrationSkillCompleted,
		GameObjectInfo, entityInfo, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_2(evSkillLearned,
        SkillCode, removeSkillCode, SkillCode, addSkillCode);
    OVERRIDE_SRPC_METHOD_2(evPassiveSkillActivated,
        GameObjectInfo, entityInfo, SkillCode, skillCode);
public:
    ErrorCode lastErrorCode_;
};
