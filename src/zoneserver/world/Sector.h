#pragma once

#include "../zoneserver_export.h"
#include "Sector.h"
#include "SectorAddress.h"
#include "../model/gameobject/Entity.h"
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

/**
 * @class Sector
 * Space 내의 최소 구역
 */
class ZoneServer_Export Sector :
    public sne::core::ThreadSafeMemoryPoolMixin<Sector>
{
    typedef std::array<Sector*, dirCount> Neighbors;

    typedef std::mutex LockType;

public:
    Sector(const SectorAddress& address, float sectorLength, bool isGlobalSector);

    void reset();

    void setNeighbor(SectorDirection direction, Sector* neighbor);

public:
    bool enter(go::Entity& entity, const ObjectPosition& position);

    void leave(go::Entity& entity);

    /// destinSector로 entity를 이주시킨다
    bool migrate(go::Entity& entity, Sector& destinSector,
        const ObjectPosition& destination);

public:
    const SectorAddress& getAddress() const {
        return address_;
    }

    float getX() const {
        return x_;
    }

    float getY() const {
        return y_;
    }

public:
    size_t getEntityCount() const;
    size_t getInterestingEntityCount() const;

public:
    Neighbors& getNeighbors() {
        return neighbors_;
    }

    const Neighbors& getNeighbors() const {
        return neighbors_;
    }

    /// 주의! nullptr를 리턴할 수 있다
    const Sector* getNeighbor(SectorDirection direction) const {
        assert(isValid(direction));
        return neighbors_[direction];
    }

    /// 주의! nullptr를 리턴할 수 있다
    Sector* getNeighbor(SectorDirection direction) {
        assert(isValid(direction));
        return neighbors_[direction];
    }

public:
    bool isEntered(const go::Entity& entity) const;

public:
    LockType& getLock() {
        return lock_;
    }

private:    
    bool addEntity(go::Entity& entity);
    bool removeEntity(go::Entity& entity);

    void entityEntered(go::Entity& entity, const ObjectPosition& position);
    void entityLeft(go::Entity& entity);

    void appearedInNeighbor(go::Entity& entity, Sector& entitySector);
    void disappearedInNeighbor(go::Entity& entity, Sector& entitySector);

    void notifyEntitiesAppeared(go::Entity& to, const go::EntityMap& entities);
    void notifyEntitiesDisappeared(go::Entity& to, const go::EntityMap& entities);

    void notifyEntityAppeared(go::Entity& entity,
        Sector* entitySector = nullptr);
    void notifyEntityDisappeared(go::Entity& entity,
        Sector* entitySector = nullptr);
    void notifyNearEntityAppeared(go::Entity& entity,
        Sector* entitySector);
    void notifyNearEntityDisappeared(go::Entity& entity,
        Sector* entitySector);

    void fillEntities(go::EntityMap& entityMap) const;
	void fillBuildings(go::EntityMap& entityMap) const;

private:
    go::EntityMap getInterestingEntities() const;
	go::EntityMap getBuildingEntities() const;

    bool isEntered_i(const go::Entity& entity) const {
        return entities_.find(entity.getGameObjectInfo()) != entities_.end();
    }

    const go::EntityMap& getEntities_i() const {
        return entities_;
    }
	const go::EntityMap& getBuildingEntities_i() const {
		return buildings_;
	}

    bool contains(const Sector& sector) const {
        if (&sector == this) {
            return true;
        }
        const Neighbors::const_iterator pos =
            std::find(neighbors_.begin(), neighbors_.end(), &sector);
        return pos != neighbors_.end();
    }

    bool shouldNotifyAppearance(const go::Entity& entity) const;
	
	bool canSectorUseable(const go::Entity& entity) const;

private:
	bool isGlobalSector_;
    const SectorAddress address_;
    const float x_;
    const float y_;

    Neighbors neighbors_;

    go::EntityMap entities_; ///< 입장 중인 게임 Entity 목록
	go::EntityMap buildings_;		

    mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {
