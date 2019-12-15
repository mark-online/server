#include "BotPCH.h"
#include "BotPlayer.h"
#include <gideon/3d/3d.h>
#include <gideon/clientnet/ServerProxy.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/Rate.h>
#include <iostream>

namespace gideon { namespace bot {
namespace {

inline bool isAdjacentPosition(const Position& p1, const Position& p2)
{
    const float32_t tolerance = 0.1f;

    if (fabs(p1.x_ - p2.x_) > tolerance) {
        return false;
    }

    if (fabs(p1.y_ - p2.y_) > tolerance) {
        return false;
    }

    if (fabs(p1.z_ - p2.z_) > tolerance) {
        return false;
    }

    return true;
}

} // namespace

namespace {

inline std::wostream& operator<<(std::wostream& lhs, const GameObjectInfo& entityInfo)
{
    lhs << toString(entityInfo.objectType_) << L"-" << entityInfo.objectId_;
    return lhs;
}

} // namespace {

IMPLEMENT_SRPC_EVENT_DISPATCHER(BotPlayer);

BotPlayer::BotPlayer(clientnet::ServerProxy& serverProxy) :
    characterInfo_(nullptr),
    isEnterWorld_(false),
    isReadyToPlay_(false),
    isDestinationChanged_(false),
    isMoving_(false),
    isCastSkill_(false),
    shouldStopDestination_(false),
    lastMoveTick_(0),
    moveTolerance_(0.1f)
{
    serverProxy.registerRpcForwarder(*this);
    serverProxy.registerRpcReceiver(*this);
}


void BotPlayer::initialize(const UserId& userId, FullCharacterInfo& characterInfo)
{
    userId_ = userId;
    characterInfo_ = &characterInfo;
}


void BotPlayer::reset()
{
    userId_.clear();
    characterInfo_ = nullptr;
    isEnterWorld_ = false;
    isReadyToPlay_ = false;
    isDestinationChanged_ = false;
    isMoving_ = false;
    isCastSkill_ = false;
    lastMoveTick_ = 0;
}


void BotPlayer::worldEntered(const ObjectPosition& position)
{
    assert(isInitialized());

    characterInfo_->position_ = position;
    isEnterWorld_ = true;
    
    readyToPlay();
}


void BotPlayer::moveTo(const Position& destination)
{
    if (shouldStopDestination_) {
        return;
    }
    static_cast<Position&>(destination_) = destination;
    isMoving_ = true;
    lastMoveTick_ = GetTickCount();
    goToward(destination_);
}


void BotPlayer::move()
{
    // TODO: prevent movement while casting

    if (! isMoving_) {
        return;
    }

    const GameTime moveInterval = 100;
    const GameTime elapsedTime = GetTickCount() - lastMoveTick_;
    if (elapsedTime < moveInterval) {
        return;
    }

    ObjectPosition currentPosition = characterInfo_->position_;
    const float32_t distanceToDestin = calcCurrentPosition(currentPosition, elapsedTime);

    //SNE_LOG_DEBUG("move(%f, %f, %f)", elapsedTime / 1000.0f, distanceToDestin, moveTolerance_);

    if (isApproachedToDestination(distanceToDestin)) {
        isMoving_ = false;
        stop(currentPosition);
        return;
    }
    isMoving_ = true;
    lastMoveTick_ = GetTickCount();
    move(currentPosition);
}


bool BotPlayer::canCastManaSkill() const
{
    if (! isSuccessRate(500)) {
        return false;
    }
    return (! isCastSkill_ && isEnterWorld_);
}


void BotPlayer::castManaSkill()
{
    if (! isCastSkill_ && isEnterWorld_) {
        isCastSkill_ = true;
        /*castSkillTo(static_cast<const GameObjectInfo&>(*characterInfo_),
            16784385);*/
    }
}


bool BotPlayer::isApproachedToDestination(float32_t distance) const 
{
    const float32_t tolerance = 0.01f;
    return fabs(distance) <= tolerance;
}


float32_t BotPlayer::calcCurrentPosition(ObjectPosition& position,
    GameTime elapsedTime)
{
    const Vector2& destination = asVector2(destination_);

    const Vector2 dirDestination = destination - asVector2(position);
    const float32_t distToDestination = glm::length(dirDestination);
    const Vector2 direction = normalizeTo(dirDestination, distToDestination);

    const float32_t moveSpeed = 5.0f / 1000.0f;
    const float32_t moveDistance = moveSpeed * elapsedTime;
    Vector2 delta = direction * moveDistance;
    if (glm::length(delta) > distToDestination) {
        delta = direction * distToDestination;
    }

    position.x_ += delta.x;
    position.y_ += delta.y;
    //position.z_ = position.z_ + delta.z;
    position.heading_ = destination_.heading_;

    return glm::length(destination - asVector2(position));
}


bool BotPlayer::setDestination(const Position& destination)
{
    isDestinationChanged_ = false;

    const ObjectPosition source = characterInfo_->position_;
    const Vector2 dirDestination = asVector2(destination) - asVector2(source);
    const float32_t distToDestination = glm::length(dirDestination);
    const Vector2 direction = normalizeTo(dirDestination, distToDestination);

    if (isApproachedToDestination(distToDestination)) {
        if (! isMoving_) {
            const Heading newHeading = getHeading(direction);
            if (newHeading != source.heading_) {
                turn(newHeading);
            }
        }
        return false;
    }

    const float32_t moveDistance = (distToDestination - moveTolerance_);

    Position newDestination = source;
    newDestination.x_ += direction.x * moveDistance;
    newDestination.y_ += direction.y * moveDistance;
    //newDestination.z_ = ;

    isDestinationChanged_ = (! isAdjacentPosition(newDestination, destination_));
    if (isDestinationChanged_) {
        //SNE_LOG_DEBUG("Destination: (%f,%f)->(%f,%f)",
        //    destination_.x_, destination_.y_, newDestination.x_, newDestination.y_);

        static_cast<Position&>(destination_) = newDestination;
        destination_.heading_ = getHeading(direction);
    }

    return true;
}


// = sne::srpc::RpcForwarder overriding

void BotPlayer::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    std::wcout << L"BotPlayer::onForwarding(" << rpcId.get() << L"," <<
        rpcId.getMethodName() << L")\n";
}

// = sne::srpc::RpcReceiver overriding

void BotPlayer::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    std::wcout << L"BotPlayer::onReceiving(" << rpcId.get() << L"," <<
        rpcId.getMethodName() << L")\n";
}

// = rpc::CheatRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, cheat,
    ChatMessage, cheatMessage);


