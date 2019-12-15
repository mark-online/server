#include "ZoneServerPCH.h"
#include "Device.h"
#include "EntityEvent.h"
#include "ability/Castable.h"
#include "ability/Questable.h"
#include "ability/Inventoryable.h"
#include "ability/Castnotificationable.h"
#include "ability/impl/KnowableAbility.h"
#include "ability/impl/SkillCastableAbility.h"
#include "skilleffect/StaticObjectEffectScriptApplier.h"
#include "status/StaticObjectStatus.h"
#include "../../controller/callback/DeviceCallback.h"
#include "../../service/spawn/SpawnService.h"
#include "../../service/distance/DistanceChecker.h"
#include "../../world/WorldMap.h"
#include "../../controller/EntityController.h"
#include "../../service/time/GameTimer.h"
#include "../../ai/evt/EventTrigger.h"
#include "../quest/QuestRepository.h"
#include "../item/Inventory.h"
#include "../item/QuestInventory.h"
#include <gideon/3d/3d.h>
#include "sne/base/concurrent/TaskScheduler.h"

namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class ActivateTask
 */
class ActivateTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<ActivateTask>
{
public:
    ActivateTask(Entity& activator, Device& device) :
        activator_(activator),
        device_(device) {}

private:
    virtual void run() {
        const ErrorCode errorCode = device_.activated(activator_);
        if (isFailed(errorCode)) {
            // TODO: 에러 리턴??
        }
    }

private:
    Entity& activator_;
    Device& device_;
};


/**
 * @class DeactivateTask
 */
class DeactivateTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<DeactivateTask>
{
public:
    DeactivateTask(Device& device) :
        device_(device) {}

private:
    virtual void run() {
        if (device_.isValid()) {
            device_.deactivate();
        }
    }

private:
    Device& device_;
};


/**
 * @class DeviceDeactivatedEvent
 */
class DeviceDeactivatedEvent : public EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<DeviceDeactivatedEvent>
{
public:
    DeviceDeactivatedEvent(ObjectId deviceObjectId) :
        deviceObjectId_(deviceObjectId) {}

private:
    virtual void call(Entity& entity) {
        gc::DeviceCallback* deviceCallback =
            entity.getController().queryDeviceCallback();
        if (deviceCallback != nullptr) {
            deviceCallback->deviceDeactivated(deviceObjectId_);
        }
    }

private:
    const ObjectId deviceObjectId_;
};

} // namespace

Device::Device(std::unique_ptr<gc::EntityController> controller) :
    StaticObject(std::move(controller))
{
    knowable_= std::make_unique<KnowableAbility>(*this);
}


Device::~Device()
{
}


bool Device::initialize(ObjectId deviceId, 
    const datatable::DeviceTemplate& deviceTemplate)
{
    assert(isValidObjectId(deviceId));

    std::lock_guard<LockType> lock(getLock());

    if (! Parent::initialize(otDevice, deviceId)) {
        return false;
    }

    deviceTemplate_ = &deviceTemplate;
    deviceCode_ = deviceTemplate_->getDeviceCode();

    DeviceInfo& deviceInfo = static_cast<DeviceInfo&>(getUnionEntityInfo_i().asDeviceInfo());
    deviceInfo.objectType_ = otDevice;
    deviceInfo.objectId_ = deviceId;
    deviceInfo.deviceCode_ = deviceCode_;
    deviceInfo.activated_ = false;

    eventTrigger_ = std::make_unique<ai::evt::EventTrigger>(*this);
    return true;
}


ErrorCode Device::activate(Entity& activator)
{
    const ErrorCode errorCode = checkActivatable(activator);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    const DeviceType deviceType = getDeviceType(deviceCode_);
    if (isQuestDevice(deviceType)) {            
        QuestRepository& questRepository = activator.queryQuestable()->getQuestRepository();
        const ErrorCode missionErrorCode = questRepository.checkActivationMission(deviceCode_);
        if (isFailed(missionErrorCode)) {
            return missionErrorCode;
        }
    }

    if (deviceTemplate_->getCastTime() <= 0) {
        return activated(activator);
    }

    Castable* castable = activator.queryCastable();
    CastNotificationable* notificationable = activator.queryCastNotificationable();
    if (! castable & ! notificationable) {
        return ecServerInternalError;
    }

    auto task = std::make_unique<ActivateTask>(activator, *this);
    castable->startCasting(deviceCode_, false, std::move(task), deviceTemplate_->getCastTime() * 1000);

    StartCastResultInfo startCastInfo;
    startCastInfo.set(uctDeviceActivation, activator.getGameObjectInfo(), getGameObjectInfo());
    notificationable->notifyStartCasting(startCastInfo);
    return ecOk;
}


void Device::deactivate()
{
    setActivateState(false);

    queryKnowable()->broadcast(
        std::make_shared<DeviceDeactivatedEvent>(getObjectId()));
}


ErrorCode Device::activated(Entity& activator)
{
    Castable* castable = activator.queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }

    CastNotificationable* notificationable = activator.queryCastNotificationable();
    if (notificationable != nullptr) {
        CompleteCastResultInfo completeInfo;
        completeInfo.set(uctDeviceActivation, activator.getGameObjectInfo(), getGameObjectInfo());
        notificationable->notifyCompleteCasting(completeInfo);
    }

    const DeviceType deviceType = getDeviceType(deviceCode_);
    if (isQuestDevice(deviceType)) {
        if (activator.isPlayer()) {
            QuestRepository& questRepository = activator.queryQuestable()->getQuestRepository();
            const ErrorCode errorCode = questRepository.completeActivationMission(deviceCode_);
            if (isFailed(errorCode)) {
                return errorCode;
            }
        }
        else {
            // by EventTrigger
        }
    }

    setActivateState(true);

    afterActivation(activator);

    eventTrigger_->deviceActivated();

    reserveDeactivation();
    return ecOk;
}


