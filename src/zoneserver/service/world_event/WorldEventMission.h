#pragma once

#include "../../zoneserver_export.h"
#include "callback/WorldEventMissionCallback.h"
#include <gideon/cs/shared/data/WorldEventInfo.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionSpawnTable.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gdt {
class world_event_mission_t;
class world_event_invader_spawn_t;
} // namespace gdt {

namespace gideon { namespace zoneserver {


class WorldMap;
class WorldMapHelper;
class WorldEventSpawnMapRegion;
class WorldEventScoreCallback;

/***
 * @class WorldEventMission
 ***/
class WorldEventMission : public WorldEventMissionCallback
{
    typedef std::mutex LockType;

    struct InvaderSpawnInfo
    {
        const gdt::world_event_invader_spawn_t* spawnTemplate_;
        sec_t nextSpawnTime_;

        InvaderSpawnInfo(const gdt::world_event_invader_spawn_t* spawnTemplate = nullptr,
            sec_t nextSpawnTime = 0) :
            spawnTemplate_(spawnTemplate),
            nextSpawnTime_(nextSpawnTime)
        {
        }
    };


    typedef sne::core::Vector<InvaderSpawnInfo> InvaderSpawnInfos;
    typedef sne::core::HashMap<uint32_t, InvaderSpawnInfos> InvaderSpawnInfosMap;
    typedef sne::core::HashMap<ObjectId, go::Entity*> PlayerMap;
    typedef sne::core::Vector<go::Entity*> Npcs;

public:
    WorldEventMission(WorldMap& worldMap, WorldMapHelper& worldMapHelper,
        WorldEventScoreCallback& scoreCallback,
        const gdt::world_event_mission_t& worldEventTemplate);
    ~WorldEventMission();

    bool initialize(const datatable::RegionTable& regionTable,
        const datatable::RegionCoordinates& regionCoordinates);

    void ready();
    void activate();
    void deactivate();

    void update();    
    void setParty(PartyId partyId);
    void setFailMission();

    void enter(go::Entity& player);
    void leave(ObjectId playerId);
    void fillWorldEventMissionInfo(WorldEventMissionInfos& missionInfos) const;


public:
    WorldEventMissionStateType getWorldEventMissionStateType() const {
        return missionState_;
    }

    bool isCompleteMission() const {
        return isCompleteWorldEventMission(missionState_);
    }
    bool isFailMission() const {
        return isFailWorldEventMission(missionState_);
    }

    bool hasParty() const {
        return isValidPartyId(partyId_);
    }
    
    WorldEventMissionCode getMissionCode() const;

private:
    void spawnMissionEntity();
    void spawnInvaderEntity(); 
    bool initRegion(const datatable::RegionTable& regionTable,
        const datatable::RegionCoordinates& regionCoordinates);
    void initInvaderSpawnInfo(bool isAcitive);

    void createPublicParty();
    void destoryPublicParty();
    void changeNextPhase();

    void notifyChangeMissionPhase();
    void nofifyKillMissionInfoUpdated(DataCode dataCode, uint8_t currentValue);
    
    bool canChangeNextPhase() const;
    bool isLastPhase() const;

private:
    // = WorldEventMissionCallback overrding
    virtual void killed(go::Entity* killer, CreatureLevel level, const GameObjectInfo& entityInfo);
    virtual void pathNodeGoalArrived(go::Entity& npc);
    virtual void reserveRespawn(go::Entity& npc);
    virtual void joinMission(go::Entity& npc, go::Entity& player);

private:
    mutable LockType lock_;
    mutable LockType playerLock_;
    const gdt::world_event_mission_t& worldEventMissionTemplate_;

    WorldMap& worldMap_;
    WorldMapHelper& worldMapHelper_;
    WorldEventScoreCallback& scoreCallback_;

    std::unique_ptr<WorldEventSpawnMapRegion> spawnMapRegion_;
    PlayerMap playerMap_;
    Npcs respawnNpcs_; // 월드맵에서 죽은 NPC
    InvaderSpawnInfosMap InvaderSpawnInfosMap_;
    WorldEventEntityInfoMap entityInfoMap_;
    
    uint32_t currentMissionPhase_;
    PartyId partyId_;
    sec_t waitExpireTime_;
    WorldEventMissionStateType phaseState_;   
    WorldEventMissionStateType missionState_;   

    
    WorldEventKillMissionInfoMap killMissionInfoMap_;
};

}} // namespace gideon { namespace zoneserver {
