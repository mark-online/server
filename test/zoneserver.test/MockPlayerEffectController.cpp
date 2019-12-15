#include "ZoneServerTestPCH.h"
#include "MockPlayerEffectController.h"

using namespace gideon::zoneserver;

MockPlayerEffectController::MockPlayerEffectController(zoneserver::go::Entity* owner) :
    PlayerEffectController(owner)
{
}


DEFINE_SRPC_METHOD_3(MockPlayerEffectController, evEffectApplied,
    GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
    SkillEffectResult, skillEffectResult)
{
    addCallCount("evEffectApplied");
    targetInfo, sourceInfo, skillEffectResult;
}


DEFINE_SRPC_METHOD_3(MockPlayerEffectController, evItemEffected,
    GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
    ItemEffectResult, itemEffectResult)
{
    addCallCount("evItemEffected");
    targetInfo, sourceInfo, itemEffectResult;
}


DEFINE_SRPC_METHOD_3(MockPlayerEffectController, evMesmerizationEffected,
    GameObjectInfo, entityInfo, MesmerizationType, mezt, bool, isActivate)
{
    addCallCount("evMesmerizationEffected");
    entityInfo, mezt, isActivate;
}


DEFINE_SRPC_METHOD_3(MockPlayerEffectController, evCreatureTransformed,
    GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate)
{
    addCallCount("evCreatureTransformed");
    entityInfo, npcCode, isActivate;
}


DEFINE_SRPC_METHOD_3(MockPlayerEffectController, evCreatureMutated,
    GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate)
{
    addCallCount("evCreatureMutated");
    entityInfo, npcCode, isActivate;
}


DEFINE_SRPC_METHOD_1(MockPlayerEffectController, evCreatureReleaseHidden,
    UnionEntityInfo, entityInfo)
{
    addCallCount("evCreatureReleaseHidden");
    entityInfo;
}


DEFINE_SRPC_METHOD_2(MockPlayerEffectController, evCreatureMoveSpeedChanged,
    GameObjectInfo, entityInfo, float32_t, currentSpeed)
{
    addCallCount("evCreatureMoveSpeedChanged");
    entityInfo, currentSpeed;
}


DEFINE_SRPC_METHOD_3(MockPlayerEffectController, evCreatureFrenzied,
    GameObjectInfo, entityInfo, float32_t, currnetScale, bool, isActivate)
{
    addCallCount("evCreatureFrenzied");
    entityInfo, currnetScale, isActivate;
}


DEFINE_SRPC_METHOD_2(MockPlayerEffectController, evCreatureKnockbacked,
    GameObjectInfo, entityInfo, Position, position)
{
    addCallCount("evCreatureKnockbacked");
    entityInfo, position;
}


DEFINE_SRPC_METHOD_1(MockPlayerEffectController, evCreatureKnockbackReleased,
    GameObjectInfo, entityInfo)
{
    addCallCount("evCreatureKnockbackReleased");
    entityInfo;
}


DEFINE_SRPC_METHOD_2(MockPlayerEffectController, evCreatureDashing,
    GameObjectInfo, entityInfo, Position, position)
{
    addCallCount("evCreatureDashing");
    entityInfo, position;
}


DEFINE_SRPC_METHOD_1(MockPlayerEffectController, evPlayerGraveStoneStood,
    GraveStoneInfo, graveStroneInfo)
{
    addCallCount("evPlayerGraveStoneStood");
    lastGraveStoneInfo_ = graveStroneInfo;
}


DEFINE_SRPC_METHOD_2(MockPlayerEffectController, evEffectAdded,
    GameObjectInfo, entityInfo, DebuffBuffEffectInfo, info)
{
    addCallCount("evEffectAdded");
    entityInfo, info;
}