void Device::finalize()
{
    Parent::finalize();
}


ErrorCode Device::respawn(WorldMap& worldMap)
{
    setActivateState(false);

    return Parent::respawn(worldMap);
}


void Device::afterActivation(Entity& activator)
{
    if (! activator.isPlayer()) {
        return;
    }
    
    for (const datatable::DeviceCondition& condition : deviceTemplate_->getConditions()) {
        if (datatable::shouldDestroyItem(condition.condition_)) {
            const CodeType itemCodeType = getCodeType(condition.item_.itemCode_);
            if (itemCodeType == ctQuestItem) {
                activator.queryInventoryable()->getQuestInventory().removeItemsByItemCode(condition.item_.itemCode_,
                    uint8_t(condition.item_.itemCount_));
            }
            else {
                activator.queryInventoryable()->getInventory().useItemsByBaseItemInfo(
                    BaseItemInfo(condition.item_.itemCode_, uint8_t(condition.item_.itemCount_)));
            }
        }
    }
}


void Device::setActivateState(bool isActivated)
{
    std::lock_guard<LockType> lock(getLock());

    DeviceInfo& deviceInfo = static_cast<DeviceInfo&>(getUnionEntityInfo_i().asDeviceInfo());

    deviceInfo.activated_ = isActivated;
}


void Device::reserveDeactivation()
{
    const msec_t reactivateInterval = deviceTemplate_->getInfo().reactivate_interval();
    if (reactivateInterval <= 0) {
        return;
    }

    (void)TASK_SCHEDULER->schedule(std::make_unique<DeactivateTask>(*this), reactivateInterval);
}


ErrorCode Device::checkActivatable(Entity& activator) const
{
    if (! activator.isPlayer()) {
        return ecServerInternalError;
    }

    if (! DISTANCE_CHECKER->checkDeviceDistance(activator.getPosition(), getPosition())) {
        return ecQuestTargetTooFar;
    }

    for (const datatable::DeviceCondition& condition : deviceTemplate_->getConditions()) {
        ErrorCode errorCode = ecOk;
        if (datatable::shouldHaveItem(condition.condition_)) {
            errorCode = checkItemCondition(activator, condition);
        }
        else {
            assert(false && "TODO:");
            errorCode = ecServerInternalError;
        }
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }
    return ecOk;
}


ErrorCode Device::checkItemCondition(Entity& activator,
    const datatable::DeviceCondition& condition) const
{
    if (! activator.isPlayer()) {
        return ecOk; // by EventTrigger
    }
    
    const CodeType itemCodeType = getCodeType(condition.item_.itemCode_);
    if (itemCodeType == ctQuestItem) {
        if (! activator.queryInventoryable()->getQuestInventory().checkHasQuestItem(
            condition.item_.itemCode_, uint8_t(condition.item_.itemCount_))) {
            return ecDeviceCannotActivateBecauseHaveNoItem;
        }
    }
    else if (isItemType(itemCodeType)) {
        if (! activator.queryInventoryable()->getInventory().checkHasItemsByBaseItemInfo(
            BaseItemInfo(condition.item_.itemCode_, uint8_t(condition.item_.itemCount_)))) {
            return ecDeviceCannotActivateBecauseHaveNoItem;
        }
    }
    else {
        assert(false && "TODO: ??");
        return ecServerInternalError;
    }
    return ecOk;
}

// = StaticObject overriding

std::unique_ptr<EffectScriptApplier> Device::createEffectScriptApplier()
{
    return std::make_unique<StaticObjectEffectScriptApplier>(*this);
}


// = Positionable overriding

WorldPosition Device::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(getPosition(), worldMap->getMapCode());
}

// = SkillCastable overriding

ErrorCode Device::castTo(const GameObjectInfo& targetInfo, SkillCode skillCode)
{
    return skillCastableAbility_->castTo(targetInfo, skillCode);
}


ErrorCode Device::castAt(const Position& targetPosition, SkillCode skillCode)
{    
    return skillCastableAbility_->castAt(targetPosition, skillCode);
}


void Device::cancel(SkillCode skillCode)
{
    skillCastableAbility_->cancel(skillCode);
}


void Device::cancelConcentrationSkill(SkillCode /*skillCode*/)
{
}


void Device::cancelAll()
{
    skillCastableAbility_->cancelAll();
}


void Device::consumePoints(const Points& points)
{
    if (points.hp_ > 0) {
        getStaticObjectStatus().reduceHp(points.hp_);
    }
}


void Device::consumeMaterialItem(const BaseItemInfo& /*itemInfo*/)
{

}


ErrorCode Device::checkSkillCasting(SkillCode skillCode,
    const GameObjectInfo& targetInfo) const
{    
    return skillCastableAbility_->checkSkillCasting(skillCode, targetInfo);
}


ErrorCode Device::checkSkillCasting(SkillCode skillCode,
    const Position& targetPosition) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetPosition);
}


float32_t Device::getLongestSkillDistance() const
{
    return skillCastableAbility_->getLongestSkillDistance();
}


bool Device::isUsing(SkillCode skillCode) const
{
    return skillCastableAbility_->isUsing(skillCode);
}


bool Device::canCast(SkillCode skillCode) const
{
    return skillCastableAbility_->canCast(skillCode);
}

// = Tickable overriding

void Device::tick(GameTime diff)
{
    getEffectScriptApplier().tick();

    eventTrigger_->update(diff);
}

}}} // namespace gideon { namespace zoneserver { namespace go {
