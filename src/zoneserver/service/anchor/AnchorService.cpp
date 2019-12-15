#include "ZoneServerPCH.h"
#include "AnchorService.h"
#include "../spawn/EntityPool.h"
#include "../../controller/AnchorController.h"
#include "../../controller/BuildingController.h"
#include "../../model/gameobject/allocator/EntityAllocator.h"
#include "../../model/gameobject/Anchor.h"
#include "../../model/gameobject/Building.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/gameobject/ability/CastGameTimeable.h"
#include "../../model/gameobject/ability/Guildable.h"
#include "../../model/item/Inventory.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../world/region/SpawnMapRegion.h"
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/3d/3d.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

namespace {


/**
 * @class BuildingAllocator
 */
class BuildingAllocator : public go::EntityAllocator
{
public:
    static BuildingAllocator& instance(BuildingMarkCallback& callback) {
        static BuildingAllocator s_buildingAllocator(callback);
        return s_buildingAllocator;
    }

public:
    BuildingAllocator(BuildingMarkCallback& markCallback) :
        markCallback_(markCallback) {}

private:
    virtual go::Entity* malloc() {
        return new go::Building(std::make_unique<gc::BuildingController>(), markCallback_);
    }

    void free(go::Entity* building) {
        boost::checked_delete(static_cast<go::Building*>(building));
    }

private:
    BuildingMarkCallback& markCallback_;
};


ErrorCode spawnAbstractAnchor(go::AbstractAnchor& building,
    WorldMap& worldMap, const ObjectPosition& position,
    SpawnMapRegion* spawnMapRegion = nullptr)
{
    PositionSpawnTemplate spawnTemplate;
    spawnTemplate.entityCode_ = building.getEntityCode();
    spawnTemplate.position_ = position;

    if (! spawnMapRegion) {
        spawnMapRegion = static_cast<SpawnMapRegion*>(&worldMap.getGlobalMapRegion());
    }
    assert(spawnMapRegion != nullptr);

    building.setWorldMap(worldMap);
    building.setMapRegion(*spawnMapRegion);
    building.setSpawnTemplate(spawnTemplate);

    const ErrorCode errorCode = spawnMapRegion->spawnEntity(building, spawnTemplate);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    return ecOk;
}

const size_t defaultAnchorPoolSize = 100;
const size_t buildAnchorPoolSize = 100;

} //  namepsace

SNE_DEFINE_SINGLETON(AnchorService);

AnchorService::AnchorService()
{
    buildingPool_ = std::make_unique<EntityPool>(BuildingAllocator::instance(*this), buildAnchorPoolSize);
}


AnchorService::~AnchorService()
{
}


bool AnchorService::initialize(ObjectId maxBuildId, const BuildingInfoMap& buildInfoMap)
{
    if (! BUILDING_TABLE) {
        return false;
    }

    buildingPool_->set(maxBuildId);
    WorldMap& worldMap = WORLD->getGlobalWorldMap();


    for (const BuildingInfoMap::value_type& value : buildInfoMap) {
        const BuildingInfo& buildingInfo = value.second;

        const datatable::BuildingTemplate* buildingTemplate = BUILDING_TABLE->getBuildingTemplate(buildingInfo.buildingCode_);
        if (! buildingTemplate) {
            return false;
        }
        if (! buildingTemplate->isCoreBuilding()) {
            continue;
        }

        go::Building* building = nullptr;
        ErrorCode errorCode = createBuilding(building, invalidObjectId, buildingInfo);
        if (isFailed(errorCode)) {
            return false;
        }

        errorCode = spawnAbstractAnchor(*building, worldMap, buildingInfo.position_);
        if (isFailed(errorCode)) {
            return false;
        }

        if (! building->initializeGuard()) {
            return false;
        }

        addBuilding(*building);
    }

    for (const BuildingInfoMap::value_type& value : buildInfoMap) {
        const BuildingInfo& buildingInfo = value.second;

        const datatable::BuildingTemplate* buildingTemplate = BUILDING_TABLE->getBuildingTemplate(buildingInfo.buildingCode_);
        if (! buildingTemplate) {
            return false;
        }

        if (buildingTemplate->isCoreBuilding()) {
            continue;
        }

        const datatable::CheckAnchorInfo& checkAnchorInfo = buildingTemplate->getCheckAnchorInfo();
        ObjectId buildingId = invalidObjectId;
        if (checkAnchorInfo.isValid()) {
            buildingId = getNeedBuildingId(buildingInfo.position_, checkAnchorInfo);
        }

        go::Building* building = nullptr;
        ErrorCode errorCode = createBuilding(building, buildingId, buildingInfo);
        if (isFailed(errorCode)) {
            return false;
        }

        errorCode = spawnAbstractAnchor(*building, worldMap, buildingInfo.position_);
        if (isFailed(errorCode)) {
            return false;
        }

        if (! building->initializeGuard()) {
            return false;
        }

        addBuilding(*building);
    }

    AnchorService::instance(this);
    return true;
}


