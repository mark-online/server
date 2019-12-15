#include "ZoneServerPCH.h"
#include "WorldEventMission.h"
#include "callback/WorldEventScoreCallback.h"
#include "../../model/gameobject/ability/WorldEventable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/WorldEventCallback.h"
#include "../../world/WorldMap.h"
#include "../../world/World.h"
#include "../../world/region/WorldEventSpawnRegion.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "../spawn/SpawnService.h"
#include <gideon/cs/datatable/WorldEventMissionTable.h>
#include <boost/foreach_fwd.hpp>

namespace gideon { namespace zoneserver {

namespace {


/**
 * @class WorldEventMissionResultEvent
 */
class WorldEventMissionResultEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<WorldEventMissionResultEvent>
{
public:
    WorldEventMissionResultEvent(WorldEventCode worldEventCode,
        WorldEventMissionCode worldEventMissionCode,
        WorldEventMissionStateType stateType) :
        worldEventCode_(worldEventCode),
        worldEventMissionCode_(worldEventMissionCode),
        stateType_(stateType) {}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* callback = entity.getController().queryWorldEventCallback();
        if (callback) {
            callback->worldEventMissionResult(worldEventCode_, worldEventMissionCode_, stateType_);
        }
    }

private:
    WorldEventCode worldEventCode_;
    WorldEventMissionCode worldEventMissionCode_;
    WorldEventMissionStateType stateType_;
};


}

WorldEventMission::WorldEventMission(WorldMap& worldMap, WorldMapHelper& worldMapHelper,
    WorldEventScoreCallback& scoreCallback,
    const gdt::world_event_mission_t& worldEventMissionTemplate) :
    worldMap_(worldMap),
    worldMapHelper_(worldMapHelper),
    scoreCallback_(scoreCallback),
    worldEventMissionTemplate_(worldEventMissionTemplate),
    partyId_(invalidPartyId),
    waitExpireTime_(0)
{
}


WorldEventMission::~WorldEventMission()
{
}


bool WorldEventMission::initialize(const datatable::RegionTable& regionTable,
    const datatable::RegionCoordinates& regionCoordinates)
{
    if (! initRegion(regionTable, regionCoordinates)) {
        return false;
    }

    for (uint32_t i = 1; i <= worldEventMissionTemplate_.mission_phase_count(); ++i) {
        const datatable::WorldEventInvaderSpawnTable::Spawns* spawns =
            WORLD_EVENT_INVADER_SPAWN_TABLE->getSpawns(getMissionCode(), i);
        if (! spawns) {
            continue;
        }

        InvaderSpawnInfos infos;
        datatable::WorldEventInvaderSpawnTable::Spawns::const_iterator pos = spawns->begin();
        const datatable::WorldEventInvaderSpawnTable::Spawns::const_iterator end = spawns->end();
        for (; pos != end; ++pos) {
            infos.push_back(InvaderSpawnInfo((*pos)));
        }
        InvaderSpawnInfosMap_.emplace(i, infos);
    }

    currentMissionPhase_ = 1;

    return true;  
}


void WorldEventMission::ready()
{
    createPublicParty();
}


void WorldEventMission::activate()
{
    std::lock_guard<LockType> lock(lock_);

    phaseState_ = wemstPlay;
    missionState_ = wemstPlay;
    waitExpireTime_ = 0;
    currentMissionPhase_ = 1;
    entityInfoMap_.clear();
    spawnMapRegion_->startAiTask();
    spawnMissionEntity();
    initInvaderSpawnInfo(true);
}


void WorldEventMission::deactivate()
{
    if (spawnMapRegion_.get() != nullptr) {
        spawnMapRegion_->prefinalize();
        spawnMapRegion_->despawnNpcs();
    }

    {
        std::lock_guard<LockType> lock(lock_);

        entityInfoMap_.clear();
        phaseState_ = wemstWait;
        missionState_ = wemstWait;
        waitExpireTime_ = 0;
        initInvaderSpawnInfo(false);

        for (go::Entity* npc : respawnNpcs_) {
            const ErrorCode errorCode = npc->respawn(worldMap_);
            if (isFailed(errorCode)) {
                assert(false);
            }
        }
        respawnNpcs_.clear();
        destoryPublicParty();
    }
}


void WorldEventMission::update()
{
    std::lock_guard<LockType> lock(lock_);

    if (isPlayWorldEventMission(missionState_) || isFailWorldEventMission(missionState_)) {
        spawnInvaderEntity();
    }

    if (canChangeNextPhase()) {
        if (waitExpireTime_ == 0) {
            waitExpireTime_ = getTime() + worldEventMissionTemplate_.mission_phase_wait_time();
        }
        if (waitExpireTime_ < getTime()) {
            changeNextPhase();
        }
    }
}


void WorldEventMission::setParty(PartyId partyId)
{
    std::lock_guard<LockType> lock(lock_);

    partyId_ = partyId;
}



void WorldEventMission::setFailMission()
{
    std::lock_guard<LockType> lock(lock_);

    phaseState_ = wemstFailed;
    missionState_ = wemstFailed;
}


void WorldEventMission::enter(go::Entity& player)
{
    std::lock_guard<LockType> lock(lock_);

    if (playerMap_.find(player.getObjectId()) != playerMap_.end()) {
        playerMap_.erase(player.getObjectId());
    }
    playerMap_.emplace(player.getObjectId(), &player);
    player.getController().queryWorldEventCallback()->worldEventJoined(worldEventMissionTemplate_.world_event_code(),
        getMissionCode(), currentMissionPhase_, partyId_);

    const WorldEventMissionType missionType = 
        toWorldEventMissionType(worldEventMissionTemplate_.mission_type());    
    if ((missionType == wemtKillBoss) || (missionType == wemtKillAll)) {
        player.getController().queryWorldEventCallback()->worldEventKillMissionInfo(worldEventMissionTemplate_.world_event_code(),
            getMissionCode(), uint8_t(currentMissionPhase_), killMissionInfoMap_);
    }
}


void WorldEventMission::leave(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    playerMap_.erase(playerId);
}


void WorldEventMission::fillWorldEventMissionInfo(WorldEventMissionInfos& missionInfos) const
{
    missionInfos.push_back(WorldEventMissionInfo(getMissionCode(), missionState_));
}


WorldEventMissionCode WorldEventMission::getMissionCode() const 
{
    return worldEventMissionTemplate_.mission_code();
}


void WorldEventMission::spawnMissionEntity()
{    
    const datatable::WorldEventMissionSpawnTable::Spawns* spawns =
        WORLD_EVENT_MISSION_SPAWN_TABLE->getSpawns(getMissionCode(),
        currentMissionPhase_);
    if (spawns) {
        spawnMapRegion_->spawnEntities(entityInfoMap_, *this, *spawns);
    }

    const WorldEventMissionType missionType = 
        toWorldEventMissionType(worldEventMissionTemplate_.mission_type());    
    if ((missionType == wemtKillBoss) || (missionType == wemtKillAll)) {
        killMissionInfoMap_.clear();
        for (WorldEventEntityInfoMap::value_type& value : entityInfoMap_) {
            DataCode dataCode = value.second;
            WorldEventKillMissionInfoMap::iterator pos = killMissionInfoMap_.find(dataCode);
            if (pos != killMissionInfoMap_.end()) {
                WorldEventKillMissionInfo& killMission = (*pos).second;
                ++killMission.goalCount_;
            }
            else {
                WorldEventKillMissionInfo killMission(1, 0);
                killMissionInfoMap_.emplace(dataCode, killMission);
            }
        }        
    }
}


bool WorldEventMission::initRegion(const datatable::RegionTable& regionTable,
    const datatable::RegionCoordinates& regionCoordinates)
{
    RegionCode regionCode = worldEventMissionTemplate_.region_code();
    const datatable::RegionTemplate* regionTemplate = regionTable.getRegion(regionCode);
    if (! regionTemplate) {
        return false;
    }

    const datatable::RegionCoordinates::CoordinatesList* coordinates =
        regionCoordinates.getCoordinates(regionCode);
    if (! coordinates) {
        SNE_LOG_ERROR("Can't load region coordinate(R%u).", regionCode);
        return false;
    }

    spawnMapRegion_ = std::make_unique<WorldEventSpawnMapRegion>(worldMap_,
        worldMapHelper_, *regionTemplate, *coordinates);
    return true;
}


void WorldEventMission::initInvaderSpawnInfo(bool isAcitive)
{
    InvaderSpawnInfosMap::iterator pos = InvaderSpawnInfosMap_.find(currentMissionPhase_);
    if (pos != InvaderSpawnInfosMap_.end()) {
        InvaderSpawnInfos& invaderSpawnInfos = (*pos).second;
        for (InvaderSpawnInfo& invaderInfo : invaderSpawnInfos) {
            if (isAcitive) {
                invaderInfo.nextSpawnTime_ = getTime() + invaderInfo.spawnTemplate_->first_spawn_sec();        
            }
            else {
                invaderInfo.nextSpawnTime_ = 0;
            }
        }
    }
}


void WorldEventMission::spawnInvaderEntity()
{
    sec_t now = getTime();
    InvaderSpawnInfosMap::iterator pos = InvaderSpawnInfosMap_.find(currentMissionPhase_);
    if (pos != InvaderSpawnInfosMap_.end()) {
        InvaderSpawnInfos& invaderSpawnInfos = (*pos).second;
        for (InvaderSpawnInfo& invaderInfo : invaderSpawnInfos) {
            if (invaderInfo.nextSpawnTime_ != 0 && invaderInfo.nextSpawnTime_ <= now) {
                spawnMapRegion_->spawnInvaders(*this, *invaderInfo.spawnTemplate_);
                sec_t spawnTime = 
                    esut::random(invaderInfo.spawnTemplate_->min_spawn_sec(), invaderInfo.spawnTemplate_->max_spawn_sec());
                invaderInfo.nextSpawnTime_ = now + spawnTime; 
            }
        }   
    }
}


void WorldEventMission::createPublicParty()
{
    if (! isValidPartyId(partyId_)) {
        ZONE_SERVICE->getCommunityServerProxy().z2m_createWorldEventPublicParty(
            worldEventMissionTemplate_.world_event_code(), getMissionCode());
    }
}


void WorldEventMission::destoryPublicParty()
{
    if (isValidPartyId(partyId_)) {
        ZONE_SERVICE->getCommunityServerProxy().z2m_destoryWorldEventPublicParty(partyId_);
    }
    partyId_ = invalidPartyId;
}


void WorldEventMission::changeNextPhase()
{
    phaseState_ = wemstPlay;
    waitExpireTime_ = 0;
    ++currentMissionPhase_;
    playerMap_.clear();
    spawnMissionEntity();
    initInvaderSpawnInfo(true);
    notifyChangeMissionPhase();
}


void WorldEventMission::notifyChangeMissionPhase()
{
    std::lock_guard<LockType> lock(lock_);

    for (PlayerMap::value_type& value : playerMap_) {
        go::Entity* player = value.second;
        player->getController().queryWorldEventCallback()->worldEventMissionPhaseChanged(
            worldEventMissionTemplate_.world_event_code(),
            getMissionCode(), currentMissionPhase_);
    }
}


void WorldEventMission::nofifyKillMissionInfoUpdated(DataCode dataCode, uint8_t currentValue)
{
    std::lock_guard<LockType> lock(lock_);

    for (PlayerMap::value_type& value : playerMap_) {
        go::Entity* player = value.second;
        player->getController().queryWorldEventCallback()->worldEventKillMissionInfoUpdated(
            worldEventMissionTemplate_.world_event_code(),
            getMissionCode(), dataCode, currentValue);
    }
}


bool WorldEventMission::canChangeNextPhase() const
{
    if (isLastPhase() && ! isPlayWorldEventMission(missionState_)) {
        return false;
    }

    return phaseState_ == wemstComplete;
}


bool WorldEventMission::isLastPhase() const
{
    return worldEventMissionTemplate_.mission_phase_count() == currentMissionPhase_;
}


void WorldEventMission::killed(go::Entity* killer, CreatureLevel level, const GameObjectInfo& entityInfo)
{
    uint32_t score = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        if (isCompleteMission() || isFailMission()) {
            return;
        }

        DataCode entityCode = invalidDataCode;
        const WorldEventMissionType missionType = 
            toWorldEventMissionType(worldEventMissionTemplate_.mission_type());

        
        WorldEventEntityInfoMap::iterator pos = entityInfoMap_.find(entityInfo);
        if (pos == entityInfoMap_.end()) {
            score = 10;            
        }
        else {
            entityCode = (*pos).second;
            entityInfoMap_.erase(entityInfo);
        }
                    

        if ((missionType == wemtKillBoss) || (missionType == wemtKillAll)) {
            score = 10;            
            WorldEventKillMissionInfoMap::iterator killPos = killMissionInfoMap_.find(entityCode);
            if (killPos != killMissionInfoMap_.end()) {
                WorldEventKillMissionInfo& killInfo = (*killPos).second;
                ++killInfo.currentCount_;
                nofifyKillMissionInfoUpdated(entityCode, killInfo.currentCount_);
            }
            if (entityInfoMap_.empty()) {
                phaseState_ = wemstComplete;
            }
        }
        else if (missionType == wemtGuard) {
            uint32_t param1 = worldEventMissionTemplate_.mission_param_1();
            uint32_t param2 = worldEventMissionTemplate_.mission_param_2();
            if (entityCode == param1) {
                param2;
                phaseState_ = wemstFailed; 
            }
            else {
                score = 10;
            }
        }
        else if (missionType == wemtEscort) {
            uint32_t param1 = worldEventMissionTemplate_.mission_param_1();
            uint32_t param2 = worldEventMissionTemplate_.mission_param_2();
            if (entityCode == param1) {
                param2;
                phaseState_ = wemstFailed; 
            }
        }

        if (isFailWorldEventMission(phaseState_)) {
            missionState_ = wemstFailed; 
            auto event = std::make_shared<WorldEventMissionResultEvent>(
                worldEventMissionTemplate_.world_event_code(), getMissionCode(), wemstFailed);
            WORLD->broadcast(event);
        }
        else if (isCompleteWorldEventMission(phaseState_) && isLastPhase()) {
            missionState_ = wemstComplete;
            auto event = std::make_shared<WorldEventMissionResultEvent>(
                worldEventMissionTemplate_.world_event_code(), getMissionCode(), wemstComplete);
            WORLD->broadcast(event);
        }
    }