RECEIVE_SRPC_METHOD_2(BotPlayer, onCheat,
    ErrorCode, errorCode, ChatMessage, cheatMessage)
{
    errorCode, cheatMessage;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evZoneInUsers,
    ZoneUserInfos, infos)
{
    infos;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evWorldInUsers,
    WorldUserInfos, infos)
{
    infos;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evCheatValueTypeUpdated,
    CheatValueType, cheatType, uint64_t, currentValue)
{
    cheatType, currentValue;
}

RECEIVE_SRPC_METHOD_1(BotPlayer, evCheatCompleteQuestRemoved,
    QuestCode, questCode)
{
    questCode;
}


// = rpc::GameWorldRpc overriding

FORWARD_SRPC_METHOD_0(BotPlayer, readyToPlay);

FORWARD_SRPC_METHOD_0(BotPlayer, queryServerTime)

RECEIVE_SRPC_METHOD_1(BotPlayer, onReadyToPlay,
    ErrorCode, errorCode)
{
    isReadyToPlay_ = true;
    std::wcout << L"* " << userId_ <<
        L": Failed to ready to play(E" << errorCode << L")\n";
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onQueryServerTime,
    sec_t, serverTime)
{
    serverTime;
}

// = rpc::CreatureInterestAreaRpc overriding

RECEIVE_SRPC_METHOD_1(BotPlayer, evEntitiesAppeared,
    UnionEntityInfos, entityInfos)
{
    assert(isInitialized());

    for (const UnionEntityInfo& entityInfo : entityInfos) {
        assert(entityInfo.isValid());
        const EntityInfo& info = entityInfo.asEntityInfo();
        std::wcout << L"* " << userId_ <<
            L": Entity(" << toString(entityInfo.objectType_) << "-" <<
            entityInfo.getObjectId() << L") appeared(" <<
            info.position_.x_ << L"," << info.position_.y_ << L"," <<
            info.position_.z_ << L").\n";
    }
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evEntityAppeared,
    UnionEntityInfo, entityInfo)
{
    assert(isInitialized());

    const EntityInfo& info = entityInfo.asEntityInfo();
    std::wcout << L"* " << userId_ <<
        L": Entity(" << toString(entityInfo.objectType_) << "-" <<
        entityInfo.getObjectId() << L") appeared(" <<
        info.position_.x_ << L"," << info.position_.y_ << L"," <<
        info.position_.z_ << L").\n";
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evEntitiesDisappeared,
    GameObjects, entities)
{
    assert(isInitialized());

    for (const GameObjectInfo& entityInfo : entities) {
        std::wcout << L"* " << userId_ <<
            L": Creature(" << entityInfo << L") disappeared.\n";
    }
}



RECEIVE_SRPC_METHOD_1(BotPlayer, evEntityDisappeared,
    GameObjectInfo, entityInfo)
{
    assert(isInitialized());

    std::wcout << L"* " << userId_ <<
        L": Creature(" << entityInfo << L") disappeared.\n";
}

// = rpc::EntityQueryRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, selectTarget,
    GameObjectInfo, entityInfo);


RECEIVE_SRPC_METHOD_3(BotPlayer, onSelectTarget,
    ErrorCode, errorCode, EntityStatusInfo, targetInfo, EntityStatusInfo, targetOfTargetInfo)
{
    errorCode, targetInfo, targetOfTargetInfo;

    // TODO:
}

// = rpc::CreatureMovementRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, goToward,
    ObjectPosition, destination);


FORWARD_SRPC_METHOD_1(BotPlayer, move,
    ObjectPosition, position);


FORWARD_SRPC_METHOD_1(BotPlayer, stop,
    ObjectPosition, position);


FORWARD_SRPC_METHOD_1(BotPlayer, turn,
    Heading, heading);


FORWARD_SRPC_METHOD_1(BotPlayer, jump,
    ObjectPosition, position);


FORWARD_SRPC_METHOD_0(BotPlayer, run);


FORWARD_SRPC_METHOD_0(BotPlayer, walk);


FORWARD_SRPC_METHOD_0(BotPlayer, fallen);


FORWARD_SRPC_METHOD_0(BotPlayer, landing);


FORWARD_SRPC_METHOD_1(BotPlayer, environmentEntered,
    EnvironmentType, type);


FORWARD_SRPC_METHOD_0(BotPlayer, environmentLeft);


RECEIVE_SRPC_METHOD_1(BotPlayer, onLanding,
    HitPoint, damage)
{
    damage;
}


FORWARD_SRPC_METHOD_1(BotPlayer, correctionPosition,
    ObjectPosition, position);


FORWARD_SRPC_METHOD_1(BotPlayer, regionEntered,
    RegionCode, regionCode);


FORWARD_SRPC_METHOD_1(BotPlayer, regionLeft,
    RegionCode, regionCode);


RECEIVE_SRPC_METHOD_2(BotPlayer, evEntityGoneToward,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    assert(isInitialized());

    if (entityInfo == *characterInfo_) {
        // TODO: 이동 시작 요청에 대한 응답 처리
    }

    std::wcout << L"* " << userId_ <<
        L": Character(" << entityInfo <<
        L") went toward(" << position.x_ << L"," <<
        position.y_ << L"," << position.z_ << L").\n";
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEntityMoved,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    assert(isInitialized());

    if (entityInfo == *characterInfo_) {
        characterInfo_->position_ = position;
    }

    std::wcout << L"* " << userId_ <<
        L": Character(" << entityInfo <<
        L") moved(" << position.x_ << L"," <<
        position.y_ << L"," << position.z_ << L").\n";
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEntityStopped,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    assert(isInitialized());

    if (entityInfo == *characterInfo_) {
        characterInfo_->position_ = position;
    }

    std::wcout << L"* " << userId_ <<
        L": Character(" << entityInfo <<
        L") stopped(" << position.x_ << L"," <<
        position.y_ << L"," << position.z_ << L").\n";
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEntityTeleported,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    assert(isInitialized());

    if (entityInfo == *characterInfo_) {
        characterInfo_->position_ = position;
    }

    std::wcout << L"* " << userId_ <<
        L": Character(" << entityInfo <<
        L") teleported(" << position.x_ << L"," <<
        position.y_ << L"," << position.z_ << L").\n";
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEntityTurned,
    GameObjectInfo, entityInfo, Heading, heading)
{
    assert(isInitialized());

    if (entityInfo == *characterInfo_) {
        characterInfo_->position_.heading_ = heading;
    }

    std::wcout << L"* " << userId_ <<
        L": Character(" << entityInfo <<
        L") turned(" << int(heading) << L").\n";
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEntityJumped,
    GameObjectInfo, entityInfo, ObjectPosition, position)
{
    assert(isInitialized());

    if (entityInfo == *characterInfo_) {
        characterInfo_->position_ = position;
    }

    std::wcout << L"* " << userId_ <<
        L": Character(" << entityInfo <<
        L") jumped(" << position.x_ << L"," <<
        position.y_ << L"," << position.z_ << L").\n";
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evRun,
    GameObjectInfo, entityInfo)
{
    entityInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evWalked,
    GameObjectInfo, entityInfo)
{
    entityInfo;
}

