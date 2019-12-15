#pragma once

#include "ZoneServer/controller/PlayerController.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerController
 *
 * 테스트 용 mock PlayerController
 */
class MockPlayerController :
    public zoneserver::gc::PlayerController,
    public sne::test::CallCounter
{
public:
    MockPlayerController();

    void rewardExp(ExpPoint exp);

    GameObjectInfo getAnyEntity(ObjectType objectType) const;

private:
    // = GamePlayRpc overriding
    OVERRIDE_SRPC_METHOD_1(onReadyToPlay,
        ErrorCode, errorCode);

    // = rpc::CharacterInterestAreaRpc overriding
    OVERRIDE_SRPC_METHOD_1(evEntitiesAppeared,
        UnionEntityInfos, entityInfos);
    OVERRIDE_SRPC_METHOD_1(evEntityAppeared,
        UnionEntityInfo, entityInfo);
    OVERRIDE_SRPC_METHOD_1(evEntitiesDisappeared,
        GameObjects, entities);
    OVERRIDE_SRPC_METHOD_1(evEntityDisappeared,
        GameObjectInfo, entityInfo);

    // = rpc::EntityQueryRpc overriding
    OVERRIDE_SRPC_METHOD_3(onSelectTarget,
        ErrorCode, errorCode, EntityStatusInfo, targetInfo, EntityStatusInfo, targetOfTargetInfo);

    // = rpc::CharacterSocialRpc overriding
    OVERRIDE_SRPC_METHOD_2(evCreatureSaid,
        Nickname, nickname, ChatMessage, message);

    // = rpc::CreatureStatusRpc overriding
    OVERRIDE_SRPC_METHOD_3(evPlayerDied,
        GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, ObjectPosition, position);
    OVERRIDE_SRPC_METHOD_3(evNpcDied,
        GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, GraveStoneInfo, graveStoneInfo);
    OVERRIDE_SRPC_METHOD_1(evPointsRestored,
        GameObjectInfo, playerInfo);

    // = rpc::CreatureReviveRpc overriding
    OVERRIDE_SRPC_METHOD_2(onRevive,
        ErrorCode, errorCode, ObjectPosition, position);


    // = rpc::ActionBarRpc overriding
    OVERRIDE_SRPC_METHOD_4(onSaveActionBar,
        ErrorCode, errorCode, ActionBarIndex, abiIndex,
        ActionBarPosition, abpIndex, DataCode, code);

    // = rpc::GrowthRpc overriding
    OVERRIDE_SRPC_METHOD_4(evPlayerLeveledUpInfo,
        CurrentLevel, levelInfo, ExpPoint, exp, ExpPoint, rewardExp, SkillPoint, skillPoint);
    OVERRIDE_SRPC_METHOD_1(evExpPointUpdated,
        ExpPoint, currentExp);
    OVERRIDE_SRPC_METHOD_2(evPlayerLeveledUp,
        GameObjectInfo, playerInfo, bool, isMajorLevelUp);

    // = rpc:StatsRpc overriding
    OVERRIDE_SRPC_METHOD_3(evPointChanged,
        GameObjectInfo, entityInfo, PointType, pointType, 
        uint32_t, currentPoint);
    OVERRIDE_SRPC_METHOD_4(evMaxPointChanged,
        GameObjectInfo, entityInfo, PointType, pointType, 
        uint32_t, currentPoint, uint32_t, maxPoint);
    //OVERRIDE_SRPC_METHOD_2(evAttributeRateChanged,
    //    AttributeRateIndex, rateType, AttributeRate, rate);
    //OVERRIDE_SRPC_METHOD_2(evPowerChanged,
    //    PowerType, powerType, uint32_t, currentPower);
    OVERRIDE_SRPC_METHOD_1(evFullCreatureStatusInfoChanged,
        FullCreatureStatusInfo, stats);
    OVERRIDE_SRPC_METHOD_2(evCreatureStatusChanged,
        EffectStatusType, effectStatusType, int32_t, currentValue);

    // = rpc::Chao overriding
    OVERRIDE_SRPC_METHOD_2(evChaoChanged,
        ObjectId, playerId, bool, isChao);
    OVERRIDE_SRPC_METHOD_2(evTempChaoChanged,
        ObjectId, playerId, bool, isChao);
    OVERRIDE_SRPC_METHOD_1(evChaoticUpdated,
        Chaotic, chaotic);
    OVERRIDE_SRPC_METHOD_3(evShieldCreated,
        GameObjectInfo, entityInfo, PointType, pointType, 
        uint32_t, shieldPoint);
    OVERRIDE_SRPC_METHOD_2(evShieldDestroyed,
        GameObjectInfo, entityInfo, PointType, pointType);
    OVERRIDE_SRPC_METHOD_3(evShieldPointChanged,
        GameObjectInfo, entityInfo, PointType, pointType, 
        uint32_t, currentPoint);

    // = rpc::DungeonRpc overriding
    OVERRIDE_SRPC_METHOD_4(onEnterDungeon,
        ErrorCode, errorCode, WorldPosition, spawnPosition, std::string, mapData, PartyId, partyId);
    OVERRIDE_SRPC_METHOD_2(onLeaveDungeon,
        ErrorCode, errorCode, WorldPosition, spawnPosition);

    OVERRIDE_SRPC_METHOD_1(evMoneyRewarded,
        GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_2(evCharacterClassChanged,
        ObjectId, playerId, CharacterClass, characterClass);

    OVERRIDE_SRPC_METHOD_2(evEmotionNotified,
        ObjectId, playerId, std::string, emotion);

public:
    ErrorCode lastErrorCode_;
    UnionEntityInfos lastEntityInfos_;
    UnionEntityInfo lastAppearedEntity_;
    EntityStatusInfo lastTargetInfo_;
    EntityStatusInfo lastTargetOfTargetInfo_;
    LifeStats lastStats_;

    GraveStoneInfo lastGraveStoneInfo_;
    LootInvenItemInfoMap lastGraveStoneInsideinfo_;
    bool lastIsMajorLevelUp_;

    Points lastMyPoints_;

    MapCode nextMapCode_;
};