ErrorCode AnchorService::spawnBuilding(BuildingCode buildingCode, ObjectId linkBuildingId,
    const ObjectPosition& position, WorldMap& worldMap, SpawnMapRegion& spawnMapRegion)
{
    go::Building* building = nullptr;
    if (isNearDominionBuilding(position)) {
        return ecOk;
    }
    ErrorCode errorCode =
        createBuilding(building, linkBuildingId, position, buildingCode);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    errorCode = spawnAbstractAnchor(*building, worldMap, position, &spawnMapRegion);
    if (isFailed(errorCode)) {
        buildingPool_->releaseEntity(building);
        return errorCode;
    }

    addBuilding(*building);
    return ecOk;
}


ErrorCode AnchorService::spawnBuildings(WorldMap& worldMap, SpawnMapRegion& spawnMapRegion,
    const datatable::PositionSpawnTable::Spawns& spawns)
{
    for (const PositionSpawnTemplate& spawnTemplate :  spawns) {
        const DataCode entityCode = spawnTemplate.entityCode_;
        if (! isValidBuildingCode(entityCode)) {
            continue;
        }
        const datatable::BuildingTemplate* buildingTemplate = BUILDING_TABLE->getBuildingTemplate(entityCode);
        if (! buildingTemplate) {
            return ecServerInternalError;
        }
        if (! buildingTemplate->isCoreBuilding()) {
            continue;
        }
        const ErrorCode errorCode = spawnBuilding(entityCode, invalidObjectId, spawnTemplate.position_, worldMap, spawnMapRegion);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }


    for (const PositionSpawnTemplate& spawnTemplate :  spawns) {
        const DataCode entityCode = spawnTemplate.entityCode_;
        if (! isValidBuildingCode(entityCode)) {
            continue;
        }
        const datatable::BuildingTemplate* buildingTemplate = BUILDING_TABLE->getBuildingTemplate(entityCode);
        if (! buildingTemplate) {
            return ecServerInternalError;
        }
        if (buildingTemplate->isCoreBuilding()) {
            continue;
        }
        const datatable::CheckAnchorInfo& checkAnchorInfo = buildingTemplate->getCheckAnchorInfo();
        ObjectId buildingId = invalidObjectId;
        if (checkAnchorInfo.isValid()) {
            buildingId = getNeedBuildingId(spawnTemplate.position_, checkAnchorInfo);
        }
        const ErrorCode errorCode = spawnBuilding(entityCode, buildingId, spawnTemplate.position_, worldMap, spawnMapRegion);
        if (isFailed(errorCode)) {
            return errorCode;
        }
    }

    return ecOk;
}


