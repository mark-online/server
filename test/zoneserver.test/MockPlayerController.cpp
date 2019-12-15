#include "ZoneServerTestPCH.h"
#include "MockPlayerController.h"
#include "ZoneServer/controller/callback/RewardCallback.h"
#include "ZoneServer/model/item/Inventory.h"

using namespace gideon::zoneserver;

MockPlayerController::MockPlayerController() :
    lastErrorCode_(ecWhatDidYouTest),
    lastIsMajorLevelUp_(false),
    nextMapCode_(invalidMapCode)
{
}


void MockPlayerController::rewardExp(ExpPoint exp)
{
    gc::RewardCallback* reward = queryRewardCallback();
    reward->expRewarded(exp);
}


GameObjectInfo MockPlayerController::getAnyEntity(ObjectType objectType) const
{
    for (const UnionEntityInfo& entityInfo : lastEntityInfos_) {
        if (entityInfo.objectType_ == objectType) {
            return entityInfo.asEntityInfo();
        }
    }
    return GameObjectInfo();
}

// = GamePlayRpc overriding

DEFINE_SRPC_METHOD_1(MockPlayerController, onReadyToPlay,
    ErrorCode, errorCode)
{
    addCallCount("onReadyToPlay");

    lastErrorCode_ = errorCode;
}

// = rpc::CharacterInterestAreaRpc overriding

DEFINE_SRPC_METHOD_1(MockPlayerController, evEntitiesAppeared,
    UnionEntityInfos, entityInfos)
{
    addCallCount("evEntitiesAppeared");

    lastEntityInfos_ = entityInfos;
}


DEFINE_SRPC_METHOD_1(MockPlayerController, evEntityAppeared,
    UnionEntityInfo, entityInfo)
{
    addCallCount("evEntityAppeared");

    lastAppearedEntity_ = entityInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerController, evEntitiesDisappeared,
    GameObjects, entities)
{
    addCallCount("evEntitiesDisappeared");

    entities;
}


DEFINE_SRPC_METHOD_1(MockPlayerController, evEntityDisappeared,
    GameObjectInfo, entityInfo)
{
    addCallCount("evEntityDisappeared");

    entityInfo;
}

// = rpc::EntityQueryRpc overriding

DEFINE_SRPC_METHOD_3(MockPlayerController, onSelectTarget,
    ErrorCode, errorCode, EntityStatusInfo, targetInfo, EntityStatusInfo, targetOfTargetInfo)
{
    addCallCount("onSelectTarget");

    lastErrorCode_ = errorCode;
    lastTargetInfo_ = targetInfo;
    lastTargetOfTargetInfo_ = targetOfTargetInfo;
}


// = rpc::CharacterSocialRpc overriding

DEFINE_SRPC_METHOD_2(MockPlayerController, evCreatureSaid,
    Nickname, nickname, ChatMessage, message)
{
    addCallCount("evCreatureSaid");

    nickname, message;
}

// = rpc::CreatureStatusRpc overriding

DEFINE_SRPC_METHOD_3(MockPlayerController, evPlayerDied,
    GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, ObjectPosition, position)
{
    addCallCount("evPlayerDied");

    creatureInfo, killerInfo, position;
}


DEFINE_SRPC_METHOD_3(MockPlayerController, evNpcDied,
    GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, GraveStoneInfo, graveStoneInfo)
{
    addCallCount("evNpcDied");
    lastGraveStoneInfo_ = graveStoneInfo;
    creatureInfo, killerInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerController, evPointsRestored,
    GameObjectInfo, playerInfo)
{
    addCallCount("evPointsRestored");

    playerInfo;
}

// = rpc::CreatureReviveRpc overriding

DEFINE_SRPC_METHOD_2(MockPlayerController, onRevive,
    ErrorCode, errorCode, ObjectPosition, position)
{
    addCallCount("onRevive");

    lastErrorCode_ = errorCode;
    position;
}

// = rpc::ActionBarRpc overriding
DEFINE_SRPC_METHOD_4(MockPlayerController, onSaveActionBar,
    ErrorCode, errorCode, ActionBarIndex, abiIndex,
    ActionBarPosition, abpIndex, DataCode, code)
{
    addCallCount("onSaveActionBar");

    lastErrorCode_ = errorCode;
    abiIndex, abpIndex, code;
}


// = rpc::GrowthRpc overriding
DEFINE_SRPC_METHOD_4(MockPlayerController, evPlayerLeveledUpInfo,
    CurrentLevel, levelInfo, ExpPoint, exp, ExpPoint, rewardExp, SkillPoint, skillPoint)
{
    addCallCount("evPlayerLeveledUpInfo");
    levelInfo, exp, rewardExp, skillPoint;
}