// = rpc::CreatureSocialRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, say,
    ChatMessage, message);


RECEIVE_SRPC_METHOD_2(BotPlayer, evCreatureSaid,
    Nickname, nickname, ChatMessage, message)
{
    std::wcout << L"* " << userId_ <<
        L": Character(" << nickname <<
        L"): " << message << L").\n";
}

// = rpc::VehicleRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, dismountVehicle,
    bool, isForce);


RECEIVE_SRPC_METHOD_1(BotPlayer, onDismountVehicle,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evVehicleDismounted,
    GameObjectInfo, creatureInfo, float32_t, speed, bool, isForce)
{
    creatureInfo , speed, isForce;
    // TODO:
}

// = rpc::GliderRpc overriding


FORWARD_SRPC_METHOD_0(BotPlayer, dismountGlider);


RECEIVE_SRPC_METHOD_2(BotPlayer, onDismountGlider,
    ErrorCode, errorCode, uint32_t, currentDurability)
{
    errorCode, currentDurability;
}



RECEIVE_SRPC_METHOD_2(BotPlayer, evGliderDismounted,
    GameObjectInfo, creatureInfo, float32_t, speed)
{
    creatureInfo, speed;
    // TODO:
}

// = rpc::SkillRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, learnSkills,
    SkillCodes, skillCodes);


FORWARD_SRPC_METHOD_0(BotPlayer, resetLearnedSkills);


RECEIVE_SRPC_METHOD_2(BotPlayer, onLearnSkills,
    ErrorCode, errorCode, SkillPoint, currentSkillPoint)
{
    errorCode, currentSkillPoint;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onResetLearnedSkills,
    ErrorCode, errorCode, GameMoney, currentGameMoney, SkillPoint, currentSkillPoint)
{
    errorCode, currentGameMoney, currentSkillPoint;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evConcentrationSkillCancelled,
    GameObjectInfo, entityInfo, SkillCode, skillCode)
{
    // TODO:
    entityInfo, skillCode;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evConcentrationSkillCompleted,
    GameObjectInfo, entityInfo, SkillCode, skillCode)
{
    entityInfo, skillCode;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evSkillLearned,
    SkillCode, removeSkillCode, SkillCode, addSkillCode)
{
    removeSkillCode, addSkillCode;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evPassiveSkillActivated,
    GameObjectInfo, entityInfo, SkillCode, skillCode)
{
    entityInfo, skillCode;
}


RECEIVE_SRPC_METHOD_0(BotPlayer, evAllSkillRemoved)
{

}


// = rpc::InventoryRpc overriding

