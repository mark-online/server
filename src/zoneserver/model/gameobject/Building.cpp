#include "ZoneServerPCH.h"
#include "Building.h"
#include "Entity.h"
#include "ability/Guildable.h"
#include "building/BuildingFoundationState.h"
#include "building/BuildingBuildState.h"
#include "building/BuildingCompleteState.h"
#include "ability/Castable.h"
#include "ability/CastNotificationable.h"
#include "ability/Inventoryable.h"
#include "ability/Moneyable.h"
#include "ability/BuildingGuardable.h"
#include "../item/Inventory.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/BuildingStateCallback.h"
#include "../../ai/BrainFactory.h"
#include "../../ai/Brain.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../world/region/SpawnMapRegion.h"
#include "../../service/anchor/BuildingMarkCallback.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "../../ZoneService.h"
#include "../../helper/InventoryHelper.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/BuildingGuardTable.h>
#include <gideon/cs/datatable/BuildingGuardSellTable.h>
#include <gideon/3d/3d.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class BuildingLootingTask
 */
class BuildingLootingTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<BuildingLootingTask>
{
public:
    BuildingLootingTask(Entity& player, BuildingProductionInventoryable& building, ObjectId itemId, InvenType invenType) :
        player_(player),
        building_(building),
        itemId_(itemId),
        invenType_(invenType) {}

private:
    virtual void run() {
        const ErrorCode errorCode = building_.completeLooting(player_, itemId_, invenType_);
        if (isFailed(errorCode)) {
            // TODO: 에러 리턴??
        }
    }

private:
    Entity& player_;
    BuildingProductionInventoryable& building_;
    ObjectId itemId_;
    InvenType invenType_;
};

} // namespace


Building::Building(std::unique_ptr<gc::EntityController> controller,
    BuildingMarkCallback& markCallback) :
    Parent(std::move(controller)),
    markCallback_(markCallback)
{
}


Building::~Building()
{
}


void Building::finalize()
{
    markCallback_.removeBuildingMarkInfo(buildingInfo_.objectId_);
    {
        std::lock_guard<LockType> lock(getLock());

        buildingTemplate_ = nullptr;
        buildingInfo_.reset();
    }

    Parent::finalize();
}


ErrorCode Building::downBuildTime(sec_t downSec)
{
    std::lock_guard<LockType> lock(getLock());

    if (buildingInfo_.isBuildingState()) {
        buildingInfo_.startBuildTime_ -= downSec;
        return ecOk;
    }

    return ecCheatInvalidState;
}


ErrorCode Building::addBuildingMaterials()
{
    if (! (buildingInfo_.isFoundationState() || buildingInfo_.isWreckageState())) {
        return ecCheatInvalidState;
    }

    BaseItemInfos materials = buildingTemplate_->getMaterials();
    BaseItemInfos addItems;
    for (BaseItemInfo& material : materials) {
        uint8_t itemStackCount = getStackItemCount(material.itemCode_);
        while (material.count_ != 0) {
            if (material.count_ <= itemStackCount) {
                addItems.push_back(material);
                material.count_ = 0;
            }
            else {
                addItems.push_back(BaseItemInfo(material.itemCode_, itemStackCount));
                material.count_ -= itemStackCount;
            }
        }
    }
    {
        std::lock_guard<LockType> lock(getLock());

        if (! fundationInventory_.get()) {
            return ecServerInternalError;
        }

        if (! fundationInventory_->canAddItemByBaseItemInfos(addItems)) {
            return ecAnchorFullInventory;
        }
        for (BaseItemInfo& material : addItems) {
            fundationInventory_->addItem(createAddItemInfoByBaseItemInfo(material));
        }
    }
    return ecOk;
}


