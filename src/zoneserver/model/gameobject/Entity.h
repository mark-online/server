#pragma once

#include "../../zoneserver_export.h"
#include "GameObject.h"
#include "EntityEvent.h"
#include "ability/EntityAbility.h"
#include "ability/Positionable.h"
#include "ability/Summonable.h"
#include "../state/EntityStateAbility.h"
#include "../../world/SectorAddress.h"
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/UnionEntityInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Rate.h>
#include <gideon/cs/datatable/SpawnTemplate.h>
#include <sne/base/concurrent/Future.h>

namespace gdt {
class entity_path_t;
} // namespace gdt {

namespace gideon { namespace zoneserver {
class WorldMap;
class MapRegion;
class CastChecker;
class EffectHelper;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace gc {
class EntityController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

class EffectScriptApplier;

/**
 * @class Entity
 * 월드 상에 존재하는(눈에 보이는) 객체의 부모 클래스
 */
class ZoneServer_Export Entity : public GameObject,
    public EntityAbility,
    public EntityStateAbility,
    public Positionable,
    protected Summonable
{
    typedef GameObject Parent;

public:
    typedef std::recursive_mutex LockType;

public:
    Entity(std::unique_ptr<gc::EntityController> controller);
    virtual ~Entity();

public:
    virtual bool initialize(ObjectType objectType, ObjectId objectId);
    virtual void finalize();

public:
    virtual ErrorCode spawn(WorldMap& worldMap, const ObjectPosition& spawnPosition);
    virtual ErrorCode respawn(WorldMap& worldMap);
    virtual ErrorCode despawn();

    virtual void reserveRespawn(WorldMap& worldMap);
    virtual void reserveDespawn();

public:
    virtual ObjectPosition getNextSpawnPosition() const;
    virtual float32_t getModelingRadiusSize() const {
        return 0.3f;
    }

public:
    EffectScriptApplier& getEffectScriptApplier() {
        return (*effectApplier_.get());
    }

    const EffectScriptApplier& getEffectScriptApplier() const {
        return (*effectApplier_.get());
    }

    virtual std::unique_ptr<EffectScriptApplier> createEffectScriptApplier();

protected:
    virtual std::unique_ptr<EffectHelper> createEffectHelper();

public:
    virtual DataCode getEntityCode() const {
        return invalidDataCode;
    }
    
    virtual const Nickname& getNickname() const {
        assert(false && "Not Call");
        static const Nickname nullName;
        return nullName;	
    }

    virtual AccountId getAccountId() const {
        assert(false && "Not Call");
        return invalidAccountId;
    }

public:
    /// 내가 target을 공격할 때의 레벨 차이에 따른 보너스
    virtual bips_t getLevelBonus(const Entity& target) const {
        target;
        return 0;
    }

    virtual bips_t getMissChance(const Entity& target) const {
        target;
        return 0;
    }

    virtual bips_t getDodgeChance() const {
        return 0;
    }

    virtual bips_t getParryChance() const {
        return 0;
    }

    virtual bips_t getBlockChance() const {
        return 0;
    }

    virtual bips_t getResistChance(AttributeRateType attributeType) const {
        attributeType;
        return 0;
    }

    virtual bips_t getPhysicalCriticalChance() const {
        return 0;
    }

    virtual bips_t getMagicCriticalChance() const {
        return 0;
    }

public:
    void registerObserver(const GameObjectInfo& entityInfo);
    void unregisterObserver(const GameObjectInfo& entityInfo);

    /// observer와 observer의 observer에게 이벤트 통지
    void notifyToOberversOfObservers(go::EntityEvent::Ref event, bool exceptSelf = false);

    /// observer들에게 이벤트 통지
    void notifyToObervers(go::EntityEvent::Ref event, bool exceptSelf = false);

public:
    void setMapRegion(MapRegion& mapRegion, int regionSequence = 0);
    const MapRegion& getMapRegion() const;
    MapRegion& getMapRegion();
    int getRegionSequence() const {
        return regionSequence_;
    }

public:
    void setSpawnTemplate(const SpawnTemplate& spawnTemplate) {
        spawnTemplate_ = spawnTemplate;
    }

    const SpawnTemplate& getSpawnTemplate() const {
        return spawnTemplate_;
    }

    const CastChecker& getCastChecker() const {
        return *castChecker_.get();
    }

    EffectHelper& getEffectHelper() {
        return *effectHelper_.get();
    }
    
public:
    void setHomePosition(const ObjectPosition& position);

    ObjectPosition getHomePosition() const;

    /// 스폰한 위치 근처에 있는가?
    bool isAtSpawnLocation(float32_t delta) const {
        return getSquaredLength(getHomePosition()) < (delta * delta);
    }

public:
    const UnionEntityInfo& getUnionEntityInfo() const;

    /// 스킬 효과를 받을 수 있는가? (상대가 스킬을 쏠 수 있는가?)
    bool isEffectInfoable() const;

public:
    /// position과의 거리를 제곱미터 단위로 구한다
    float32_t getSquaredLength(const Position& position) const;

    float32_t getLength(const Position& position) const {
        return sqrtf(getSquaredLength(position));
    }

    /// 평면 상에서 (x,y)를 바라보는 각도를 구한다
    float32_t getAngle(float32_t x, float32_t y) const;

    float32_t getAngle(const go::Entity& entity) const {
        const ObjectPosition position = entity.getPosition();
        return getAngle(position.x_, position.y_);
    }

public:
    void setPath(const gdt::entity_path_t* path) {
        entityPath_ = path;
        isPathForward_ = true;
    }

    void setPathForward(bool forward) {
        isPathForward_ = forward;
    }

    void setCurrentPathIndex(int currentPathIndex) {
        currentPathIndex_ = currentPathIndex;
    }

    bool hasPath() const;

    const gdt::entity_path_t* getPath() const {
        return entityPath_;
    }

    int getNearestPathNodeIndex() const;

    bool isPathForward() const {
        return isPathForward_;
    }

    int getCurrentPathIndex() const {
        return currentPathIndex_;
    }

    int getLastPathIndex() const;

    bool isLastPathNode(int index) const {
        if (index < 0) {
            return false;
        }
        const int lastNodeIndex = getLastPathIndex();
        if (lastNodeIndex < 0) {
            return false;
        }
        return index == lastNodeIndex;
    }

    ObjectType getDestoryObjectType() const {
        return entityDestoryObjectType_;
    }

public:
    void playAction(uint32_t actionCode);

public:
    gc::EntityController& getController() {
        return *controller_;
    }

    const gc::EntityController& getController() const {
        return *controller_;
    }

    template <typename ControllerT>
    ControllerT& getControllerAs() {
        return static_cast<ControllerT&>(*controller_);
    }

    template <typename ControllerT>
    const ControllerT& getControllerAs() const {
        return static_cast<const ControllerT&>(*controller_);
    }

public:
    LockType& getLock() {
        return lockThis_;
    }

    LockType& getLock() const {
        return lockThis_;
    }

    void fillObserverSet(EntitySet& entitySet, const GameObjectInfo& exceptTarget) const;

protected:
    LockType& getLockPositionable() {
        return lockPositionable_;
    }

    LockType& getLockPositionable() const {
        return lockPositionable_;
    }

protected:
    UnionEntityInfo& getUnionEntityInfo_i() {
        return unionEntityInfo_;
    }

    const UnionEntityInfo& getUnionEntityInfo_i() const {
        return unionEntityInfo_;
    }

private:
    void despawned();
    void resetObservers();

public:
    // = EntityAbility overriding
    virtual Positionable* queryPositionable() { return this; }
    virtual Summonable* querySummonable() { return this; }

public:
    // = Positionable overriding
    virtual void setWorldMap(WorldMap& worldMap);
    virtual WorldMap* getCurrentWorldMap();
    virtual const WorldMap* getCurrentWorldMap() const;

    virtual bool isEnteredAtWorld() const;

    virtual void setSectorAddress(const SectorAddress& address);
    virtual SectorAddress getSectorAddress() const;
    virtual void setGlobalSectorAddress(const SectorAddress& address);
    virtual SectorAddress getGlobalSectorAddress() const;

    virtual void setPosition(const ObjectPosition& position);
    virtual void setHeading(Heading heading);

    virtual ObjectPosition getPosition() const;

protected:
    // = Summonable overriding
    virtual void setSummoner(go::Entity& summoner, SpawnType spawnType);
    virtual void unsetSummoner();

    virtual void addSummon(const GameObjectInfo& summon);
    virtual void removeSummon(const GameObjectInfo& summon);

    virtual EntitySet getSummons() const;
    virtual GameObjectInfo getSummoner() const;

private:
    std::unique_ptr<gc::EntityController> controller_;
    std::unique_ptr<EffectScriptApplier> effectApplier_;
    std::unique_ptr<CastChecker> castChecker_;
    std::unique_ptr<EffectHelper> effectHelper_;

    EntitySet observerEntitySet_;
    ObjectType entityDestoryObjectType_;

    SectorAddress sectorAddress_;
    SectorAddress globalSectorAddress_;

    WorldMap* currentWorldMap_;
    MapRegion* currentMapRegion_;
    int regionSequence_;

    GameObjectInfo summoner_;
    EntitySet summons_;

    UnionEntityInfo unionEntityInfo_;

    ObjectPosition homePosition_; //< 부활지 & 귀환지
    SpawnTemplate spawnTemplate_;

    const gdt::entity_path_t* entityPath_;
    bool isPathForward_;
    int currentPathIndex_;

    sne::base::Future::WeakRef respawnTask_;
    sne::base::Future::WeakRef despawnTask_;

    mutable LockType lockThis_;
    mutable LockType lockPositionable_;
};


/**
 * @class EntityMap
 * forward decl.을 위해 클래스로 만듦
 */
class EntityMap : public sne::core::Map<GameObjectInfo, go::Entity*>
{
    typedef sne::core::Map<GameObjectInfo, go::Entity*> Parent;
public:
    void insertEntity(go::Entity& entity) {
        const GameObjectInfo& key = entity.getGameObjectInfo();
        if (! key.isValid()) {
            return;
        }
        assert(find(key) == end());
        emplace(key, &entity);
    }

    void removeEntity(go::Entity& entity) {
        assert(entity.isValid());
        erase(entity.getGameObjectInfo());
    }

public:
    bool hasPlayer() const {
        for (const Parent::value_type& value : *this) {
            const go::Entity* entity = value.second;
            if (entity->isPlayer()) {
                return true;
            }
        }
        return false;
    }
};

}}} // namespace gideon { namespace zoneserver { namespace go {
