#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/WorldEventInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/base/concurrent/Runnable.h>
#include <sne/base/concurrent/Future.h>
#include <sne/core/utility/Singleton.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {

class WorldMapHelper;
class WorldMap;
class WorldEvent;

/**
 * @class WorldEventService
 *
 * 월드 서비스
 */
class ZoneServer_Export WorldEventService :
    private sne::base::Runnable
{
    SNE_DECLARE_SINGLETON(WorldEventService);

    typedef std::mutex LockType;
    typedef sne::core::Map<WorldEventCode, std::unique_ptr<WorldEvent>> WorldEventMap;

public:
    WorldEventService();
    ~WorldEventService();

    bool initialize(WorldMap& worldMap, WorldMapHelper& worldMapHelper);
    void login(go::Entity& player);
    void logout(ObjectId playerId);

    void fillWorldEventInfos(WorldEventInfos& worldEventInfos);
    WorldEvent* getWorldEvent(WorldEventCode worldEventCode);

private:
    void startEventTask();
    void stopEventTask();

private:
    // = sne::base::Runnable overriding
    virtual void run();

private:
    mutable LockType lock_;
    sne::base::Future::WeakRef eventTask_;
    GameTime lastUpdateTime_;
    WorldEventMap worldEventMap_;
    WorldEvent* uniquePlayEvent_;
};

}} // namespace gideon { namespace zoneserver {


#define WORLD_EVENT_SERVICE gideon::zoneserver::WorldEventService::instance()