FORWARD_SRPC_METHOD_3(BotPlayer, moveInventoryItem,
    InvenType, invenType, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_3(BotPlayer, switchInventoryItem,
    InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2);


FORWARD_SRPC_METHOD_2(BotPlayer, removeInventoryItem,
    InvenType, invenType, ObjectId, itemId1);


FORWARD_SRPC_METHOD_1(BotPlayer, equipItem,
    ObjectId, itemId);


FORWARD_SRPC_METHOD_2(BotPlayer, unequipItem,
    ObjectId, itemId, SlotId, slotId);

                                 
FORWARD_SRPC_METHOD_2(BotPlayer, equipAccessoryItem,
    ObjectId, itemId, AccessoryIndex, index);


FORWARD_SRPC_METHOD_2(BotPlayer, unequipAccessoryItem,
    ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_4(BotPlayer, divideItem,
    InvenType, invenType, ObjectId, itemId, uint8_t, count, SlotId, slotId);


FORWARD_SRPC_METHOD_3(BotPlayer, movePlayerInvenItemToBankInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, SlotId, bankInvenSlot);


FORWARD_SRPC_METHOD_3(BotPlayer, moveBankInvenItemToPlayerInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, bankItemId, SlotId, playerInvenSlot);


FORWARD_SRPC_METHOD_3(BotPlayer, switchBankInvenItemAndPlayerInvenItem,
    GameObjectInfo, gameObjectInfo, ObjectId, playerItemId, ObjectId, bankItemId);


FORWARD_SRPC_METHOD_2(BotPlayer, moveInvenItemToVehicleInvenItem,
    ObjectId, playerItemId, SlotId, vehicleInvenSlot);


FORWARD_SRPC_METHOD_2(BotPlayer, moveVehicleInvenItemToPlayerInvenItem,
    ObjectId, vehicleItemId, SlotId, playerInvenSlot);


FORWARD_SRPC_METHOD_2(BotPlayer, switchVehicleInvenItemAndPlayerInvenItem,
    ObjectId, playerItemId, ObjectId, vehicleItemId);


FORWARD_SRPC_METHOD_2(BotPlayer, depositGameMoney,
    GameObjectInfo, targetInfo, GameMoney, invenGameMoney)


FORWARD_SRPC_METHOD_2(BotPlayer, withdrawGameMoney,
    GameObjectInfo, targetInfo, GameMoney, bankGameMoney);


RECEIVE_SRPC_METHOD_4(BotPlayer, onMoveInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, SlotId, slotId)
{
    // TODO:
    errorCode, invenType, itemId, slotId;
}


RECEIVE_SRPC_METHOD_4(BotPlayer, onSwitchInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2)
{
    // TODO:
    errorCode, invenType, itemId1, itemId2;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onRemoveInventoryItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId)
{
    errorCode, invenType, itemId;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onEquipItem,
    ErrorCode, errorCode, ObjectId, itemId)
{
    // TODO:
    errorCode, itemId;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onUnequipItem,
    ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId)
{
    // TODO:
    errorCode, itemId, slotId;
}


RECEIVE_SRPC_METHOD_4(BotPlayer, onDivideItem,
    ErrorCode, errorCode, InvenType, invenType, ObjectId, itemId, uint8_t, count)
{
    // TODO:
    errorCode, invenType, itemId, count;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onEquipAccessoryItem,
    ErrorCode, errorCode, ObjectId, itemId, AccessoryIndex, index)
{
    errorCode, itemId, index;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onUnequipAccessoryItem,
    ErrorCode, errorCode, ObjectId, itemId, SlotId, slotId)
{
    errorCode, itemId, slotId;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onMovePlayerInvenItemToBankInvenItem,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onMoveBankInvenItemToPlayerInvenItem,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onSwitchBankInvenItemAndPlayerInvenItem,
    ErrorCode, errorCode)
{
    errorCode;
}

RECEIVE_SRPC_METHOD_1(BotPlayer, onMoveInvenItemToVehicleInvenItem,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onMoveVehicleInvenItemToPlayerInvenItem,
    ErrorCode, errorCode)
{
    errorCode;
}

RECEIVE_SRPC_METHOD_1(BotPlayer, onSwitchVehicleInvenItemAndPlayerInvenItem,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onDepositGameMoney,
    ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney)
{
    errorCode, bankGameMoney, invenGameMoney;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onWithdrawGameMoney,
    ErrorCode, errorCode, GameMoney, bankGameMoney, GameMoney, invenGameMoney)
{
    errorCode, bankGameMoney, invenGameMoney;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evItemEquipped,
    GameObjectInfo, creatureInfo, EquipCode, equipCode)
{
    // TODO:
    creatureInfo, equipCode;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evItemUnequipped,
    GameObjectInfo, creatureInfo, EquipCode, equipCode)
{
    // TODO:
    creatureInfo, equipCode;
}



RECEIVE_SRPC_METHOD_3(BotPlayer, evInventoryItemCountUpdated,
    InvenType, invenType, ObjectId, itemId, uint8_t, ItemCount)
{
    invenType, itemId, ItemCount;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evInventoryItemAdded,
    InvenType, invenType, ItemInfo, itemInfo)
{
    invenType, itemInfo;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evInventoryEquipItemChanged,
    ObjectId, itemId, EquipCode, newEquipCode, uint8_t, socketCount)
{
    itemId, newEquipCode, socketCount;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evEquipItemEnchanted,
    ObjectId, itemId, SocketSlotId, id, EquipSocketInfo, socketInfo)
{
    itemId, id, socketInfo;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEquipItemUnenchanted,
    ObjectId, itemId, SocketSlotId, id)
{
    itemId, id;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evInventoryItemRemoved,
    InvenType, invenType, ObjectId, itemId)
{
    invenType, itemId;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evInventoryItemMoved,
    InvenType, invenType, ObjectId, itemId, SlotId, slotId)
{
    invenType, itemId, slotId;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evInventoryItemSwitched,
    InvenType, invenType, ObjectId, itemId1, ObjectId, itemId2)
{
    invenType, itemId1, itemId2;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evUnequipItemReplaced,
    ObjectId, equipItemId, SlotId, slotId, EquipPart, unequipPrt)
{
    equipItemId, slotId, unequipPrt;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEquipItemReplaced,
    ObjectId, equipItemId, EquipPart, equipPrt)
{
    equipItemId, equipPrt;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evInventoryWithEquipItemReplaced,
    ObjectId, equipItemId1, ObjectId, equipItemId2)
{
    equipItemId1, equipItemId2;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evUnequipAccessoryItemReplaced, 
    ObjectId, itemId, SlotId, slotId, AccessoryIndex, unequipPrt)
{
    itemId, slotId, unequipPrt;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEquipAccessoryItemReplaced,
    ObjectId, itemId, AccessoryIndex, equipPrt)
{
    itemId, equipPrt;
}


RECEIVE_SRPC_METHOD_4(BotPlayer, evInventoryWithAccessoryItemReplaced,
    ObjectId, unequipItemId, AccessoryIndex, unequipIndex,
    ObjectId, equipItemId, AccessoryIndex, equipIndex)
{
    unequipItemId, unequipIndex, equipItemId, equipIndex;
}


RECEIVE_SRPC_METHOD_3(BotPlayer,evInventoryInfoUpdated, 
    InvenType, invenTpe, bool, isCashSlot, uint8_t, currentExtendSlot)
{
    invenTpe, isCashSlot, currentExtendSlot;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evQuestItemAdded, 
    QuestItemInfo, questItemInfo)
{
    questItemInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evQuestItemRemoved,
    ObjectId, itemId)
{
    itemId;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evQuestItemUsableCountUpdated,
    ObjectId, itemId, uint8_t, usableCount)
{
    itemId, usableCount;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evQuestItemCountUpdated,
    ObjectId, itemId, uint8_t, stackCount)
{
    itemId, stackCount;
}

// = rpc::EntityStatusRpc overriding


FORWARD_SRPC_METHOD_3(BotPlayer, unenchantEquip,
    GameObjectInfo, checkObjectInfo, ObjectId, equipItemId, SocketSlotId, openSlotIndex);


FORWARD_SRPC_METHOD_1(BotPlayer, useVehicleItem,
    ObjectId, itemId);


FORWARD_SRPC_METHOD_1(BotPlayer, useGliderItem,
    ObjectId, itemId);


FORWARD_SRPC_METHOD_1(BotPlayer, selectVehicleItem,
    ObjectId, itemId);


FORWARD_SRPC_METHOD_1(BotPlayer, selectGliderItem,
    ObjectId, itemId);


FORWARD_SRPC_METHOD_1(BotPlayer, repairGliderItem,
    ObjectId, itemId);


FORWARD_SRPC_METHOD_1(BotPlayer, queryLootItemInfoMap,
    GameObjectInfo, targetInfo);


RECEIVE_SRPC_METHOD_4(BotPlayer, onUnenchantEquip,
    ErrorCode, errorCode, ObjectId, equipItemId, uint8_t, openSlotIndex, GameMoney, currentGameMoney)
{
    errorCode, equipItemId, openSlotIndex, currentGameMoney;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onUseVehicleItem,
    ErrorCode, errorCode, VehicleInfo, info)
{
    errorCode, info;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onUseGliderItem,
    ErrorCode, errorCode, GliderInfo, info)
{
    errorCode, info;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onSelectVehicleItem,
    ErrorCode, errorCode, ObjectId, itemId)
{
    errorCode, itemId;
}



RECEIVE_SRPC_METHOD_2(BotPlayer, onSelectGliderItem,
    ErrorCode, errorCode, ObjectId, itemId)
{
    errorCode, itemId;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onRepairGliderItem,
    ErrorCode, errorCode, ObjectId, itemId, GameMoney, currentGameMoney)
{
    errorCode, itemId, currentGameMoney;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onLootItemInfoMap,
    ErrorCode, errorCode, GameObjectInfo, targetInfo, LootInvenItemInfoMap, infoMap)
{
    errorCode, targetInfo, infoMap;
}


FORWARD_SRPC_METHOD_2(BotPlayer, buyItem,
    ObjectId, npcId, BaseItemInfo, itemInfo);


FORWARD_SRPC_METHOD_3(BotPlayer, sellItem,
    ObjectId, npcId, ObjectId, itemId, uint8_t, itemCount);


FORWARD_SRPC_METHOD_1(BotPlayer, queryBuyBackItemInfos,
    ObjectId, npcId);


FORWARD_SRPC_METHOD_2(BotPlayer, buyBackItem,
    ObjectId, npcId, uint32_t, index);


RECEIVE_SRPC_METHOD_5(BotPlayer, onBuyItem,
    ErrorCode, errorCode, ObjectId, npcId, BaseItemInfo, itemInfo,
    CostType, costType, uint64_t, currentCostValue)
{
    errorCode, npcId, itemInfo, costType, currentCostValue;
}


RECEIVE_SRPC_METHOD_5(BotPlayer, onSellItem,
    ErrorCode, errorCode, uint32_t, buyBackIndex, BuyBackItemInfo, buyBackInfo, CostType, costType, uint64_t, currentValue)
{
    errorCode, buyBackIndex, buyBackInfo, costType, currentValue;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onBuyBackItemInfos,
    ErrorCode, errorCode, BuyBackItemInfoMap, itemMap)
{
    errorCode, itemMap;
}


RECEIVE_SRPC_METHOD_4(BotPlayer, onBuyBackItem,
    ErrorCode, errorCode, uint32_t, index, CostType, ct, uint64_t, currentValue)
{
    errorCode, index, ct, currentValue;
}
// = rpc::EntityStatusRpc overriding

FORWARD_SRPC_METHOD_0(BotPlayer, commitSuicide);


FORWARD_SRPC_METHOD_0(BotPlayer, releaseBeginnerProtection);


RECEIVE_SRPC_METHOD_3(BotPlayer, evPlayerDied,
    GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, ObjectPosition, position)
{
    // TODO:
    creatureInfo, killerInfo, position;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evNpcDied,
    GameObjectInfo, creatureInfo, GameObjectInfo, killerInfo, GraveStoneInfo, graveStoneInfo)
{
    // TODO:
    creatureInfo, killerInfo, graveStoneInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evThreated,
    GameObjectInfo, entityInfo)
{
    // TODO:
    entityInfo;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evTargetSelected,
    GameObjectInfo, creatureInfo, GameObjectInfo, targetInfo, bool, isAggressive)
{
    // TODO:
    creatureInfo, targetInfo, isAggressive;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evTargetChanged,
    GameObjectInfo, entityInfo, EntityStatusInfo, targetStatusInfo)
{
    entityInfo, targetStatusInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evPointsRestored,
    GameObjectInfo, playerInfo)
{
    playerInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evBeginnerProtectionReleased,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evCombatStateChanged,
    ObjectId, playerId, bool, isCombatState)
{
    playerId, isCombatState;
}


// = rpc::CreatureReviveRpc overriding

FORWARD_SRPC_METHOD_0(BotPlayer, revive);


RECEIVE_SRPC_METHOD_2(BotPlayer, onRevive,
    ErrorCode, errorCode, ObjectPosition, position)
{
    // TODO:
    errorCode, position;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evPlayerRevived,
    ObjectId, reviverId, ObjectPosition, position)
{
    reviverId, position;
}


FORWARD_SRPC_METHOD_3(BotPlayer, saveActionBar,
    ActionBarIndex, abiIndex, ActionBarPosition, abpIndex, DataCode, code);


FORWARD_SRPC_METHOD_1(BotPlayer, lockActionBar,
    bool, isLocked)


RECEIVE_SRPC_METHOD_4(BotPlayer,onSaveActionBar,
    ErrorCode, errorCode, ActionBarIndex, abiIndex,
    ActionBarPosition, abpIndex, DataCode, code)
{
    errorCode, abiIndex, abpIndex, code;
}


RECEIVE_SRPC_METHOD_4(BotPlayer, evPlayerLeveledUpInfo,
    CurrentLevel, levelInfo, ExpPoint, exp, ExpPoint, rewardExp, SkillPoint, skillPoint)
{
    levelInfo, exp, rewardExp, skillPoint;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evExpPointUpdated,
    ExpPoint, currentExp)
{
    currentExp;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evPlayerLeveledUp,
    GameObjectInfo, playerInfo, bool, isMajorLevelUp)
{
    playerInfo, isMajorLevelUp;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evPointChanged,
    GameObjectInfo, creatureInfo, PointType, pointType, 
    uint32_t, currentPoint)
{
    creatureInfo, pointType, currentPoint;
}


RECEIVE_SRPC_METHOD_4(BotPlayer, evMaxPointChanged,
    GameObjectInfo, creatureInfo, PointType, pointType, 
    uint32_t, currentPoint, uint32_t, maxPoint)
{
    creatureInfo, pointType, currentPoint, maxPoint;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evCreatureStatusChanged,
    EffectStatusType, effectStatusType, int32_t, currentValue)
{
    effectStatusType, currentValue;
}


RECEIVE_SRPC_METHOD_2(BotPlayer,evAllAttributeChanged,
    AttributeRates, rates, bool, isResist)
{
    rates, isResist;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evFullCreatureStatusInfoChanged,
    FullCreatureStatusInfo, stats)
{
    stats;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evLifeStatusChanged,
    GameObjectInfo, entityInfo, LifeStats, stats)
{
    entityInfo, stats;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evShieldCreated,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, shieldPoint)
{
    entityInfo, pointType, shieldPoint;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evShieldDestroyed,
    GameObjectInfo, entityInfo, PointType, pointType)
{
    entityInfo, pointType;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evShieldPointChanged,
    GameObjectInfo, entityInfo, PointType, pointType, 
    uint32_t, currentPoint)
{
    entityInfo, pointType, currentPoint;
}

RECEIVE_SRPC_METHOD_2(BotPlayer, evChaoChanged,
    ObjectId, playerId, bool, isChao)
{
    playerId, isChao;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evTempChaoChanged,
    ObjectId, playerId, bool, isChao)
{
    playerId, isChao;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evChaoticUpdated,
    Chaotic, chaotic)
{
    chaotic;
}

// = rpc::DungeonRpc overriding


FORWARD_SRPC_METHOD_1(BotPlayer, requestTrade,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_2(BotPlayer, respondTrade,
    ObjectId, playerId, bool, isAnswer);


FORWARD_SRPC_METHOD_0(BotPlayer, cancelTrade); 


FORWARD_SRPC_METHOD_1(BotPlayer, toggleTradeReady,
    GameMoney, gameMoney);


FORWARD_SRPC_METHOD_0(BotPlayer, toggleTradeConform);


FORWARD_SRPC_METHOD_1(BotPlayer, addTradeItem, 
    ObjectId, itemId);


FORWARD_SRPC_METHOD_1(BotPlayer, removeTradeItem, 
    ObjectId, itemId);


FORWARD_SRPC_METHOD_2(BotPlayer, switchTradeItem, 
    ObjectId, invenItemId, ObjectId, tradeItemId);



RECEIVE_SRPC_METHOD_2(BotPlayer, onRequestTrade,
    ErrorCode, erroCode, ObjectId, playerId)
{
    erroCode, playerId;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onRespondTrade,
    ErrorCode, erroCode, ObjectId, playerId, bool, isAnwser)
{
    erroCode, playerId, isAnwser;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onCancelTrade,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onToggleTradeReady,
    ErrorCode, errorCode, bool, isReady, GameMoney, gameMoney)
{
    errorCode, isReady, gameMoney;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onToggleTradeConform,
    ErrorCode, errorCode, bool, isConform)
{
    errorCode, isConform;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onAddTradeItem, 
    ErrorCode, errorCode, ObjectId, itemId)
{
    errorCode, itemId;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onRemoveTradeItem, 
    ErrorCode, errorCode, ObjectId, itemId)
{
    errorCode, itemId;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onSwitchTradeItem, 
    ErrorCode, errorCode, ObjectId, invenItemId, ObjectId, tradeItemId)
{
    errorCode, invenItemId, tradeItemId;
}



RECEIVE_SRPC_METHOD_1(BotPlayer, evTradeRequested,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evTradeReponded,
    ObjectId, playerId, bool, isAnwser)
{
    playerId, isAnwser;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evTradeCancelled,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evTradeReadyToggled,
    bool, isReady, GameMoney, gameMoney)
{
    isReady, gameMoney;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evTradeConformToggled,
    bool, isConform)
{
    isConform;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evTradeItemAdded,
    ItemInfo, itemInfo)
{
    itemInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evTradeItemRemoved,
    ObjectId, itemId)
{
    itemId;
}



// = rpc::DungeonRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, enterDungeon,
    ObjectId, dungeonId);


FORWARD_SRPC_METHOD_0(BotPlayer, leaveDungeon);


RECEIVE_SRPC_METHOD_4(BotPlayer, onEnterDungeon,
    ErrorCode, errorCode, WorldPosition, spawnPosition, std::string, mapData, PartyId, partyId)
{
    errorCode, spawnPosition, mapData, partyId;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onLeaveDungeon,
    ErrorCode, errorCode, WorldPosition, spawnPosition)
{
    errorCode, spawnPosition;
}


// = QuestRpc overriding

FORWARD_SRPC_METHOD_2(BotPlayer, acceptQuest,
    QuestCode, questCode, GameObjectInfo, objectInfo);


FORWARD_SRPC_METHOD_1(BotPlayer, cancelQuest,
    QuestCode, questCode);


FORWARD_SRPC_METHOD_2(BotPlayer, completeTransportMission,
    QuestCode, questCode, QuestMissionCode, questMissionCode)


FORWARD_SRPC_METHOD_3(BotPlayer, completeQuest,
    QuestCode, questCode, ObjectId, npcId, DataCode, selectItemCode);


FORWARD_SRPC_METHOD_4(BotPlayer, completeTransportQuest,
    QuestCode, questCode, QuestMissionCode, questMissionCode, ObjectId, npcId, DataCode, selectItemCode);


FORWARD_SRPC_METHOD_2(BotPlayer, doContentsQuest, 
    QuestContentMissionType, contentMissionType, DataCode, dataCode);


FORWARD_SRPC_METHOD_1(BotPlayer, pushQuestToParty, 
    QuestCode, questCode);


FORWARD_SRPC_METHOD_3(BotPlayer, responsePushQuestToParty, 
    QuestCode, questCode, GameObjectInfo, pusherInfo, bool, isAccept);


RECEIVE_SRPC_METHOD_2(BotPlayer, onAcceptQuest,
    ErrorCode, errorCode, QuestCode, questCode)
{
    errorCode, questCode; 
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onCancelQuest,
    ErrorCode, errorCode, QuestCode, questCode)
{
    errorCode, questCode;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onCompleteTransportMission,
    ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode)
{
    errorCode, questCode, questMissionCode;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onCompleteQuest,
    ErrorCode, errorCode, QuestCode, questCode, DataCode, selectItemCode)
{
    errorCode, questCode, selectItemCode;
}


RECEIVE_SRPC_METHOD_5(BotPlayer, onCompleteTransportQuest,
    ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode, 
    ObjectId, npcId, DataCode, selectItemCode)
{
    errorCode, questCode, questMissionCode, npcId, selectItemCode;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onPushQuestToParty, 
    ErrorCode, errorCode, QuestCode, questCode)
{
    errorCode, questCode;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, onResponsePushQuestToParty, 
    ErrorCode, errorCode, QuestCode, questCode, bool, isAccept)
{
    errorCode, questCode, isAccept;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evQuestMissionUpdated,
    QuestCode, questCode, QuestMissionCode, missionCode,
    QuestGoalInfo, questGoalInfo)
{
    questCode, missionCode, questGoalInfo;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evQuestToPartyPushed, 
    QuestCode, questCode, GameObjectInfo, memberInfo)
{
    questCode, memberInfo;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evQuestToPartyPushResult, 
    QuestCode, questCode, GameObjectInfo, member, QuestToPartyResultType, resultType)
{
    questCode, member, resultType;
}

// = rpc::RewardRpc overriding

RECEIVE_SRPC_METHOD_1(BotPlayer, evMoneyRewarded,
    GameMoney, gameMoney)
{
    gameMoney;
}

// = TeleportRpc

FORWARD_SRPC_METHOD_2(BotPlayer, teleportToRegion,
    MapCode, worldMapCode, RegionCode, spawnRegionCode);


FORWARD_SRPC_METHOD_2(BotPlayer, teleportToPosition,
    MapCode, worldMapCode, Position, position);


FORWARD_SRPC_METHOD_1(BotPlayer, addBindRecall,
    ObjectId, bindlingId);


FORWARD_SRPC_METHOD_1(BotPlayer, removeBindRecall,
    ObjectId, bindlingId);


FORWARD_SRPC_METHOD_1(BotPlayer, setRecallToPlayer,
    Nickname, nickname);


FORWARD_SRPC_METHOD_1(BotPlayer, responseRecall,
    bool, isAnswer);


RECEIVE_SRPC_METHOD_5(BotPlayer, onTeleportTo,
    ErrorCode, errorCode, MapCode, worldMapCode, ObjectPosition, spawnPosition,
    MigrationTicket, migrationTicket, GameMoney, currentGameMoney)
{
    errorCode, worldMapCode, spawnPosition, migrationTicket, currentGameMoney;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onAddBindRecall,
    ErrorCode, errorCode, BindRecallInfo, bindRecallInfo)
{
    errorCode, bindRecallInfo;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onRemoveBindRecall,
    ErrorCode, errorCode, ObjectId, bindlingId)
{
    errorCode, bindlingId;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onSetRecallToPlayer,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onResponseRecall,
    ErrorCode, errorCode, MigrationTicket, migrationTicket)
{
    errorCode, migrationTicket;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evBindRecallTicket,
    ErrorCode, errorCode, MigrationTicket, migrationTicket)
{
    errorCode, migrationTicket;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evRecallRequested,
    Nickname, callerNickname)
{
    callerNickname;
}


// = rpc::TreasureRpc overriding


FORWARD_SRPC_METHOD_1(BotPlayer, closeTreasure,
    ObjectId, treasureId);

RECEIVE_SRPC_METHOD_2(BotPlayer, onCloseTreasure,
    ErrorCode, errorCode, ObjectId, treasureId)
{
    errorCode, treasureId;
}


// = rpc::HarvestRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, acquireHarvest,
    DataCode, itemCode);


RECEIVE_SRPC_METHOD_2(BotPlayer, onAcquireHarvest,
    ErrorCode, errorCode, DataCode, itemCode)
{
    errorCode, itemCode;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evHarvestRewarded,
    ObjectId, haravestId, BaseItemInfos, itemInfos,
    QuestItemInfos, questItemInfos)
{
    haravestId, itemInfos, questItemInfos;
}


FORWARD_SRPC_METHOD_1(BotPlayer, queryPartyMemberSubInfo,
    ObjectId, memberId);


RECEIVE_SRPC_METHOD_6(BotPlayer, onPartyMemberSubInfo,
    ObjectId, memberId, CharacterClass, characterClass,
    CreatureLevel, level, HitPoints, hitPoints, ManaPoints, manaPoints, Position, position)
{
    memberId, characterClass, level, hitPoints, manaPoints, position;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evPartyMemberLevelup,
    ObjectId, objectId, CreatureLevel, level)
{
    objectId, level;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evPartyMemberMoved,
    ObjectId, objectId, Position, position)
{
    objectId, position;
}


FORWARD_SRPC_METHOD_2(BotPlayer, createAnchor,
    ObjectId, itemId, ObjectPosition, position);
//
//
//FORWARD_SRPC_METHOD_1(BotPlayer, buildAnchor,
//    GameObjectInfo, anchorInfo);
//
//
//FORWARD_SRPC_METHOD_3(BotPlayer, putfuelItem,
//    GameObjectInfo, anchorInfo, ObjectId, itemId, SlotId, ancorInvenId);
//
//
//FORWARD_SRPC_METHOD_3(BotPlayer, popfuelItem,
//    GameObjectInfo, anchorInfo, ObjectId, itemId, SlotId, invenId);
//
//
//FORWARD_SRPC_METHOD_1(BotPlayer, queryInsideAnchorInfo,
//    GameObjectInfo, anchorInfo);
//
//
//FORWARD_SRPC_METHOD_3(BotPlayer, excuteAnchorEffectByPosition,
//    GameObjectInfo, anchorInfo, uint8_t, index, Position, position);
//
//
//FORWARD_SRPC_METHOD_3(BotPlayer, excuteAnchorEffectByTarget,
//    GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo);


RECEIVE_SRPC_METHOD_1(BotPlayer, onCreateAnchor,
    ErrorCode, errorCode)
{
    errorCode;
}
//
//
//RECEIVE_SRPC_METHOD_2(BotPlayer, onBuildAnchor,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo)
//{
//    errorCode, anchorInfo;
//}
//
//
//RECEIVE_SRPC_METHOD_3(BotPlayer, onPutfuelItem,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemInfo, itemInfo)
//{
//    errorCode, anchorInfo, itemInfo;
//}
//
//
//RECEIVE_SRPC_METHOD_3(BotPlayer, onPopfuelItem,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ObjectId, itemId)
//{
//    errorCode, anchorInfo, itemId;
//}
//
//
//RECEIVE_SRPC_METHOD_3(BotPlayer, onQueryInsideAnchorInfo,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, ItemMap, itemMap)
//{
//    errorCode, anchorInfo, itemMap;
//}
//
//
//
//RECEIVE_SRPC_METHOD_4(BotPlayer, onExcuteAnchorEffectByPosition,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, Position, position)
//{
//    errorCode, anchorInfo, index, position;
//}
//
//
//RECEIVE_SRPC_METHOD_4(BotPlayer, onExcuteAnchorEffectByTarget,
//    ErrorCode, errorCode, GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo)
//{
//        errorCode, anchorInfo, index, targetInfo;
//}
//
//
//RECEIVE_SRPC_METHOD_3(BotPlayer, evAnchorStartBuilt,
//    GameObjectInfo, anchorInfo, ObjectId, ownerId, sec_t, startBuildTime)
//{
//    anchorInfo, ownerId, startBuildTime;
//}
//
//
//RECEIVE_SRPC_METHOD_1(BotPlayer, evAnchorBroken,
//    GameObjectInfo, anchorInfo)
//{
//    anchorInfo;
//}
//
//
//RECEIVE_SRPC_METHOD_1(BotPlayer, evAnchorReconnaissanced,
//    Positions, positions)
//{
//    positions;
//}
//
//
//RECEIVE_SRPC_METHOD_3(BotPlayer, evAnchorPositionFired,
//    GameObjectInfo, anchorInfo, uint8_t, index, Position, position)
//{
//    anchorInfo, index, position;
//}
//
//
//RECEIVE_SRPC_METHOD_3(BotPlayer, evAnchorTargetFired,
//    GameObjectInfo, anchorInfo, uint8_t, index, GameObjectInfo, targetInfo)
//{
//    anchorInfo, index, targetInfo;
//}


RECEIVE_SRPC_METHOD_2(BotPlayer, evCharacterClassChanged,
    ObjectId, playerId, CharacterClass, characterClass)
{
    playerId, characterClass;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evBotMovePosition,
    Position, position, bool, shouldStopDestination)
{
    if (isMoving_) {
        stop(characterInfo_->position_);
    }
    shouldStopDestination_ = shouldStopDestination;
    static_cast<Position&>(destination_) = position;
    const ObjectPosition source = characterInfo_->position_;
    const Heading newHeading = getHeading(position, source);
    const Vector2 direction = getDirection(newHeading);

    isDestinationChanged_ = (! isAdjacentPosition(position, source));
    if (isDestinationChanged_) {
        //SNE_LOG_DEBUG("Destination: (%f,%f)->(%f,%f)",
        //    destination_.x_, destination_.y_, newDestination.x_, newDestination.y_);

        static_cast<Position&>(destination_) = position;
        destination_.heading_ = getHeading(direction);
    }

    if (newHeading != source.heading_) {
        turn(newHeading);
    }

    goToward(destination_);
    isMoving_ = true;
}


RECEIVE_SRPC_METHOD_0(BotPlayer, evBotCastingSkill)
{
    if (isCastSkill_) {
        //cancelCastingSkill(16784385);
    }
    castManaSkill();    
}


FORWARD_SRPC_METHOD_1(BotPlayer, notifyEmotion,
    std::string, emotion);


RECEIVE_SRPC_METHOD_2(BotPlayer, evEmotionNotified,
    ObjectId, playerId, std::string, emotion)
{
    playerId, emotion;
}

// = rpc::GuildRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, purchaseVault,
    BaseVaultInfo, vaultInfo);