bool Building::initialize(DataCode dataCode, ObjectId objectId, ObjectId linkBuildingId,
    const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLock());

    if (! Parent::initialize(otBuilding, objectId)) {
        return false;
    }

    buildingTemplate_ = BUILDING_TABLE->getBuildingTemplate(dataCode);
    if (!buildingTemplate_) {
        return false;
    }
    saveTime_ = getTime();
    fillFoundationInfo(dataCode, objectId, position);
    buildingInfo_.createTime_ = getTime();
    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    entityInfo.objectType_ = getObjectType();
    MoreBuildingInfo& moreBuildingInfo = entityInfo.asBuildingInfo();
    static_cast<BaseBuildingInfo&>(moreBuildingInfo) = buildingInfo_;
    linkCoreBuildingId_ = linkBuildingId;
    initInventories();

    gc::BuildingStateCallback* callback = getController().queryBuildingStateCallback();
    if (callback) {
        callback->buildingCreated();
    }
    BuildingMarkInfo markInfo(buildingInfo_.objectId_, buildingInfo_.buildingCode_, buildingInfo_.position_);
    markCallback_.addBuildingMarkInfo(buildingInfo_.objectId_, markInfo);

    initBuildingState();
    initBuildingBrain();
    return true;
}


bool Building::initialize(ObjectId linkBuildingId, const BuildingInfo& buildingInfo)
{
    std::lock_guard<LockType> lock(getLock());

    if (! Parent::initialize(otBuilding, buildingInfo.objectId_)) {
        return false;
    }

    buildingTemplate_ = BUILDING_TABLE->getBuildingTemplate(buildingInfo.buildingCode_);
    if (! buildingTemplate_) {
        return false;
    }

    buildingInfo_ = buildingInfo;
    buildingInfo_.objectType_ = otBuilding;
    bool isChangeBuildingState = false;
    const sec_t now = getTime();
    if (buildingInfo_.isBuildingState()) {
        const datatable::BuildingFoundationInfo& fundationInfo = buildingTemplate_->getBuildingFoundationInfo();              
        const sec_t completeTime = (now - buildingInfo_.startBuildTime_);
        if (completeTime >= fundationInfo.buildTime_) {
            buildingInfo_.state_ = bstComplete;
            isChangeBuildingState = true;        
        }
    }

    DominionType dominionType = buildingTemplate_->getDominionType();
    if (dominionType != dtNone && buildingInfo_.ownerInfo_.isValid()) {
        MoreBuildingDominionInfo info(buildingInfo_.objectId_, buildingInfo_.position_,
            buildingInfo.buildingCode_, buildingInfo.ownerInfo_.guildInfo_, L"",
            true, buildingTemplate_->canTeleport(), dominionType);
        ZoneCommunityServerProxy& communityServerProxy =
            ZONE_SERVICE->getCommunityServerProxy();
        communityServerProxy.z2m_addDominion(
            WORLD->getGlobalWorldMap().getMapCode(), info);
    }

    initInventories();

    if (buildingInfo_.isCompleteState()) {
        const datatable::BaseAnchorInfo& baseAnchorInfo_ = buildingTemplate_->getBaseAnchorInfo();        
        buildingInfo_.points_.maxHp_ = baseAnchorInfo_.completeHp_;
        if (isChangeBuildingState) {      
            buildingInfo_.points_.hp_ = baseAnchorInfo_.completeHp_;
        }        
    }
    else if (buildingInfo_.isBuildingState() || buildingInfo_.isFoundationState() || buildingInfo_.isWreckageState()) {
        const datatable::BuildingFoundationInfo& fundationInfo = buildingTemplate_->getBuildingFoundationInfo();
        buildingInfo_.points_.maxHp_ = fundationInfo.foundationHp_;        
    }

    BuildingMarkInfo markInfo(buildingInfo_.objectId_, buildingInfo_.buildingCode_, buildingInfo_.position_);
    markCallback_.addBuildingMarkInfo(buildingInfo_.objectId_, markInfo);

    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    entityInfo.objectType_ = getObjectType();
    MoreBuildingInfo& moreBuildingInfo = entityInfo.asBuildingInfo();
    static_cast<BaseBuildingInfo&>(moreBuildingInfo) = buildingInfo_;
    linkCoreBuildingId_ = linkBuildingId;

    initBuildingState();
    initBuildingBrain();
    return true;
}