DEFINE_SRPC_METHOD_1(MockPlayerController, evExpPointUpdated,
    ExpPoint, currentExp)
{
    addCallCount("evExpPointUpdated");
    currentExp;
}


DEFINE_SRPC_METHOD_2(MockPlayerController, evPlayerLeveledUp,
    GameObjectInfo, playerInfo, bool, isMajorLevelUp)
{
    addCallCount("evPlayerLeveledUp");
    playerInfo;
    lastIsMajorLevelUp_ = isMajorLevelUp;
}


DEFINE_SRPC_METHOD_3(MockPlayerController, evPointChanged,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, currentPoint)
{
    addCallCount("evPointChanged");
    entityInfo, pointType, currentPoint;
}


DEFINE_SRPC_METHOD_4(MockPlayerController, evMaxPointChanged,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, currentPoint, uint32_t, maxPoint)
{
    addCallCount("evMaxPointChanged");
    entityInfo, pointType, currentPoint, maxPoint;
}


//DEFINE_SRPC_METHOD_2(MockPlayerController, evAttributeRateChanged,
//    AttributeRateIndex, rateType, AttributeRate, rate)
//{
//    addCallCount("evAttributeRateChanged");
//    rateType, rate;
//}

//
//DEFINE_SRPC_METHOD_2(MockPlayerController, evPowerChanged,
//    PowerType, powerType, uint32_t, currentPower)
//{
//    addCallCount("evPowerChanged");
//    powerType, currentPower;
//}
//

DEFINE_SRPC_METHOD_1(MockPlayerController, evFullCreatureStatusInfoChanged,
    FullCreatureStatusInfo, stats)
{
    addCallCount("evFullCreatureStatusInfoChanged");
    stats;
}


DEFINE_SRPC_METHOD_2(MockPlayerController, evCreatureStatusChanged,
    EffectStatusType, effectStatusType, int32_t, currentValue)
{
    addCallCount("evCreatureStatusChanged");
    effectStatusType, currentValue;
}


DEFINE_SRPC_METHOD_3(MockPlayerController, evShieldCreated,
    GameObjectInfo, entityInfo, PointType, pointType, uint32_t, shieldPoint)
{
    addCallCount("evShieldCreated");
    entityInfo, pointType, shieldPoint;
}


DEFINE_SRPC_METHOD_2(MockPlayerController, evShieldDestroyed,
    GameObjectInfo, entityInfo, PointType, pointType)
{
    addCallCount("evShieldDestroyed");
    entityInfo, pointType;
}


DEFINE_SRPC_METHOD_3(MockPlayerController, evShieldPointChanged,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, currentPoint)
{
    addCallCount("evShieldPointChanged");
    entityInfo, pointType, currentPoint;
}

// = rpc::Chao overriding

DEFINE_SRPC_METHOD_2(MockPlayerController, evChaoChanged,
    ObjectId, playerId, bool, isChao)
{
    addCallCount("evChaoChanged");

    playerId, isChao;
}


DEFINE_SRPC_METHOD_2(MockPlayerController, evTempChaoChanged,
    ObjectId, playerId, bool, isChao)
{
    addCallCount("evTempChaoChanged");

    playerId, isChao;
}


DEFINE_SRPC_METHOD_1(MockPlayerController, evChaoticUpdated,
    Chaotic, chaotic)
{
    addCallCount("evChaoticUpdated");

    chaotic;
}



// = rpc::DungeonRpc overriding

DEFINE_SRPC_METHOD_4(MockPlayerController, onEnterDungeon,
    ErrorCode, errorCode, WorldPosition, spawnPosition, std::string, mapData, PartyId, partyId)
{
    addCallCount("onEnterDungeon");

    lastErrorCode_ = errorCode;
    nextMapCode_ = spawnPosition.mapCode_;
    mapData, partyId;
}


DEFINE_SRPC_METHOD_2(MockPlayerController, onLeaveDungeon,
    ErrorCode, errorCode, WorldPosition, spawnPosition)
{
    addCallCount("onLeaveDungeon");

    lastErrorCode_ = errorCode;
    nextMapCode_ = spawnPosition.mapCode_;
    spawnPosition;
}


DEFINE_SRPC_METHOD_1(MockPlayerController, evMoneyRewarded,
    GameMoney, gameMoney)
{
    addCallCount("evMoneyRewarded");
    gameMoney;
}


DEFINE_SRPC_METHOD_2(MockPlayerController, evCharacterClassChanged,
    ObjectId, playerId, CharacterClass, characterClass)
{
    addCallCount("evCharacterClassChanged");
    playerId, characterClass;
}


DEFINE_SRPC_METHOD_2(MockPlayerController, evEmotionNotified,
    ObjectId, playerId, std::string, emotion)
{
    addCallCount("evEmotionNotified");
    playerId, emotion;
}