FORWARD_SRPC_METHOD_1(BotPlayer, activateGuildSkill,
    SkillCode, skillCode);


FORWARD_SRPC_METHOD_0(BotPlayer, deactivateGuildSkills)


RECEIVE_SRPC_METHOD_2(BotPlayer, onPurchaseVault,
    ErrorCode, errorCode, BaseVaultInfo, vaultInfo)
{
    errorCode, vaultInfo;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, onActivateGuildSkill,
    ErrorCode, errorCode, SkillCode, skillCode)
{
    errorCode, skillCode;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, onDeactivateGuildSkills,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evGuildCreated,
    ObjectId, characterId, BaseGuildInfo, guildInfo)
{
    characterId, guildInfo;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evGuildJoined,
    ObjectId, characterId, BaseGuildInfo, guildInfo)
{
    characterId, guildInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evGuildLeft,
    ObjectId, characterId)
{
    characterId;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evGuildSkillActivated,
    SkillCode, skillCode)
{
    skillCode;
}


RECEIVE_SRPC_METHOD_0(BotPlayer, evGuildSkillsDeactivated)
{
}

// = rpc::PropertyRpc overriding

FORWARD_SRPC_METHOD_0(BotPlayer, loadProperties);


FORWARD_SRPC_METHOD_2(BotPlayer, saveProperties,
                       std::string, config, std::string, prefs);