bool Building::initializeGuard()
{
    ObjectIdSet removeGuards;
    {
        std::lock_guard<LockType> lock(getLock());

        sec_t now = getTime();
        BuildingGuardInfos::iterator pos = buildingInfo_.buildingGuardInfos_.begin();
        while (pos != buildingInfo_.buildingGuardInfos_.end()) {
            BuildingGuardInfo& buildingGuardInfo = (*pos);
            if (buildingGuardInfo.expireTime_ < now) {
                removeGuards.insert(buildingGuardInfo.objectId_);
                pos = buildingInfo_.buildingGuardInfos_.erase(pos);
                continue;
            }
            else {
                const msec_t lifeTime = static_cast<msec_t>((now - buildingGuardInfo.expireTime_) * 1000);
                const ErrorCode errorCode = spawnBuildingGuard(buildingGuardInfo.objectId_,
                    buildingGuardInfo.npcCode_, lifeTime, buildingGuardInfo.position_);
                if (isFailed(errorCode)) {
                    removeGuards.insert(buildingGuardInfo.objectId_);
                    pos = buildingInfo_.buildingGuardInfos_.erase(pos);
                    continue;
                }
            }
            ++pos;
        }

    }

    for (ObjectId id : removeGuards) {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveBuildingGuard(id);
    }
    return true;
}


ErrorCode Building::fillBindRecallInfo(BindRecallInfo& bindRecallInfo, ObjectId playerId)
{
    if (! isCompleteState()) {
        return ecAnchorNotAddBindRecallNotComplete;
    }

    if (! isPrivateOwnerType(buildingInfo_.ownerInfo_.ownerType_)) {
        return ecAnchorPrivateOwnerAddBindRecall;
    }

    if (getOwnerId() != playerId) {
        return ecAnchorNotOwner;
    }

    if (! buildingTemplate_->canBindRecall()) {
        return ecAnchorCanNotBindRecallAnchor;
    }
    
    bindRecallInfo.dataCode_ = buildingInfo_.buildingCode_;
    bindRecallInfo.linkId_ = buildingInfo_.objectId_;

    bindRecallInfo.wordlPosition_ = getWorldPosition();
    Vector2 offset = getDirection(getWorldPosition().heading_) * (getModelingRadiusSize() + 1.0f);
    bindRecallInfo.wordlPosition_.x_ += offset.x;
    bindRecallInfo.wordlPosition_.y_ += offset.y;

    return ecOk;
}


void Building::saveHitPoint()
{
    getController().queryBuildingStateCallback()->buildCurrentStateSaved();
}



SlotId Building::getItemSlotId(InvenType invenType, ObjectId itemId) const
{
    const Inventory* inventory = getInventory(invenType);
    if (! inventory) {
        return invalidSlotId;
    }
    const ItemInfo itemInfo = inventory->getItemInfo(itemId);
    if (! itemInfo.isValid()) {
        return invalidSlotId;
    }
    return itemInfo.slotId_;
}


ErrorCode Building::toggleDoor(bool& isOpenDoor, go::Entity& player)
{
    std::lock_guard<LockType> lock(getLock());

    if (! buildingTemplate_->canToggleDoor()) {
        return ecAnchorHasNotDoor;
    }

    if (! buildingInfo_.isCompleteState()) {
        return ecAnchorNotCompleteNotToggleDoor;
    }

    if (! isEnoughBuildingActionGrade(player)) {
        return ecAnchorToggleDoorNotEnoughGrade;
    }


    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    MoreBuildingInfo& moreBuildingInfo = entityInfo.asBuildingInfo();
    moreBuildingInfo.isOpen_ = ! moreBuildingInfo.isOpen_;
    isOpenDoor = moreBuildingInfo.isOpen_;
    return ecOk;
}


void Building::addLinkBuilding(ObjectId buildingId)
{
    std::lock_guard<LockType> lock(getLock());

    if (isCoreBuilding()) {
        linkBuildingIds_.insert(buildingId);
    }
}


void Building::removeLinkBuilding(ObjectId buildingId)
{
    std::lock_guard<LockType> lock(getLock());

    if (isCoreBuilding()) {
        linkBuildingIds_.erase(buildingId);
    }
}

// =  Buildable overriding