    if (killer && score > 0) {
        scoreCallback_.addScore(*killer, level);
    }
}


void WorldEventMission::pathNodeGoalArrived(go::Entity& npc)
{
    bool isMissionComplete = false;
    {
        std::lock_guard<LockType> lock(lock_);

        WorldEventEntityInfoMap::iterator pos = entityInfoMap_.find(npc.getGameObjectInfo());
        if (pos == entityInfoMap_.end()) {
            return;
        }
        const WorldEventMissionType missionType = 
            toWorldEventMissionType(worldEventMissionTemplate_.mission_type());
        if (missionType == wemtEscort) {
            uint32_t param1 = worldEventMissionTemplate_.mission_param_1();
            //uint32_t param2 = worldEventMissionTemplate_.mission_param_2();
            if (npc.getEntityCode()  == param1) {
                isMissionComplete = true;
                entityInfoMap_.erase(pos);
            }
        }     
        if (isCompleteWorldEventMission(phaseState_) && isLastPhase()) {
            missionState_ = wemstComplete;
            auto event = std::make_shared<WorldEventMissionResultEvent>(
                worldEventMissionTemplate_.world_event_code(), getMissionCode(), wemstComplete);
            WORLD->broadcast(event);
        }
    }

    if (isMissionComplete) {
        npc.despawn();
    }
}


void WorldEventMission::reserveRespawn(go::Entity& npc)
{
    std::lock_guard<LockType> lock(lock_);

    if (npc.isNpc()) {
        respawnNpcs_.push_back(&npc);
    }

}


void WorldEventMission::joinMission(go::Entity& npc, go::Entity& player)
{
    {
        std::lock_guard<LockType> lock(lock_);

        PlayerMap::iterator pos = playerMap_.find(player.getObjectId());
        if (pos != playerMap_.end()) {
            return;
        }        

        const float32_t joinDistanceSq = npc.queryWorldEventable()->getJoinDistanceSq();
        const float32_t distanceToTargetSq = npc.getSquaredLength(player.getPosition());
        if (joinDistanceSq <= 0) {
            return;
        }

        if (distanceToTargetSq > joinDistanceSq) {
            return;
        }        
    }

    enter(player);
}

}}  // namespace gideon { namespace zoneserver {