ErrorCode AnchorService::spawnBuilding(const ObjectPosition& position,
    ObjectId itemId, go::Entity& player)
{
    DataCode itemCode = invalidDataCode;
    ObjectId linkBuildingId = invalidObjectId;
    ErrorCode errorCode = checkCreateBuilding(itemCode, linkBuildingId, player, position, itemId);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    WorldMap* worldMap = player.getCurrentWorldMap();
    if (! worldMap) {
        return ecServerInternalError;
    }

    go::Building* building = nullptr;
    errorCode = createBuilding(building, linkBuildingId, position, itemCode);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    errorCode = spawnAbstractAnchor(*building, *worldMap, position);
    if (isFailed(errorCode)) {
        buildingPool_->releaseEntity(building);
        return errorCode;
    }

    addBuilding(*building);
    player.queryInventoryable()->getInventory().useItemsByItemId(itemId, 1);

	const datatable::BuildingTemplate* buildingTemlate = BUILDING_TABLE->getBuildingTemplate(itemCode);
	if (buildingTemlate->getCooltime() > 0) {		
		player.queryCastGameTimeable()->setCooldown(itemCode, buildingTemlate->getCooltime(), 0, 0);
	}

    return ecOk;
}


go::Building* AnchorService::getBuilding(const GameObjectInfo& objectInfo)
{
    std::lock_guard<LockType> lock(lock_);

    BuildingMap::iterator pos = buildingMap_.find(objectInfo);
    if (pos != buildingMap_.end()) {
        go::Building* building = (*pos).second;
        return building;        
    }
    return nullptr;
}


void AnchorService::destroyBuilding(go::Entity& building)
{
    std::lock_guard<LockType> lock(lock_);

    removeBuilding(building);
}


bool AnchorService::isModelingIntersection(const Position& position, float32_t modelingRadius) const
{
    std::lock_guard<LockType> lock(lock_);

    return isModelingIntersection_i(position, modelingRadius);
}


ErrorCode AnchorService::createBuilding(go::Building*& building, ObjectId linkBuildingId,
    const ObjectPosition& position, DataCode buildingCode)
{
    building = buildingPool_->acquireEntity<go::Building>();
    if (! building) {
        return ecServerInternalError;
    }

    const ObjectId objectId = buildingPool_->generateObjectId();

    if (! building->initialize(buildingCode, objectId, linkBuildingId, position)) {
        return ecServerInternalError;
    }

    return ecOk;
}


ErrorCode AnchorService::createBuilding(go::Building*& building,
    ObjectId linkBuildingId, const BuildingInfo& buildingInfo)
{
    building = buildingPool_->acquireEntity<go::Building>();
    if (! building) {
        return ecServerInternalError;
    }

    if (! building->initialize(linkBuildingId, buildingInfo)) {
        return ecServerInternalError;
    }

    return ecOk;
}


bool AnchorService::addBuilding(go::Building& building)
{
    std::lock_guard<LockType> lock(lock_);

    if (building.isCoreBuilding()) {
        coreBuildingMap_.emplace(GameObjectInfo(otBuilding, building.getObjectId()), &building);
    }
    else if (isValidObjectId(building.getLinkCoreBuildingId())) {
        BuildingMap::iterator pos = coreBuildingMap_.find(GameObjectInfo(otBuilding, building.getLinkCoreBuildingId()));
        if (pos != coreBuildingMap_.end()) {
            go::Building* coreBuilding = static_cast<go::Building*>((*pos).second);
            coreBuilding->addLinkBuilding(building.getObjectId());
        }
    }
    

    return buildingMap_.insert(BuildingMap::value_type(building.getGameObjectInfo(), &building)).second;
}


void AnchorService::removeBuilding(go::Entity& entity)
{
    if (! entity.isBuilding()) {
        return;
    }

    go::Building& building = static_cast<go::Building&>(entity);
    buildingMap_.erase(building.getGameObjectInfo());

    if (building.isCoreBuilding()) {
        coreBuildingMap_.erase(building.getGameObjectInfo());
    }
    else if (isValidObjectId(building.getLinkCoreBuildingId())) {
        BuildingMap::iterator pos = coreBuildingMap_.find(GameObjectInfo(otBuilding, building.getLinkCoreBuildingId()));
        if (pos != coreBuildingMap_.end()) {
            go::Building* coreBuilding = static_cast<go::Building*>((*pos).second);
            coreBuilding->removeLinkBuilding(building.getObjectId());
        }
    }
    
    coreBuildingMap_.erase(building.getGameObjectInfo());
    buildingPool_->releaseEntity(&building);    
}