ErrorCode Building::build(Entity& player)
{
    std::lock_guard<LockType> lock(getLock());

    if (! currentState_->shouldChangeNextState(&player)) {
        return ecAnchorNotEnoughBuildCondition;
    }

    currentState_->changeNextState(&player);
    currentState_ = buildingStateMap_[currentState_->getNextState()];
    return ecOk;
}


void Building::hiringNpcDespawned(ObjectId mercenaryId)
{
    {
        std::lock_guard<LockType> lock(getLock());

        BuildingGuardInfos& buildingGuardInfos = buildingInfo_.buildingGuardInfos_;
        BuildingGuardInfos::iterator pos = buildingGuardInfos.begin();
        for (; pos != buildingGuardInfos.end(); ++pos) {
            BuildingGuardInfo& buildingGuardInfo = (*pos);
            if (buildingGuardInfo.objectId_ == mercenaryId) {
                buildingGuardInfos.erase(pos);
                break;
            }
        }
    }

    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveBuildingGuard(mercenaryId);
    }
}


ObjectId Building::getOwnerId() const
{
    std::lock_guard<LockType> lock(getLock());

    return buildingInfo_.ownerInfo_.playerOwnerInfo_.objectId_;
}


GuildId Building::getOwnerGuildId() const
{
    std::lock_guard<LockType> lock(getLock());

    return buildingInfo_.ownerInfo_.guildInfo_.guildId_;
}

// = Bankable overriding

ErrorCode Building::canBankable(go::Entity& player) const
{
    std::lock_guard<LockType> lock(getLock());
    
    if (! isEnoughBuildingActionGrade(player)) {
        return ecAnchorBankNotEnoughGrade;
    }

    if (! isActiveAbillity()) {
        return ecAnchorNotActivateBank;
    }

    if (! buildingTemplate_->canBankFunction()) {
        return ecAnchorNotBankAbillity;
    }

    return ecOk;
}

// = SelectRecipeProductionable overriding

ErrorCode Building::startRecipeProduction(go::Entity& player, RecipeCode recipeCode, uint8_t count)
{
    std::lock_guard<LockType> lock(getLock());

    if (! isEnoughBuildingActionGrade(player)) {
        return ecAnchorBankNotEnoughGrade;
    }
    return currentState_->startRecipeProduction(recipeCode, count);
}

// = BuildingProductionInventoryable overriding

ErrorCode Building::startLooting(Entity& player, ObjectId itemId, InvenType invenType)
{
    GameTime lootingDelay = 0;
    DataCode itemCode = invalidDataCode;
    {
        std::lock_guard<LockType> lock(getLock());
        
        if (! isEnoughBuildingActionGrade(player)) {
            return ecAnchorNotUsableGradeInventory;
        }

        Inventory* inventory = getInventory(invenType);
        if (! inventory) {
            return ecAnchorNotExistInventory;
        }

        const ItemInfo& itemInfo = inventory->getItemInfo(itemId);
        if (! itemInfo.isValid()) {
            return ecInventoryItemNotFound;
        }
        itemCode = itemInfo.itemCode_;
        lootingDelay = getLootingDelay(liitItem, itemInfo.itemCode_);
    }


    go::Castable* castable = player.queryCastable();
    go::CastNotificationable* notificationable = player.queryCastNotificationable();
    if (! castable & ! notificationable) {
        return ecServerInternalError;
    }

    BuildingProductionInventoryable& inventoryable = *this;
    auto task = std::make_unique<BuildingLootingTask>(player, inventoryable, itemId, invenType);
    castable->startCasting(makeCastStateCode(cstBuildingItemLooting), false,
        std::move(task), lootingDelay);
    StartCastResultInfo startCastInfo;
    startCastInfo.set(uctBuildingItemLooting, player.getGameObjectInfo(), getGameObjectInfo(), itemCode);
    notificationable->notifyStartCasting(startCastInfo);

    return ecOk;
}


