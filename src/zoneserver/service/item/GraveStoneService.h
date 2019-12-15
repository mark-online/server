#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/GraveStoneInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>
#include <sne/core/container/Containers.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver {

namespace go {
class GraveStone;
class Entity;
class Npc;
class Player;
} // namespace go

class EntityPool;
class WorldMap;

/**
 * @class GraveStoneService
 *
 * 스킬 서비스
 */
class ZoneServer_Export GraveStoneService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(GraveStoneService);

    typedef std::mutex LockType;
    typedef sne::core::HashMap<ObjectId, go::GraveStone*> GraveStoneMap;

public:
    GraveStoneService();
    ~GraveStoneService();

    bool initialize(bool resultSkip);

public:
    /// NPC 무덤 생성
    go::GraveStone* registerGraveStone(const go::Npc& npc, WorldMap& worldMap,
        go::Entity* from);

    /// Player 무덤 생성
	go::GraveStone* registerGraveStone(go::Entity& player, WorldMap& worldMap);

    void unregisterGraveStone(go::GraveStone& graveStone);

    sne::base::Future::Ref scheduleGraveStoneDespawnTask(go::GraveStone& creature, bool isCreated);
private:
	bool addGraveStone(go::GraveStone* graveStone);
	void removeGraveStone(go::GraveStone* graveStone);

private:
    const GameTime createDespawnTime_;
	const GameTime clickDespawnTime_;
	const HitPoint minHitPoint_;
	const HitPoint maxHitPoint_;

    std::unique_ptr<EntityPool> graveStonePool_;

	GraveStoneMap graveStoneMap_;
    bool resultSkip_;

	mutable LockType lock_;
};

}} // namespace gideon { namespace zoneserver {

#define GRAVE_STONE_SERVICE gideon::zoneserver::GraveStoneService::instance()
