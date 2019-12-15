#pragma once

#include "ZoneServer/controller/player/PlayerEffectController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerEffectController
 *
 * 테스트 용 mock MockPlayerEffectController
 */
class MockPlayerEffectController:
    public zoneserver::gc::PlayerEffectController,
    public sne::test::CallCounter
{
public:
    MockPlayerEffectController(zoneserver::go::Entity* owner);

    OVERRIDE_SRPC_METHOD_3(evEffectApplied,
        GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
        SkillEffectResult, skillEffectResult);
    OVERRIDE_SRPC_METHOD_3(evItemEffected,
        GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
        ItemEffectResult, itemEffectResult);
    OVERRIDE_SRPC_METHOD_3(evMesmerizationEffected,
        GameObjectInfo, entityInfo, MesmerizationType, mezt, bool, isActivate);
    OVERRIDE_SRPC_METHOD_3(evCreatureTransformed,
        GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);
    OVERRIDE_SRPC_METHOD_3(evCreatureMutated,
        GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate);
    OVERRIDE_SRPC_METHOD_1(evCreatureReleaseHidden,
        UnionEntityInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_2(evCreatureMoveSpeedChanged,
        GameObjectInfo, entityInfo, float32_t, currentSpeed);
    OVERRIDE_SRPC_METHOD_3(evCreatureFrenzied,
        GameObjectInfo, entityInfo, float32_t, currnetScale, bool, isActivate);
    OVERRIDE_SRPC_METHOD_2(evCreatureKnockbacked,
        GameObjectInfo, entityInfo, Position, position);
    OVERRIDE_SRPC_METHOD_1(evCreatureKnockbackReleased,
        GameObjectInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_2(evCreatureDashing,
        GameObjectInfo, entityInfo, Position, position);
    OVERRIDE_SRPC_METHOD_1(evPlayerGraveStoneStood,
        GraveStoneInfo, graveStroneInfo);

public:
    // 스킬 효과 추가
    OVERRIDE_SRPC_METHOD_2(evEffectAdded,
        GameObjectInfo, entityInfo, DebuffBuffEffectInfo, info);

public:
    GraveStoneInfo lastGraveStoneInfo_;
    LootInvenItemInfoMap lastGraveStoneInsideinfo_;
};
