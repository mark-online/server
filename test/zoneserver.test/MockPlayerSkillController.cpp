#include "ZoneServerTestPCH.h"
#include "MockPlayerSkillController.h"

using namespace gideon::zoneserver;

MockPlayerSkillController::MockPlayerSkillController(zoneserver::go::Entity* owner) :
    PlayerSkillController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}


// = rpc::SkillRpc overriding

DEFINE_SRPC_METHOD_2(MockPlayerSkillController, evConcentrationSkillCancelled,
    GameObjectInfo, entityInfo, SkillCode, skillCode)
{
    addCallCount("evConcentrationSkillCancelled");

    entityInfo, skillCode;
}


DEFINE_SRPC_METHOD_2(MockPlayerSkillController, evConcentrationSkillCompleted,
	GameObjectInfo, entityInfo, SkillCode, skillCode)
{
	addCallCount("evConcentrationSkillCompleted");

	entityInfo, skillCode;
}


DEFINE_SRPC_METHOD_2(MockPlayerSkillController, evSkillLearned,
    SkillCode, removeSkillCode, SkillCode, addSkillCode)
{
    addCallCount("evSkillLearned");

    removeSkillCode, addSkillCode;
}


DEFINE_SRPC_METHOD_2(MockPlayerSkillController, evPassiveSkillActivated,
    GameObjectInfo, entityInfo, SkillCode, skillCode)
{
    addCallCount("evPassiveSkillActivated");
    entityInfo, skillCode;
}
