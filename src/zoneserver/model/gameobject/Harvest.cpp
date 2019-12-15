#include "ZoneServerPCH.h"
#include "Harvest.h"
#include "Entity.h"
#include "ability/Castable.h"
#include "ability/Questable.h"
#include "ability/Castnotificationable.h"
#include "status/StaticObjectStatus.h"
#include "../../controller/NpcController.h"
#include "../../world/WorldMap.h"
#include "../../world/region/MapRegion.h"
#include "../../service/time/GameTimer.h"
#include "../../service/spawn/SpawnService.h"
#include "../../service/distance/DistanceChecker.h"
#include "../../service/item/ItemDropService.h"
#include "../../controller/callback/HarvestCallback.h"
#include <gideon/cs/datatable/HarvestTable.h>


namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class RewardTask
 */
class RewardTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<RewardTask>
{
public:
    RewardTask(Entity& entity, Harvest& harvest) :
        entity_(entity),
        harvest_(harvest) {}

private:
    virtual void run() {
        const ErrorCode errorCode = harvest_.rewarded(entity_);
        if (isFailed(errorCode)) {
            // TODO: 에러 리턴??
        }
    }

private:
    Entity& entity_;
    Harvest& harvest_;
};

} // namespace


void Harvest::setNotUseTimer()
{
    notUseTimer_ = true;
}

bool Harvest::notUseTimer_ = false;

Harvest::Harvest(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller))
{
}


Harvest::~Harvest()
{
}


bool Harvest::initialize(ObjectId harvestId, HarvestCode harvestCode)
{
    harvestTemplate_ = HARVEST_TABLE->getHarvest(harvestCode);
    if (! harvestTemplate_) {
        return false;
    }

    {
        std::lock_guard<LockType> lock(getLock());

        if (!Parent::initialize(otHarvest, harvestId)) {
            return false;
        }

        harvestInfo_.objectType_ = otHarvest;
        harvestInfo_.objectId_ = harvestId;
        assert(isValidObjectId(harvestId));
        harvestInfo_.maxCount_ = static_cast<uint8_t>(
            esut::random(harvestTemplate_->min_harvest_count(),
                harvestTemplate_->max_harvest_count()));
        harvestInfo_.count_ = harvestInfo_.maxCount_;

        UnionEntityInfo& info = getUnionEntityInfo_i();
        MoreHarvestInfo& moreHarvestInfo = info.asHarvestInfo();
        moreHarvestInfo.harvestCode_ = harvestCode;
        moreHarvestInfo.currentCount_ = harvestInfo_.count_;
        static_cast<HarvestInfo&>(moreHarvestInfo) = harvestInfo_;
    }
    return true;
}


void Harvest::finalize()
{
    cancelDespawn();

    reset();

    Parent::finalize();
}


ErrorCode Harvest::respawn(WorldMap& worldMap)
{
    if (notUseTimer_) {
        return ecOk; // for Test
    }

    reset();

    return Parent::respawn(worldMap);
}


void Harvest::reserveDespawn()
{
    if (notUseTimer_) {
        return;
    }

    sne::base::Future::Ref despawnHarvestTask = despawnTask_.lock();
    if (despawnHarvestTask.get() != nullptr) {
        return;
    }

    despawnTask_ = SPAWN_SERVICE->scheduleDespawn(*this, harvestTemplate_->live_mil_sec());    
}


bool Harvest::shouldDespawn() const
{
    return harvestInfo_.count_ <= 0;
}


ErrorCode Harvest::startHarvest(Entity& entity)
{
    std::lock_guard<LockType> lock(getLock());

    if (harvestInfo_.count_ <= 0) {
        return ecHarvestNotHarvestState;
    }

    if (! DISTANCE_CHECKER->checkHarvestDistance(getPosition(), entity.getPosition())) {
        return ecHarvestTargetTooFar;
    }

    cancelDespawn();
    reserveDespawn();

    GameTime interval = harvestTemplate_->harvest_mil_sec();
    if (interval <= 0) {
        return rewarded(entity);
    }

    Castable* castable = entity.queryCastable();
    CastNotificationable* notificationable = entity.queryCastNotificationable();
    if (! castable & ! notificationable) {
        return ecServerInternalError;
    }

    auto task = std::make_unique<RewardTask>(entity, *this);
    castable->startCasting(harvestTemplate_->harvest_code(), false, std::move(task), interval);
    StartCastResultInfo startCastInfo;
    startCastInfo.set(uctHavest, entity.getGameObjectInfo(), getGameObjectInfo());
    notificationable->notifyStartCasting(startCastInfo);
    return ecOk;
}


