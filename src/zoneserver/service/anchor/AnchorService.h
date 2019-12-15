#pragma once

#include "../../zoneserver_export.h"
#include "BuildingMarkCallback.h"
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>
#include <sne/core/container/Containers.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace datatable {
struct CheckAnchorInfo;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
class Player;
class AbstractAnchor;
class Anchor;
class Building;
} // namespace go

class EntityPool;
class WorldMap;
class SpawnMapRegion;

/**
 * @class AnchorService
 *
 * 앵커 서비스
 */
class ZoneServer_Export AnchorService : public boost::noncopyable,
    private BuildingMarkCallback
{
    SNE_DECLARE_SINGLETON(AnchorService);

    typedef std::mutex LockType;
    typedef sne::core::Map<GameObjectInfo, go::Building*> BuildingMap;
    typedef std::unique_ptr<go::Building> BuildingPtr;

public:
    AnchorService();
    ~AnchorService();

    bool initialize(ObjectId maxBuildId, const BuildingInfoMap& buildInfoMap);

	// 서버 로딩시에만 .
	// spawnAnchor, spawnBuilding
    ErrorCode spawnBuilding(BuildingCode buildingCode, ObjectId linkBuildingId,
		const ObjectPosition& position, WorldMap& worldMap, SpawnMapRegion& spawnMapRegion);
    ErrorCode spawnBuildings(WorldMap& worldMap, SpawnMapRegion& spawnMapRegion, 
        const datatable::PositionSpawnTable::Spawns& spawns);


    ErrorCode spawnBuilding(const ObjectPosition& position,
        ObjectId itemId, go::Entity& player);

    void destroyBuilding(go::Entity& building);
    
    bool isModelingIntersection(const Position& position, float32_t modelingRadius) const;

public:
    void fillBuildingMarkInfos(BuildingMarkInfos& buildingMarkInfos) const;

public:
    go::Building* getBuilding(const GameObjectInfo& objectInfo);

private:
    ErrorCode createBuilding(go::Building*& building, ObjectId linkBuildingId,
        const ObjectPosition& position, DataCode anchorCode);
    ErrorCode createBuilding(go::Building*& building, ObjectId linkBuildingId, 
        const BuildingInfo& buildingInfo);

    bool addBuilding(go::Building& building);
    void removeBuilding(go::Entity& entity);

private:
    bool isModelingIntersection_i(const Position& position, float32_t modelingRadius) const;
	bool isNearDominionBuilding(const Position& position) const;
    ErrorCode  checkExistNearNeedBuilding(go::Entity& builder, ObjectId& linkBuildingId,
        const Position& position, const datatable::CheckAnchorInfo& checkAnchorInfo) const;

    ErrorCode checkCreateAnchor(DataCode& anchorCode, go::Entity& player,
        const ObjectPosition& position, ObjectId itemId) const;
    ErrorCode checkCreateBuilding(DataCode& anchorCode, ObjectId& linkBuildingId, go::Entity& player,
        const ObjectPosition& position, ObjectId itemId) const;
    ErrorCode checkInventory(DataCode& anchorCode, go::Entity& player, ObjectId itemId) const;

    ObjectId getNeedBuildingId(const Position& position, 
        const datatable::CheckAnchorInfo& checkAncorInfo) const;
private:
    // = BuildingMarkCallback overriding
    virtual void addBuildingMarkInfo(ObjectId buildingId, const BuildingMarkInfo& buildingMarkInfo);
    virtual void removeBuildingMarkInfo(ObjectId buildingId);

private:
    BuildingMap buildingMap_;
    BuildingMap coreBuildingMap_; // 절대 삭제 되지 않는다.
    BuildingMarkInfoMap buildingMarkInfoMap_;
    
    std::unique_ptr<EntityPool> buildingPool_;

	mutable LockType lock_;
    mutable LockType markLock_;
};

}} // namespace gideon { namespace zoneserver {

#define ANCHOR_SERVICE gideon::zoneserver::AnchorService::instance()
