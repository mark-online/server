#include "ZoneServerPCH.h"
#include "Sector.h"
#include "../model/gameobject/EntityEvent.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/ability/Positionable.h"
#include "../controller/EntityController.h"
#include "../controller/callback/InterestAreaCallback.h"
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {


namespace {

inline bool canNotify(go::Entity& entity, go::Entity* listener, Sector* entitySector,
	const SectorAddress& entitySectorAddress, bool isGlobalSector)
{
	if (listener->isSame(entity)) {
		return false;
	}

	if (entitySector != nullptr) {		
		const SectorAddress listnerSectorAddress = 
			isGlobalSector ? listener->getGlobalSectorAddress() : listener->getSectorAddress();
		if (listnerSectorAddress.isNeighbor(entitySectorAddress)) {
			return false;
		}
	}

	if (isGlobalSector) {
		if (entity.isBuilding()) {
			return true;
		}
		
		if (listener->isBuilding()) {
			return true;
		}
		return false;
	}

	return true;
}

/**
 * @class EntityDisappearedEvent
 */
class EntityDisappearedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<EntityDisappearedEvent>
{
public:
    /// FYI: entity의 정보가 무효화되므로, 반드시 GameObjectInfo로 받아와야 한다.
    EntityDisappearedEvent(const GameObjectInfo& targetInfo) :
        targetInfo_(targetInfo) {}

private:
    virtual void call(go::Entity& entity) {
        go::Knowable* knowable = entity.queryKnowable();
        if (! knowable) {
            return;
        }
        if (! knowable->doesKnow(targetInfo_)) {
            return;
        }

        gc::InterestAreaCallback* interestAreaCallback =
            entity.getController().queryAppearanceCallback();
        if (! interestAreaCallback) {
            return;
        }
        interestAreaCallback->entityDisappeared(targetInfo_);
    }

private:
    const GameObjectInfo targetInfo_;
};


/**
 * @class EntityAppearedEvent
 */
class EntityAppearedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<EntityAppearedEvent>
{
public:
    EntityAppearedEvent(go::Entity& target) :
        target_(target) {}

private:
    virtual void call(go::Entity& entity) {
        // TODO: 공개파티 처리때문에 추후 수정
        if (! entity.isDungeon()) {
            go::Knowable* knowable = entity.queryKnowable();
            if (! knowable) {
                return;
            }
            if (knowable->doesKnow(target_.getGameObjectInfo())) {
                return;
            }
        }

        gc::InterestAreaCallback* interestAreaCallback =
            entity.getController().queryAppearanceCallback();
        if (! interestAreaCallback) {
            return;
        }
        interestAreaCallback->entityAppeared(target_, target_.getUnionEntityInfo());
    }

private:
    go::Entity& target_;
};

} // namespace {

// = Sector

Sector::Sector(const SectorAddress& address, float sectorLength, bool isGlobalSector) :
    address_(address),
    x_(address.getColumn() * sectorLength),
    y_(address.getRow() * sectorLength),
	isGlobalSector_(isGlobalSector)
{
    neighbors_.fill(nullptr);
}


void Sector::reset()
{
    std::unique_lock<LockType> lock(lock_);

    entities_.clear();
}


void Sector::setNeighbor(SectorDirection direction, Sector* neighbor)
{
    assert(isValid(direction));
    assert(neighbor != nullptr);

    std::unique_lock<LockType> lock(lock_);

    assert(! neighbors_[direction]);
    neighbors_[direction] = neighbor;
}


bool Sector::enter(go::Entity& entity, const ObjectPosition& position)
{
	if (! canSectorUseable(entity)) {
		return false;
	}

    {
        std::unique_lock<LockType> lock(lock_);

        if (! addEntity(entity)) {
            return false;
        }
       
        //SNE_LOG_DEBUG8("Sector::enter(E%" PRIu64 ") (%d,%d) [%2.1f,%2.1f] <%u,%u>.",
        //    entity.getObjectId(), address_.getColumn(), address_.getRow(),
        //    x_, y_, entities_.size(), listeners_.size());
    }

    entityEntered(entity, position);
    
    return true;
}


void Sector::leave(go::Entity& entity)
{
	if (! canSectorUseable(entity)) {
		return;
	}

    {
        std::unique_lock<LockType> lock(lock_);

        if (! removeEntity(entity)) {
            return;
        }

        //SNE_LOG_DEBUG8("Sector::leave(E%" PRIu64 ") (%d,%d) [%2.1f,%2.1f] <%u,%u>.",
        //    entity.getObjectId(), address_.getColumn(), address_.getRow(), x_, y_,
        //    entities_.size(), listeners_.size());
    }

    entityLeft(entity);
}


bool Sector::migrate(go::Entity& entity, Sector& destinSector,
    const ObjectPosition& destination)
{
	if (! canSectorUseable(entity)) {
		return false;
	}

    assert(&destinSector != this);

    bool isNotifyDisappeared = false;
    go::EntityMap entities;
    {
        std::unique_lock<LockType> lock(lock_);

       const bool result = removeEntity(entity);
        assert(result); result;
            
        isNotifyDisappeared = destinSector.contains(*this);
        if (! isNotifyDisappeared) {       
            if (shouldNotifyAppearance(entity)) {
				if (isGlobalSector_) {
					entities = getBuildingEntities_i();
				}
				else {
					entities = getEntities_i();
				}
            }
        }
    }
    
    if (! isNotifyDisappeared) {
        notifyEntityDisappeared(entity);
    }

    notifyEntitiesDisappeared(entity, entities);

    bool isNotifyAppeared = false;
    {
        std::unique_lock<LockType> lock(lock_);

        const bool result = destinSector.addEntity(entity);
        assert(result); result;
        isNotifyAppeared = contains(destinSector);
        if (! isNotifyAppeared) {
            if (shouldNotifyAppearance(entity)) {
				if (isGlobalSector_) {
					entities = destinSector.getBuildingEntities_i();
				}
				else {
					entities = destinSector.getEntities_i();
				}
            }
        }
    }
    
    if (! isNotifyAppeared) {
        destinSector.notifyEntityAppeared(entity);
    }

    notifyEntitiesAppeared(entity, entities);

    go::Positionable* positionable = entity.queryPositionable();
    if (positionable != nullptr) {
        positionable->setSectorAddress(destinSector.getAddress());
        positionable->setPosition(destination);
    }

    Neighbors& originNeighbors = getNeighbors();
    for (Sector* originNeighbor : originNeighbors) {
        if (! originNeighbor) {
            continue;
        }
        if (! destinSector.contains(*originNeighbor)) {
            originNeighbor->disappearedInNeighbor(entity, destinSector);
        }
    }

    Neighbors& destinNeighbors = destinSector.getNeighbors();
    for (Sector* destinNeighbor : destinNeighbors) {
        if (! destinNeighbor) {
            continue;
        }
        if (! contains(*destinNeighbor)) {
            destinNeighbor->appearedInNeighbor(entity, *this);
        }
    }

    return true;
}


size_t Sector::getEntityCount() const
{
    std::unique_lock<LockType> lock(lock_);

    return entities_.size();
}


size_t Sector::getInterestingEntityCount() const
{
    size_t count = getEntityCount();

    for (int i = 0; i < dirCount; ++i) {
        const SectorDirection direction = static_cast<SectorDirection>(i);
        const Sector* neighbor = getNeighbor(direction);
        if (neighbor != nullptr) {
            count += neighbor->getEntityCount();
        }
    }
    return count;
}


bool Sector::isEntered(const go::Entity& entity) const
{
    std::unique_lock<LockType> lock(lock_);

    return isEntered_i(entity);
}


bool Sector::addEntity(go::Entity& entity)
{
    if (isEntered_i(entity)) {
        return false;
    }

    entities_.insertEntity(entity);
	if (isGlobalSector_ && entity.isBuilding()) {
		buildings_.insertEntity(entity);
	}
    return true;
}


bool Sector::removeEntity(go::Entity& entity)
{
    if (! isEntered_i(entity)) {
        return false;
    }

    entities_.erase(entity.getGameObjectInfo());

	if (isGlobalSector_ && entity.isBuilding()) {
		buildings_.erase(entity.getGameObjectInfo());
	}

    return true;
}


void Sector::entityEntered(go::Entity& entity, const ObjectPosition& position)
{
    go::Positionable* positionable = entity.queryPositionable();
    if (positionable != nullptr) {
		if (isGlobalSector_) {
			positionable->setGlobalSectorAddress(address_);
		}
		else {
			positionable->setSectorAddress(address_);
			positionable->setPosition(position);
		}
    }


    // 비석이 스폰될 때 알림을 받지도 보내지도 않는다
    if (isGraveStone(entity.getObjectType())) {
        return;
    }
   
    notifyEntityAppeared(entity);
	
	if (! shouldNotifyAppearance(entity)) {
		return;
	}

	if (isGlobalSector_ && ! entity.isBuilding()) {		
		notifyEntitiesAppeared(entity, getBuildingEntities());
	}
	else {
		notifyEntitiesAppeared(entity, getInterestingEntities());					
	}
}


void Sector::entityLeft(go::Entity& entity)
{
    go::Positionable* positionable = entity.queryPositionable();
	
	if (positionable != nullptr) {
		if (isGlobalSector_) {
			positionable->setGlobalSectorAddress(SectorAddress());
		}
		else {
			positionable->setSectorAddress(SectorAddress());
		}
	}
    
    notifyEntityDisappeared(entity);    

    if (! shouldNotifyAppearance(entity)) {
		return ;
	}

	if (isGlobalSector_ & ! entity.isBuilding()) {		
		notifyEntitiesDisappeared(entity, getBuildingEntities());		
	}
	else {
		notifyEntitiesDisappeared(entity, getInterestingEntities());	
	}

    if (! isGlobalSector_ || (isGlobalSector_ && entity.isBuilding())) {	
		entity.getController().queryAppearanceCallback()->entityDisappeared(entity);
	}
}


void Sector::appearedInNeighbor(go::Entity& entity, Sector& entitySector)
{
    notifyNearEntityAppeared(entity, &entitySector);

    // TODO: 이미 보이는 entity가 또 보일 수 있다
    if (shouldNotifyAppearance(entity)) {
        go::EntityMap entities;
        {
            std::unique_lock<LockType> lock(lock_);

            if (isGlobalSector_ && !entity.isBuilding()) {
				entities = buildings_;
			}
			else {
			   entities = entities_;
			}
		}
        notifyEntitiesAppeared(entity, entities);
    }
}


void Sector::disappearedInNeighbor(go::Entity& entity, Sector& entitySector)
{
    notifyNearEntityDisappeared(entity, &entitySector);

    if (shouldNotifyAppearance(entity)) {
        go::EntityMap entities;
        {
            std::unique_lock<LockType> lock(lock_);

            if (isGlobalSector_ && !entity.isBuilding()) {
				entities = buildings_;
			}
			else {
				entities = entities_;
			}
        }
        notifyEntitiesDisappeared(entity, entities);
    }
}


void Sector::notifyEntitiesAppeared(go::Entity& to, const go::EntityMap& entities)
{
    if (entities.empty()) {
        return;
    }

    gc::InterestAreaCallback* interestAreaCallback =
        to.getController().queryAppearanceCallback();
    if (interestAreaCallback != nullptr) {
        interestAreaCallback->entitiesAppeared(entities);
    }
}


void Sector::notifyEntitiesDisappeared(go::Entity& to, const go::EntityMap& entities)
{
    if (entities.empty()) {
        return;
    }

    gc::InterestAreaCallback* interestAreaCallback =
        to.getController().queryAppearanceCallback();
    if (interestAreaCallback != nullptr) {
        interestAreaCallback->entitiesDisappeared(entities);
    }
}


void Sector::notifyEntityAppeared(go::Entity& entity, Sector* entitySector)
{
    Sector* sector = (entitySector != nullptr) ? entitySector : this;
    sector->notifyNearEntityAppeared(entity, entitySector);

    for (Sector* neighborSector : sector->getNeighbors()) {
        if (! neighborSector) {
            continue;
        }
        neighborSector->notifyNearEntityAppeared(entity, entitySector);
    }
}


void Sector::notifyNearEntityAppeared(go::Entity& entity, Sector* entitySector)
{
    std::unique_lock<LockType> lock(lock_);

	if (isGlobalSector_) {
		if (! entity.isBuilding() && buildings_.empty()) {
			return;
		}
	}

    SectorAddress entitySectorAddress;
    if (entitySector != nullptr) {
        assert(this->getAddress() != entitySector->getAddress());
        entitySectorAddress = entitySector->getAddress();
    }

    auto event = std::make_shared<EntityAppearedEvent>(entity);
    for (const go::EntityMap::value_type& value : entities_) {
        //const ObjectId listenerId = value.first;
        go::Entity* listener = value.second;
		if (! canNotify(entity, listener, entitySector, entitySectorAddress, isGlobalSector_)) {
			continue;
		}

        (void)TASK_SCHEDULER->schedule(std::make_unique<go::EventCallTask>(*listener, event));
    }
}


void Sector::notifyEntityDisappeared(go::Entity& entity, Sector* entitySector)
{
    Sector* sector = (entitySector != nullptr) ? entitySector : this;
    sector->notifyNearEntityDisappeared(entity, entitySector);

    for (Sector* neighborSector : sector->getNeighbors()) {
        if (! neighborSector) {
            continue;
        }
        neighborSector->notifyNearEntityDisappeared(entity, entitySector);
    }       
}


void Sector::notifyNearEntityDisappeared(go::Entity& entity, Sector* entitySector)
{
    std::lock_guard<LockType> lock(lock_);

	if (isGlobalSector_) {
		if (! entity.isBuilding() && buildings_.empty()) {
			return;
		}
	}

    SectorAddress entitySectorAddress;
    if (entitySector != nullptr) {
        assert(this->getAddress() != entitySector->getAddress());
        entitySectorAddress = entitySector->getAddress();
    }

    auto event = std::make_shared<EntityDisappearedEvent>(entity.getGameObjectInfo());
    for (const go::EntityMap::value_type& value : entities_) {
        //const ObjectId listenerId = value.first;
        go::Entity* listener = value.second;
		if (! canNotify(entity, listener, entitySector, entitySectorAddress, isGlobalSector_)) {
			continue;
		}

        (void)TASK_SCHEDULER->schedule(std::make_unique<go::EventCallTask>(*listener, event));
    }
}


void Sector::fillEntities(go::EntityMap& entityMap) const
{
    std::unique_lock<LockType> lock(lock_);

    if (! entities_.empty()) {
        entityMap.insert(entities_.begin(), entities_.end());
    }
}


void Sector::fillBuildings(go::EntityMap& entityMap) const
{
    std::unique_lock<LockType> lock(lock_);

	if (! buildings_.empty()) {
		entityMap.insert(buildings_.begin(), buildings_.end());
	}
}


go::EntityMap Sector::getInterestingEntities() const
{
    go::EntityMap allEntities;
    fillEntities(allEntities);

    for (int i = 0; i < dirCount; ++i) {
        const SectorDirection direction = static_cast<SectorDirection>(i);
        const Sector* neighbor = getNeighbor(direction);
        if (neighbor != nullptr) {
            neighbor->fillEntities(allEntities);
        }
    }

    return allEntities;
}


go::EntityMap Sector::getBuildingEntities() const
{
	go::EntityMap buildingEntities;
	fillBuildings(buildingEntities);

	for (int i = 0; i < dirCount; ++i) {
		const SectorDirection direction = static_cast<SectorDirection>(i);
		const Sector* neighbor = getNeighbor(direction);
		if (neighbor != nullptr) {
			neighbor->fillBuildings(buildingEntities);
		}
	}

	return buildingEntities;
}


bool Sector::shouldNotifyAppearance(const go::Entity& entity) const
{
    gc::EntityController& controller =
        const_cast<gc::EntityController&>(entity.getController());
    return controller.queryAppearanceCallback() != nullptr;
}


bool Sector::canSectorUseable(const go::Entity& entity) const
{
	if (! isGlobalSector_) {
		return ! entity.isBuilding();
	}

	if (entity.isCreature() || entity.isAnchorOrBuilding()) {
		return true;
	}

	return false;
}

}} // namespace gideon { namespace zoneserver {