bool AnchorService::isModelingIntersection_i(const Position& center, float32_t modelingRadius) const
{
    std::lock_guard<LockType> lock(lock_);

    const Vector2& posSource = asVector2(center);
    for (const BuildingMap::value_type& value : buildingMap_) {
        const go::Building* building = value.second;
        if (building) {
            const Position position = building->getPosition();
            Vector2 dirTarget = (asVector2(position) - posSource);
            const float32_t distanceSq = squaredLength(dirTarget);
            const float32_t targetRadiusSq = modelingRadius + building->getModelingRadiusSize();
            const float32_t checkRadiusDistance = targetRadiusSq * targetRadiusSq;
            if (distanceSq <= checkRadiusDistance) {
                return false;
            }
        }
    }

    return true;
}


bool AnchorService::isNearDominionBuilding(const Position& position) const
{
	const float32_t checkDistanceSq = 0.1f * 0.1f;
	const Vector2& posSource = asVector2(position);
	for (const BuildingMarkInfoMap::value_type& value : buildingMarkInfoMap_) {
		const BuildingMarkInfo& info= value.second;
		Vector2 dirTarget = (asVector2(info.position_) - posSource);
		const float32_t distanceSq = squaredLength(dirTarget);
		if (distanceSq <= checkDistanceSq) {
			return true;
		}
	}
	return false;
}


ErrorCode AnchorService::checkExistNearNeedBuilding(go::Entity& builder, ObjectId& linkBuildingId,
    const Position& position, const datatable::CheckAnchorInfo& checkAnchorInfo) const
{
    linkBuildingId = invalidObjectId;

    const Vector2& posSource = asVector2(position);
    for (const BuildingMap::value_type& value : buildingMap_) {
        const go::Building* building = value.second;        
        if (! checkAnchorInfo.hasCheckCode(building->getEntityCode())) {
            continue;
        }

        const datatable::BuildingTemplate* checkBuildingTemplate = 
            BUILDING_TABLE->getBuildingTemplate(building->getEntityCode());
        if (! checkBuildingTemplate) {
            return ecAnchorInvalidItem;
        }

        const datatable::BaseAnchorInfo& checkInfo = checkBuildingTemplate->getBaseAnchorInfo();
        
        const float32_t checkDistance = checkAnchorInfo.getCheckDistance(building->getEntityCode());
        const float32_t checkDistanceSq = checkDistance * checkDistance;
        const BuildingOwnerInfo& ownerInfo = 
            building->queryStaticObjectOwnerable()->getBuildingOwnerInfo();

        if (checkInfo.ownerType_ == botPrivate) {
            if (ownerInfo.playerOwnerInfo_.objectId_ != builder.getObjectId()) {
                continue;
            }
        }
        else if (checkInfo.ownerType_ == botGuild) {
            if (ownerInfo.guildInfo_.guildId_ != builder.queryGuildable()->getGuildId()) {
                continue;
            }
        }            

        Vector2 dirTarget = (asVector2(building->getPosition()) - posSource);
        const float32_t distanceSq = squaredLength(dirTarget);
        if (distanceSq > checkDistanceSq) {
            continue;
        }
        //if (! building->isCompleteState()) {
        //    return ecAnchorNotCompleteNearNeedAnchor;
        //}
        //else if (! building->isActiveAbillity()) {
        //    return ecAnchorNotActivateNearNeedAnchor;
        //}
        linkBuildingId = building->getObjectId();
        return ecOk;                
        
    }
    return ecAnchorNeedNotBuildAnchor;
}