ErrorCode Building::completeLooting(Entity& player, ObjectId itemId, InvenType invenType)
{
    Castable* castable = player.queryCastable();
    if (castable) {
        castable->releaseCastState(false);    
    }    

    ItemInfo itemInfo;
    {
        std::lock_guard<LockType> lock(getLock());

        Inventory* inventory = getInventory(invenType);
        if (! inventory) {
            return ecInventoryInvalidInvenType;
        }

        itemInfo = inventory->getItemInfo(itemId);
        if (! itemInfo.isValid()) {
            return ecInventoryItemNotFound;
        }
        inventory->removeItem(itemInfo.itemId_);
    }

    ErrorCode errorCode = player.queryInventoryable()->getInventory().addItem(createAddItemInfoByItemInfo(itemInfo));
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::CastNotificationable* notificationable = player.queryCastNotificationable();
    CompleteCastResultInfo comleteInfo;
    comleteInfo.set(itemInfo.itemId_, player.getGameObjectInfo(), getGameObjectInfo());
    notificationable->notifyCompleteCasting(comleteInfo);

    return ecOk;
}

// = OutsideInventoryable overriding

Inventory* Building::getInventory(InvenType invenType) 
{
    std::lock_guard<LockType> lock(getLock());

    if (invenType == itBuilding) {
        return buildingInventory_.get();
    }
    if (invenType == itMercenary) {
        return mercenaryInventory_.get();
    }
    if (invenType == itBuildingProduction) {
        return productionInventory_.get();
    }
    if (invenType == itBuildingSelectProduction) {
        return selectProductionInventory_.get();
    }

    if (buildingInfo_.isFoundationState() || buildingInfo_.isWreckageState()) {
        return fundationInventory_.get();
    }

    return nullptr;
}


const Inventory* Building::getInventory(InvenType invenType) const 
{
    std::lock_guard<LockType> lock(getLock());

    if (invenType == itBuilding) {
        return buildingInventory_.get();
    }
    if (invenType == itMercenary) {
        return mercenaryInventory_.get();
    }
    if (invenType == itBuildingProduction) {
        return productionInventory_.get();
    }
    if (invenType == itBuildingSelectProduction) {
        return selectProductionInventory_.get();
    }
    if (buildingInfo_.isFoundationState() || buildingInfo_.isWreckageState()) {
        return fundationInventory_.get();
    }
    return nullptr;
}


ErrorCode Building::checkInventoryUseable(InvenType invenType, go::Entity& player) const
{
    std::lock_guard<LockType> lock(getLock());

    const Inventory* inventory = getInventory(invenType);
    if (! inventory) {
        return ecAnchorNotSupportInventory;
    }

    if (buildingInfo_.isFoundationState() || buildingInfo_.isWreckageState()) {
        if (invenType == itBuildingFoundation) {
            return ecOk;
        }
        return ecAnchorNotSupportInventory;
    }

    if (buildingInfo_.isCompleteState()) {
        if (! isEnoughBuildingActionGrade(player)) {
            return ecAnchorNotUsableGradeInventory;
        }

        if (invenType == itBuilding) {
            return ecOk;
        }

        if (isActiveAbillity()) {
            return ecOk;
        }
    }

    return ecAnchorNotUseInventoryByBuildingState;   
}


ErrorCode Building::canAddItem(InvenType invenType, DataCode itemCode) const
{
    std::lock_guard<LockType> lock(getLock());

    if (invenType == itBuildingProduction || invenType == itBuildingSelectProduction) {
        return ecAnchorNotAddInventory;
    }

    if (invenType == itBuilding && (buildingInfo_.isCompleteState() || buildingInfo_.isBuildingState())) {
        if (buildingTemplate_->isFuelItem(itemCode) || buildingTemplate_->isInvincibleItem(itemCode)) {
            return ecOk;
        }
        return ecAnchorNotAddItem;
    }

    if (invenType == itBuildingFoundation && (buildingInfo_.isFoundationState() || buildingInfo_.isWreckageState())) {
        if (buildingTemplate_->isMaterials(itemCode)) {
            return ecOk;
        }
        return ecAnchorNotAddItem;
    }

    if (invenType == itBuildingFoundation) {
        if (! buildingInfo_.isCompleteState()) {
            return ecOk;
        }                
    }
    if (invenType == itMercenary) {
        if (isActiveAbillity()) {
            return ecOk;
        }
    }

    return ecAnchorNotUseInventoryByBuildingState;
}