RECEIVE_SRPC_METHOD_2(BotPlayer, onLoadProperties,
                       std::string, config, std::string, prefs)
{
    config, prefs;
}

// = rpc::NpcDialogRpc overriding

FORWARD_SRPC_METHOD_1(BotPlayer, openDialog,
    GameObjectInfo, npcInfo);


FORWARD_SRPC_METHOD_1(BotPlayer, closeDialog,
    GameObjectInfo, npcInfo);


RECEIVE_SRPC_METHOD_2(BotPlayer, evDialogOpened,
    GameObjectInfo, npc, GameObjectInfo, requester)
{
    npc, requester;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evDialogClosed,
    GameObjectInfo, npc, GameObjectInfo, requester)
{
    npc, requester;
}    


// = rpc::ActionRpc overriding

RECEIVE_SRPC_METHOD_2(BotPlayer, evActionPlayed,
    GameObjectInfo, entityInfo, uint32_t, actionCode)
{
    entityInfo, actionCode;
}

// = rpc::NpcTalkingRpc overriding

RECEIVE_SRPC_METHOD_2(BotPlayer, evTalkedFrom,
    GameObjectInfo, talker, NpcTalkingCode, talkingCode)
{
    talker, talkingCode;
}

// = rpc::CastRpc override overriding

