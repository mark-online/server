#pragma once

#include "CreatureController.h"

namespace gideon { namespace zoneserver {
class WorldMap;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class NpcController
 * Controller for NPC
 */
class ZoneServer_Export NpcController : public CreatureController
{
    typedef CreatureController Parent;
public:
    NpcController();

protected:
    // = EntityController overriding
    virtual void spawned(WorldMap& worldMap);
    virtual void despawned(WorldMap& worldMap);

    // = CreatureController overriding
    virtual void died(go::Entity* from);

    // = InterestAreaCallback overriding
    virtual void entitiesAppeared(const go::EntityMap& entities);
    virtual void entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo);
    virtual void entitiesDisappeared(const go::EntityMap& entities);
    virtual void entityDisappeared(go::Entity& entity);
    virtual void entityDisappeared(const GameObjectInfo& info);

private:
    void rewardExp(go::Entity& killEntity);
    void processQuest(go::Entity& killEntity);
    void processWorldEvent(go::Entity* killEntity);
    
private:
    bool isDieFromInvader_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