ErrorCode Building::canOutItem(InvenType invenType, go::Entity& /*player*/) const
{
    std::lock_guard<LockType> lock(getLock());
    
    if (buildingInfo_.isCompleteState() && 
        (invenType == itBuilding && buildingTemplate_->getBaseAnchorInfo().isOwnerCommon())) {        
        return ecAnchorCommonBuildingNotOutItem;
    }

    return ecOk;
}


bool Building::isActiveAbillity() const
{
    std::lock_guard<LockType> lock(getLock());

    return currentState_->isActiveAbillity();
}


void Building::fillFoundationInfo(DataCode dataCode, ObjectId objectId,
    const ObjectPosition& position)
{
    const datatable::BuildingFoundationInfo& fundationInfo = buildingTemplate_->getBuildingFoundationInfo();
        
    buildingInfo_.objectType_ = otBuilding;
    buildingInfo_.objectId_ = objectId;
    buildingInfo_.state_ = bstFoundation;
    buildingInfo_.buildingCode_ = dataCode;
    buildingInfo_.position_ = position;
    buildingInfo_.points_.maxHp_ = fundationInfo.foundationHp_;
    buildingInfo_.points_.hp_ = fundationInfo.foundationHp_;
    buildingInfo_.startBuildTime_ = 0;
    buildingInfo_.isInvincible_ = false;
    buildingInfo_.fundationExpireTime_ = 0;
    buildingInfo_.buildingInventory_.defaultInvenCount_ = buildingTemplate_->getBuildingInventoryCount();
    buildingInfo_.mercenaryInventory_.defaultInvenCount_ = buildingTemplate_->getMercenaryInventoryCount();
    if (fundationInfo.foundationLiveTime_  > 0) {
        buildingInfo_.fundationExpireTime_=  getTime() + fundationInfo.foundationLiveTime_;
    }
    
    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    entityInfo.objectType_ = getObjectType();
    MoreBuildingInfo& moreBuildingInfo = entityInfo.asBuildingInfo();
    static_cast<BaseBuildingInfo&>(moreBuildingInfo) = buildingInfo_;
}


void Building::initBuildingState()
{
    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    buildingStateMap_.insert(BuildingStateMap::value_type(bstFoundation, 
        new BuildingFoundationState(*this, *buildingTemplate_, buildingInfo_, entityInfo.asBuildingInfo(),
        fundationInventory_.get())));
    buildingStateMap_.insert(BuildingStateMap::value_type(bstBuilding, 
        new BuildingBuildState(*this, *buildingTemplate_, buildingInfo_, entityInfo.asBuildingInfo(),
        fundationInventory_.get())));
    buildingStateMap_.insert(BuildingStateMap::value_type(bstComplete, 
        new BuildingCompleteState(*this, *buildingTemplate_, buildingInfo_, entityInfo.asBuildingInfo(),
        fundationInventory_.get())));
    buildingStateMap_.insert(BuildingStateMap::value_type(bstWreckage, 
        new BuildingWreckageState(*this, *buildingTemplate_, buildingInfo_, entityInfo.asBuildingInfo(),
        fundationInventory_.get())));
    currentState_ = buildingStateMap_[buildingInfo_.state_];
    currentState_->reInitialzie();
}


void Building::initBuildingBrain()
{
    brain_ = ai::BrainFactory::createBrain(*this);
    brain_->initialize();
}


void Building::fillBuildingInfo(sec_t& expireIncibleCooltime,
    BuildingOwnerInfo& ownerInfo, BuildingItemTaskInfos& consumeTaskInfos,
    BuildingItemTaskInfos& resouceTaskInfos, BuildingItemTaskInfos& selectProductionTaskInfos)
{
    std::lock_guard<LockType> lock(getLock());

    ownerInfo = buildingInfo_.ownerInfo_;
    currentState_->fillStateInfo(expireIncibleCooltime, consumeTaskInfos, resouceTaskInfos, selectProductionTaskInfos);
}