FORWARD_SRPC_METHOD_1(BotPlayer, startCasting,
    StartCastInfo, startInfo);


FORWARD_SRPC_METHOD_0(BotPlayer, cancelCasting);


RECEIVE_SRPC_METHOD_2(BotPlayer, onStartCasting,
    ErrorCode, errorCode, StartCastInfo, startInfo)
{
    errorCode, startInfo;
}



RECEIVE_SRPC_METHOD_1(BotPlayer, evCasted,
    StartCastResultInfo, startInfo)
{
    startInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evCastCanceled,
    CancelCastResultInfo, cancelInfo)
{
    cancelInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evCastCompleted,
    CompleteCastResultInfo, completeInfo)
{
    completeInfo;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evCastCompleteFailed,
    FailCompletedCastResultInfo, failInfo)
{
    failInfo;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evEffectApplied,
    GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
    SkillEffectResult, skillEffectResult)
{
    targetInfo, sourceInfo, skillEffectResult;
}

RECEIVE_SRPC_METHOD_3(BotPlayer, evItemEffected,
    GameObjectInfo, targetInfo, GameObjectInfo, sourceInfo,
    ItemEffectResult, itemEffectResult)
{
    targetInfo, sourceInfo, itemEffectResult;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evMesmerizationEffected,
    GameObjectInfo, entityInfo, MesmerizationType, mezt, bool, isActivate)
{
    entityInfo, mezt, isActivate;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evCreatureTransformed,
    GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate)
{
    entityInfo, npcCode, isActivate;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evCreatureMutated,
    GameObjectInfo, entityInfo, NpcCode, npcCode, bool, isActivate)
{
    entityInfo, npcCode, isActivate;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evCreatureReleaseHidden,
    UnionEntityInfo, entityInfo)
{
    entityInfo;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evCreatureMoveSpeedChanged,
    GameObjectInfo, entityInfo, float32_t, currentSpeed)
{
    entityInfo, currentSpeed;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evCreatureFrenzied,
    GameObjectInfo, entityInfo, float32_t, currnetScale, bool, isActivate)
{
    entityInfo, currnetScale, isActivate;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evCreatureKnockbacked,
    GameObjectInfo, entityInfo, Position, position)
{
    entityInfo, position;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evCreatureKnockbackReleased,
    GameObjectInfo, entityInfo)
{
    entityInfo;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evCreatureDashing,
    GameObjectInfo, entityInfo, Position, position)
{
    entityInfo, position;
}