ObjectId AnchorService::getNeedBuildingId(const Position& position, 
    const datatable::CheckAnchorInfo& checkAnchorInfo) const
{
    const Vector2& posSource = asVector2(position);
    for (const BuildingMap::value_type& value : buildingMap_) {
        const go::Building* building = value.second;
        if (! checkAnchorInfo.hasCheckCode(building->getEntityCode())) {
            continue;
        }
        const float32_t checkDistance = checkAnchorInfo.getCheckDistance(building->getEntityCode());
        const float32_t checkDistanceSq = checkDistance * checkDistance;
        Vector2 dirTarget = (asVector2(building->getPosition()) - posSource);
        const float32_t distanceSq = squaredLength(dirTarget);
        if (distanceSq > checkDistanceSq) {
            continue;
        }

        return building->getObjectId();                        
    }
    return invalidObjectId;
}


ErrorCode AnchorService::checkCreateAnchor(DataCode& buildingCode, go::Entity& player,
    const ObjectPosition& position, ObjectId itemId) const
{
    ErrorCode errorCode = checkInventory(buildingCode, player, itemId);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    const datatable::AnchorTemplate* buildingTemplate = ANCHOR_TABLE->getAnchorTemplate(buildingCode);
    if (! buildingTemplate) {
        return ecAnchorInvalidItem;
    }

    if (! isModelingIntersection_i(position, buildingTemplate->getModelingRadius())) {
        return ecAnchorNearDistanceBuildAnchor;    
    }

    return ecOk;
}


ErrorCode AnchorService::checkCreateBuilding(DataCode& buildingCode, ObjectId& linkBuildingId,
    go::Entity& player, const ObjectPosition& position, ObjectId itemId) const
{
    ErrorCode errorCode = checkInventory(buildingCode, player, itemId);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    const datatable::BuildingTemplate* buildingTemplate = 
        BUILDING_TABLE->getBuildingTemplate(buildingCode);
    if (! buildingTemplate) {
        return ecAnchorInvalidItem;
    }

    if (! isModelingIntersection_i(position, buildingTemplate->getModelingRadius())) {
        return ecAnchorNearDistanceBuildAnchor;
    }

    const datatable::CheckAnchorInfo& checkAnchorInfo = buildingTemplate->getCheckAnchorInfo();
    if (checkAnchorInfo.isValid()) {
        errorCode = checkExistNearNeedBuilding(player, linkBuildingId, position, checkAnchorInfo);        
    }

    return errorCode;
}



ErrorCode AnchorService::checkInventory(DataCode& buildingCode, go::Entity& player, ObjectId itemId) const
{
    Inventory& inventory = player.queryInventoryable()->getInventory();
    DataCode itemCode = inventory.getItemCode(itemId);
    if (! isValidDataCode(itemCode)) {
        return ecInventoryItemNotFound;
    }

    buildingCode = itemCode;

	if (player.queryCastGameTimeable()->isLocalCooldown(buildingCode)) {
		return ecAnchorNotEnoughCoolTime;
	}
    
    return ecOk;
}


void AnchorService::fillBuildingMarkInfos(BuildingMarkInfos& buildingMarkInfos) const
{
    std::lock_guard<LockType> lock(markLock_);

    for (const BuildingMarkInfoMap::value_type& value : buildingMarkInfoMap_) {
        buildingMarkInfos.push_back(value.second);   
    }
}

// = BuildingMarkCallback overriding

void AnchorService::addBuildingMarkInfo(ObjectId buildingId, const BuildingMarkInfo& buildingMarkInfo)
{
    std::lock_guard<LockType> lock(markLock_);

    buildingMarkInfoMap_.emplace(buildingId, buildingMarkInfo);
}


void AnchorService::removeBuildingMarkInfo(ObjectId buildingId)
{
    std::lock_guard<LockType> lock(markLock_);

    buildingMarkInfoMap_.erase(buildingId);
}

}} // namespace gideon { namespace zoneserver {