void Building::handleExpiredTasks()
{
    if (currentState_->shouldChangeNextState(nullptr)) {
        currentState_->changeNextState(nullptr);
        currentState_ = buildingStateMap_[currentState_->getNextState()];
        currentState_->reInitialzie();
    }
    else if (currentState_->shouldChangePreState()) {
        currentState_->changePreState();
        currentState_ = buildingStateMap_[currentState_->getPreState()];
        currentState_->reInitialzie();
    }
    currentState_->handleExpiredTasks();

    const sec_t saveTime = 60 * 60;
    const sec_t now = getTime();
    if (saveTime_ + saveTime < now) {
        saveHitPoint();
        saveTime_ = now;
    }
}


ErrorCode Building::canTeleport(go::Entity& player) const
{
    std::lock_guard<LockType> lock(getLock());

    if (! buildingTemplate_->canTeleport()) {
        return ecAnchorNotTeleportBuilding;
    }
    if (! (getOwnerGuildId() != player.queryGuildable()->getGuildId())) {
        return ecAnchorTeleportNotEnoughGrade;
    }
    if (currentState_->isActiveAbillity()) {
        return ecAnchorTeleportNotActivate;
    }

    return ecOk;
}


ErrorCode Building::buyBuildingGuard(Entity& player, BuildingGuardCode code, uint8_t rentDay)
{
    {
        std::lock_guard<LockType> lock(getLock());

        if (! isActiveAbillity()) {
            return ecAnchorGuardNotActivate;
        }

        if (! isEnoughBuildingActionGrade(player)) {
            return ecAnchorGuardNotEnoughGrade;
        }
    }

    if (! buildingTemplate_->canMercenaryStore()) {
        return ecAnchorNotSellGuard;
    }

    const datatable::GuardSellList* sellIst = 
        BUILDING_GUARD_SELL_TABLE->getSellList(buildingInfo_.buildingCode_);
    if (! sellIst) {
        return ecStoreNotFindSellItemList;
    }

    if (sellIst->find(code) == sellIst->end()) {
        return ecStoreNotFindSellItemList;
    }

    const gdt::building_guard_t* guard = BUILDING_GUARD_TABLE->getBuildingGuard(code);
    if (! guard) {
        return ecServerInternalError;
    }

    GameMoney rentCost = static_cast<GameMoney>(guard->day_cost() * rentDay);
    if (player.queryMoneyable()->getGameMoney() < rentCost) {
        return ecStoreNotEnoughGameMoney;
    }

    BaseItemInfo itemInfo(code, 1);
    if (! player.queryInventoryable()->getInventory().canAddItemByBaseItemInfo(itemInfo)) {
        return ecInventoryIsFull;
    }

    player.queryMoneyable()->downGameMoney(rentCost);
    player.queryInventoryable()->getInventory().addMercenaryItem(itemInfo, rentDay);

    return ecOk;
}


ErrorCode Building::spawnBuildingGuard(Entity& player, ObjectId mercenaryId, const ObjectPosition& position){

    // TODO: 완성체크 및 주인검사 및 명수
    ItemInfo itemInfo = player.queryInventoryable()->getInventory().getItemInfo(mercenaryId);
    if (! itemInfo.isValid()) {
        return ecInventoryItemNotFound;
    }

    if (buildingTemplate_->getMercenaryCount() <= buildingInfo_.buildingGuardInfos_.size()) {
        return ecAnchorOverGuard;
    }

    const gdt::building_guard_t* guard = BUILDING_GUARD_TABLE->getBuildingGuard(itemInfo.itemCode_);
    if (! guard) {
        return ecStoreNotFindSellItemList;
    }

    sec_t expireSec = itemInfo.expireTime_ * (60 * 60 * 24);
    sec_t now = getTime();
    msec_t lifeTime = static_cast<msec_t>(expireSec * 1000);
    BuildingGuardInfo info(guard->npc_code(), itemInfo.itemId_, expireSec + now, position);
    {
        std::lock_guard<LockType> lock(getLock());

        if (! isActiveAbillity()) {
            return ecAnchorGuardNotActivate;
        }

        if (! isEnoughBuildingActionGrade(player)) {
            return ecAnchorGuardNotEnoughGrade;
        }

        ErrorCode errorCode = spawnBuildingGuard(itemInfo.itemId_, guard->npc_code(), lifeTime, position);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        buildingInfo_.buildingGuardInfos_.push_back(info);
    }

    player.queryInventoryable()->getInventory().useItemsByItemId(mercenaryId, 1);
    

    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncAddBuildingGuard(getObjectId(), info);
    return ecOk;
}