RECEIVE_SRPC_METHOD_1(BotPlayer, evPlayerGraveStoneStood,
    GraveStoneInfo, graveStoneInfo)
{
    graveStoneInfo;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evReviveEffected,
    GameObjectInfo, entityInfo, HitPoint, currentPoint, ObjectPosition, position)
{
    entityInfo, currentPoint, position;
}


RECEIVE_SRPC_METHOD_2(BotPlayer, evEffectAdded,
    GameObjectInfo, entityInfo, DebuffBuffEffectInfo, info)
{
    entityInfo, info;
}


RECEIVE_SRPC_METHOD_3(BotPlayer, evEffectRemoved,
    GameObjectInfo, entityInfo, DataCode, dataCode, bool, isCaster)
{
    entityInfo, dataCode, isCaster;
}


RECEIVE_SRPC_METHOD_3(BotPlayer,evEffectHit,
    GameObjectInfo, casterInfo, GameObjectInfo, targetInfo, DataCode, dataCode)
{
    casterInfo, targetInfo, dataCode;
}

// = rpc::DeviceRpc overriding

RECEIVE_SRPC_METHOD_1(BotPlayer, evDeviceDeactivated,
    ObjectId, deviceId)
{
    deviceId;
}


RECEIVE_SRPC_METHOD_0(BotPlayer, evSelfGraveStoneRemoved)
{

}
}} // namespace gideon { namespace bot {
