#include "ZoneServerPCH.h"
#include "FieldDuel.h"
#include "DestoryDuelCallback.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/skilleffect/EntityEffectScriptApplier.h"
#include "../../model/gameobject/ability/Achievementable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/FieldDuelCallback.h"
#include "../../controller/callback/FieldDuelCallback.h"
#include "../../world/World.h"
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver {

namespace {


/**
 * @class StartFieldDualTask
 */
class StartFieldDualTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<StartFieldDualTask>
{
public:
	StartFieldDualTask(FieldDual& fieldDual) :
		fieldDual_(fieldDual)
	{
	}

private:
    virtual void run() {
		fieldDual_.start();
    }

private:
	FieldDual& fieldDual_;
};


/**
 * @class DestoryFieldDualTask
 */
class DestoryFieldDualTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<DestoryFieldDualTask>
{
public:
	DestoryFieldDualTask(FieldDual& fieldDual) :
		fieldDual_(fieldDual)
	{
	}

private:
    virtual void run() {
		fieldDual_.stop(fdrtCancelTimeout, invalidObjectId);
    }

private:
	FieldDual& fieldDual_;
};


/**
 * @class FieldDuelResultEvent
 */
class FieldDuelResultEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<FieldDuelResultEvent>
{
public:
    FieldDuelResultEvent(ObjectId winnerId, const Nickname& winner, ObjectId loserId, const Nickname& loser) :
        winnerId_(winnerId),
        winner_(winner),
        loserId_(loserId),
        loser_(loser) {}

private:
    virtual void call(go::Entity& entity) {
        gc::FieldDuelCallback* callback = entity.getController().queryFieldDuelCallback();
        if (callback != nullptr) {
            callback->fieldDeulResult(winnerId_, winner_, loserId_, loser_);
        }
    }

private:
    ObjectId winnerId_;
    Nickname winner_;
    ObjectId loserId_; 
    Nickname loser_;
};

} // namespace {


FieldDual::FieldDual(DestroyDuelCallback* callback, ObjectId duelId, 
    go::Entity& duealer1, go::Entity& duealer2) :
    callback_(callback),
    duelId_(duelId),
    duealer1_(duealer1),
    duealer2_(duealer2),
    startPosition_(duealer1_.getPosition()),
    isDestory_(false)
{
    ready();
}


FieldDual::~FieldDual()
{
}


bool FieldDual::shouldStopByDistance(go::Entity& duealer) const
{
    std::lock_guard<LockType> lock(lock_);

    if (! (duealer1_.isSame(duealer) || duealer2_.isSame(duealer))) {
        assert(false);
        return false;
    }

    const float32_t maxDistanceSq = 50.0f * 50.0f;
    float32_t  distanceToTargetSq = duealer.getSquaredLength(startPosition_);
    if (maxDistanceSq > 0) {
        if (distanceToTargetSq > maxDistanceSq) {
            return true;
        }
    }

    return false;
}


void FieldDual::start()
{
    std::lock_guard<LockType> lock(lock_);

    const GameTime stopInterval = duelStopTime * 1000;
    destroyTask_ = TASK_SCHEDULER->schedule(
        std::make_unique<DestoryFieldDualTask>(*this), stopInterval);

    duealer1_.getEffectScriptApplier().applyEffectScript(estSpawnProtection, 3000);
    duealer2_.getEffectScriptApplier().applyEffectScript(estSpawnProtection, 3000);
    
    gc::FieldDuelCallback* callback1 = duealer1_.getController().queryFieldDuelCallback();
    callback1->fieldDuelStarted();
    gc::FieldDuelCallback* callback2 = duealer2_.getController().queryFieldDuelCallback();
    callback2->fieldDuelStarted();

}

void FieldDual::stop(FieldDuelResultType type, ObjectId winnerId)
{
    std::lock_guard<LockType> lock(lock_);

    if (isDestory_) {
        return;
    }
    isDestory_ = true;
    sne::base::Future::Ref startTask = startTask_.lock();
    if (startTask.get() != nullptr) {
        startTask->cancel();
        startTask_.reset();
    }

    sne::base::Future::Ref destoryTask = destroyTask_.lock();
    if (destoryTask.get() != nullptr) {
        destoryTask->cancel();
        destroyTask_.reset();
    }

    
    duealer1_.getEffectScriptApplier().applyEffectScript(estSpawnProtection, 3000);
    duealer2_.getEffectScriptApplier().applyEffectScript(estSpawnProtection, 3000);

    gc::FieldDuelCallback* callback1 = duealer1_.getController().queryFieldDuelCallback();
    callback1->fieldDuelStopped(type);
    gc::FieldDuelCallback* callback2 = duealer2_.getController().queryFieldDuelCallback();
    callback2->fieldDuelStopped(type);
    
    if (winnerId != invalidObjectId) {
        go::Entity& winner = winnerId == duealer1_.getObjectId() ? duealer1_: duealer2_;
        go::Entity& loser = winnerId != duealer1_.getObjectId() ? duealer1_ : duealer2_;

        auto event = std::make_shared<FieldDuelResultEvent>(winner.getObjectId(), winner.getNickname(),
            loser.getObjectId(), loser.getNickname());
        WORLD->broadcast(event);

        winner.queryAchievementable()->updateAchievement(acmtPvPWin, &loser);
    }
    
    callback_->destory(duelId_);
}



void FieldDual::ready()
{
    const GameTime startInterval = duelReadyTime * 1000;
    startTask_ = TASK_SCHEDULER->schedule(std::make_unique<StartFieldDualTask>(*this), startInterval);

    Position ownerPosition = duealer1_.getPosition();
    Position targetPosition = duealer2_.getPosition();
    Position centerPosition;
    centerPosition.x_ = (ownerPosition.x_ + targetPosition.x_) / 2.0f;
    centerPosition.y_ = (ownerPosition.y_ + targetPosition.y_) / 2.0f;
    // TODO : 높이 수정
    centerPosition.z_ = ownerPosition.z_ > targetPosition.z_ ? ownerPosition.z_ : targetPosition.z_;
    
    gc::FieldDuelCallback* callback1 = duealer1_.getController().queryFieldDuelCallback();
    callback1->fieldDuelReadied(centerPosition);
    gc::FieldDuelCallback* callback2 = duealer2_.getController().queryFieldDuelCallback();
    callback2->fieldDuelReadied(centerPosition);

}


}} // namespace gideon { namespace zoneserver {