sec_t Building::getExpireTime() const 
{   
    sec_t expireTime = 0;
    sec_t liveTime = buildingTemplate_->getBaseAnchorInfo().liveTime_;
    if (liveTime > 0) {         
        expireTime = buildingInfo_.createTime_ + liveTime;
    }
    return buildingInfo_.fundationExpireTime_ > expireTime ? 
        buildingInfo_.fundationExpireTime_ : expireTime;
}


sec_t Building::getInvincibleExpireTime() const 
{
    std::lock_guard<LockType> lock(getLock());

    return currentState_->getInvincibleExpireTime();
}


void Building::initInventories()
{
    if (buildingTemplate_->getBuildingInventoryCount() > 0) {
        buildingInfo_.buildingInventory_.defaultInvenCount_ =
            buildingTemplate_->getBuildingInventoryCount();
        buildingInventory_= std::make_unique<Inventory>(*this,
            buildingInfo_.buildingInventory_, itBuilding);
    }
    if (buildingTemplate_->getMercenaryInventoryCount() > 0) {
        buildingInfo_.mercenaryInventory_.defaultInvenCount_ =
            buildingTemplate_->getMercenaryInventoryCount();
        mercenaryInventory_= std::make_unique<Inventory>(*this,
            buildingInfo_.mercenaryInventory_, itMercenary);
    }
    if (buildingTemplate_->getProductionInventoryCount() > 0) {
        buildingInfo_.productionInventory_.defaultInvenCount_ =
            buildingTemplate_->getProductionInventoryCount();
        productionInventory_= std::make_unique<Inventory>(*this,
            buildingInfo_.productionInventory_, itBuildingProduction);
    }
    if (buildingTemplate_->getSelectProductionInventoryCount() > 0) {
        buildingInfo_.selectProductionInventory_.defaultInvenCount_ =
            buildingTemplate_->getSelectProductionInventoryCount();
        selectProductionInventory_= std::make_unique<Inventory>(*this,
            buildingInfo_.selectProductionInventory_, itBuildingSelectProduction);
    }
    
    buildingInfo_.fundationInventory_.defaultInvenCount_ = maxFundationBuildingSlotId;
    fundationInventory_= std::make_unique<Inventory>(*this,
        buildingInfo_.fundationInventory_, itBuildingFoundation);   
}


bool Building::isEnoughBuildingActionGrade(go::Entity& player) const
{
    if (! (buildingInfo_.state_ == bstFoundation || buildingInfo_.state_ == bstWreckage)) {
        if (isPrivateOwnerType(buildingInfo_.ownerInfo_.ownerType_)) {
            if (getOwnerId() != player.getObjectId()) {
                return false;
            }
        }
        else if (isGuildOwnerType(buildingInfo_.ownerInfo_.ownerType_)) {
            if (getOwnerGuildId() != player.queryGuildable()->getGuildId()) {
                return false;
            }
        }
    }
    return true;
}


ErrorCode Building::spawnBuildingGuard(ObjectId mercenaryId, 
    NpcCode npcCode, msec_t lifeTime, const ObjectPosition& position)
{
    WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return ecServerInternalError;
    }

    go::Entity* buildingGuard = nullptr;
    SpawnMapRegion& globalSpawnMapRegion =
        static_cast<SpawnMapRegion&>(worldMap->getGlobalMapRegion());
    const ErrorCode errorCode = globalSpawnMapRegion.summonNpc(buildingGuard, *this,
        position, npcCode, 0, lifeTime, sptSummon);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    BuildingGuardable* guardable = buildingGuard->queryBuildingGuardable(); assert(guardable != nullptr);
    if (guardable != nullptr) {
        guardable->setMercenaryId(mercenaryId);
    }

    return ecOk;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