ErrorCode Harvest::rewarded(Entity& entity)
{
    BaseItemInfos baseItems;
    QuestItemInfos questItems;
    
    Castable* castable = entity.queryCastable();
    if (castable) {
        castable->releaseCastState(false);
    }


    CastNotificationable* notificationable = entity.queryCastNotificationable();
    if (notificationable) {
        CompleteCastResultInfo comleteInfo;
        comleteInfo.set(entity.getGameObjectInfo(), getGameObjectInfo(),
            harvestInfo_.count_ > 0 ? harvestInfo_.count_ - 1 : harvestInfo_.count_);
        notificationable->notifyCompleteCasting(comleteInfo);
    }

    {
        std::lock_guard<LockType> lock(getLock());

        if (harvestInfo_.count_ > 0) {
            UnionEntityInfo& info = getUnionEntityInfo_i();
            MoreHarvestInfo& moreHarvestInfo = info.asHarvestInfo();        
            moreHarvestInfo.currentCount_ = harvestInfo_.count_;
            LootInvenItemInfoMap lootInvenItemInfoMap;
            ItemDropService::fillItemDropInfo(lootInvenItemInfoMap, harvestTemplate_->item_drop_id());
            for (LootInvenItemInfoMap::value_type& value : lootInvenItemInfoMap) {
                LootInvenItemInfo& itemInfo = value.second;
                CodeType codeType = getCodeType(itemInfo.baseItemInfo_.itemCode_);
                if (ctElement == codeType) {
                    baseItems.push_back(itemInfo.baseItemInfo_);
                }
                else if (ctQuestItem == codeType) {
                    Questable* questable = entity.queryQuestable();
                    if (questable) {
                        QuestCode questCode = invalidQuestCode;
                        const ErrorCode errorCode = questable->canAddItem(questCode, itemInfo.baseItemInfo_.itemCode_);
                        if (isFailed(errorCode)) {
                            continue;
                        }
                        questItems.push_back(QuestItemInfo(invalidObjectId, itemInfo.baseItemInfo_.itemCode_,
                            0, itemInfo.baseItemInfo_.count_, questCode));
                    }
                }
                else {
                    assert(false);
                    continue;
                }
            }
            --harvestInfo_.count_;
        }        
    }

    gc::HarvestCallback* callback = entity.getController().queryHarvestCallback();
    if (callback) {
        callback->rewarded(getObjectId(), baseItems, questItems);
    }

    if (shouldDespawn()) {
        despawn();        
    }

    return ecOk;
}


void Harvest::reset()
{
    std::lock_guard<LockType> lock(getLock());

    harvestInfo_.maxCount_ = static_cast<uint8_t>(esut::random(harvestTemplate_->min_harvest_count(), harvestTemplate_->max_harvest_count()));
    harvestInfo_.count_ = harvestInfo_.maxCount_;
    UnionEntityInfo& info = getUnionEntityInfo_i();
    MoreHarvestInfo& moreHarvestInfo = info.asHarvestInfo();
    moreHarvestInfo.currentCount_ = harvestInfo_.count_;
    static_cast<HarvestInfo&>(moreHarvestInfo) = harvestInfo_;
}


void Harvest::cancelDespawn()
{
    if (notUseTimer_) {
        return;
    }

    sne::base::Future::Ref despawnHarvestTask = despawnTask_.lock();
    if (! despawnHarvestTask.get()) {
        return;
    }

    despawnHarvestTask->cancel();
    despawnHarvestTask.reset();
}


// = StaticObject overriding

DataCode Harvest::getEntityCode() const
{
    return harvestTemplate_->harvest_code();
}

// = Positionable overriding

void Harvest::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);

    harvestInfo_.position_ = position;
}


void Harvest::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);

    harvestInfo_.position_.heading_ = heading;
}


WorldPosition Harvest::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(harvestInfo_.position_, worldMap->getMapCode());
}


}}} // namespace gideon { namespace zoneserver { namespace go {
