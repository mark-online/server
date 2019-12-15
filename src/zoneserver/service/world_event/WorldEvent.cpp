#include "ZoneServerPCH.h"
#include "WorldEvent.h"
#include "WorldEventPhase.h"
#include "callback/WorldEventPlayCallback.h"
#include "state/WorldEventWaitState.h"
#include "state/WorldEventReadyState.h"
#include "state/WorldEventPlayState.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../world/WorldMapHelper.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/WorldEventCallback.h"
#include "../../model/gameobject/Creature.h"
#include "../time/GameTimer.h"
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionSpawnTable.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>
#include <gideon/cs/datatable/WorldEventTable.h>
#include <gideon/cs/datatable/WorldEventMissionTable.h>
#include <boost/foreach_fwd.hpp>

namespace gideon { namespace zoneserver {

namespace {

uint32_t getCalcRewardValue(uint32_t ranking, float32_t rewardValue)
{
    switch (ranking) {
    case 1:
        return static_cast<uint32_t>(rewardValue * 0.3f);
    case 2:
        return static_cast<uint32_t>(rewardValue * 0.2f);
    case 3:
    case 4:
        return static_cast<uint32_t>(rewardValue * 0.15f);
    case 5:
        return static_cast<uint32_t>(rewardValue * 0.1f);
    case 6:
    case 7:
        return static_cast<uint32_t>(rewardValue * 0.05f);
    }
    return static_cast<uint32_t>(rewardValue * 0.01f);
}

/**
 * @class WorldEventTopRankerEvent
 */
class WorldEventTopRankerEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<WorldEventTopRankerEvent>
{
public:
    WorldEventTopRankerEvent(WorldEventCode worldEventCode,
        const RankingRewardInfos& rakingRewardInfos) :
        worldEventCode_(worldEventCode),
        rakingRewardInfos_(rakingRewardInfos) {}

private:
    virtual void call(go::Entity& entity) {
        gc::WorldEventCallback* callback = entity.getController().queryWorldEventCallback();
        if (callback) {
            callback->worldEventTopRankerRewardInfo(worldEventCode_, rakingRewardInfos_);
        }
    }

private:
    const WorldEventCode worldEventCode_;
    RankingRewardInfos rakingRewardInfos_;
};

} // namespace

WorldEvent::WorldEvent(WorldMap& worldMap, WorldMapHelper& worldMapHelper,
    const gdt::world_event_t& worldEventTemplate) :
    worldMap_(worldMap),
    worldMapHelper_(worldMapHelper),
    worldEventTemplate_(worldEventTemplate)
{
}


WorldEvent::~WorldEvent()
{
    for (EventPhaseMap::value_type& value : eventPhaseMap_) {
        delete value.second;
    }
    for (StateMap::value_type& value : stateMap_) {
        delete value.second;
    }
}


bool WorldEvent::initialize()
{
    MapCode mapCode = worldMap_.getMapCode();
    const datatable::RegionTable* regionTable = worldMapHelper_.getRegionTable(mapCode);
    if (! regionTable) {
        SNE_LOG_ERROR("Can't load region(M%u).", mapCode);
        return false;
    }

    const datatable::RegionCoordinates* regionCoordinates = worldMapHelper_.getRegionCoordinates(mapCode);
    if (! regionCoordinates) {
        SNE_LOG_ERROR("Can't load region coordinates(M%u).", mapCode);
        return false;
    }

    if (! initWorldEventPhase(*regionTable, *regionCoordinates)) {
        return false;
    }
    
    initWorldEventState();

    return true;
}


void WorldEvent::setInitWaitState()
{
    std::lock_guard<LockType> lock(lock_);

    currentState_ = stateMap_[WorldEventState::stWait];
    currentState_->reInitialize();
}


void WorldEvent::fillWorldEventInfo(WorldEventInfos& worldEventInfos) const
{
    std::lock_guard<LockType> lock(lock_);

    currentState_->fillWorldEventInfo(worldEventInfos);
}


void WorldEvent::fillWorldEventRankerInfo(WorldEventRankerInfos& infos,
    uint32_t& myScore, ObjectId playerId) const
{
    std::lock_guard<LockType> lock(lock_);

    myScore = 0;
    const PlayerScoreInfo* myInfo = getPlayerScoreInfo(playerId);
    if (myInfo) {
        myScore = myInfo->score_;
    }

    fillWorldEventRankerInfo_i(infos);
}


void WorldEvent::update()
{
    std::lock_guard<LockType> lock(lock_);

    if (currentState_->shouldNextState()) {
        currentState_->changeNextState();
        currentState_ = stateMap_[currentState_->getNextStateType()];
        currentState_->reInitialize();
    }

    currentState_->update(); 
}


void WorldEvent::enter(go::Entity& player, RegionCode regionCode)
{
    std::lock_guard<LockType> lock(lock_);

    currentState_->enter(player, regionCode);
}


void WorldEvent::leave(ObjectId playerId, RegionCode regionCode)
{
    std::lock_guard<LockType> lock(lock_);

    currentState_->leave(playerId, regionCode);
}


void WorldEvent::login(go::Entity& player)
{    
    bool shuldReward = false;
    {
        std::lock_guard<LockType> lock(lock_);

        shuldReward = currentState_->canEventRewardForLoginPlayer();
    }

    std::lock_guard<LockType> lock(playerLock_);

    if (shuldReward) {
        PlayerScoreInfo* playerScoreInfo = getPlayerScoreInfo(player.getObjectId());
        if (! playerScoreInfo) {
            return;
        }
        rewardPlayer(player);
        playerScoreInfoMap_.erase(player.getObjectId());
    }
    else {
        PlayerScoreInfo* playerScoreInfo = getPlayerScoreInfo(player.getObjectId());
        if (! playerScoreInfo) {
            return;
        }
        playerScoreInfo->player_ = &player;
    }
     
}


void WorldEvent::logout(ObjectId playerId)
{
    for (EventPhaseMap::value_type& value : eventPhaseMap_) {
        WorldEventPhase* phase = value.second;
        phase->leave(playerId);        
    }

    {
        std::lock_guard<LockType> lock(playerLock_);

        PlayerScoreInfo* info = getPlayerScoreInfo(playerId);
        if (info) {
            info->player_ = nullptr;
        }
    }
}


void WorldEvent::cheatOpen()
{
    std::lock_guard<LockType> lock(lock_);

    if (currentState_->getStateType() == WorldEventState::stWait) {
        currentState_->changeNextState();
        currentState_ = stateMap_[currentState_->getNextStateType()];
        currentState_->reInitialize();
    }
}


void WorldEvent::cheatClose()
{
    std::lock_guard<LockType> lock(lock_);

    if (currentState_->getStateType() == WorldEventState::stPlay) {
        currentState_->changeNextState();
        currentState_ = stateMap_[currentState_->getNextStateType()];
        currentState_->reInitialize();
    }
}


void WorldEvent::setParty(WorldEventMissionCode missionCode, PartyId partyId)
{
    std::lock_guard<LockType> lock(lock_);

    currentState_->setParty(missionCode, partyId);
}


bool WorldEvent::isPlaying() const
{
    std::lock_guard<LockType> lock(lock_);

    return currentState_->getStateType() == WorldEventState::stPlay;
}


bool WorldEvent::isReady() const
{
    std::lock_guard<LockType> lock(lock_);

    return currentState_->getStateType() == WorldEventState::stReady;
}


bool WorldEvent::isWaiting() const
{
    std::lock_guard<LockType> lock(lock_);

    return currentState_->getStateType() == WorldEventState::stWait;
}


bool WorldEvent::isUniqueEvent() const
{
    return worldEventTemplate_.unique_event() != 0;
}


PlayerScoreInfo* WorldEvent::getPlayerScoreInfo(ObjectId playerId)
{
    PlayerScoreInfoMap::iterator pos = playerScoreInfoMap_.find(playerId);
    if (pos != playerScoreInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


const PlayerScoreInfo* WorldEvent::getPlayerScoreInfo(ObjectId playerId) const
{
    PlayerScoreInfoMap::const_iterator pos = playerScoreInfoMap_.find(playerId);
    if (pos != playerScoreInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


uint32_t WorldEvent::getRewardValue(ObjectId playerId) const
{
    std::lock_guard<LockType> lock(playerLock_);

    uint32_t rewardValue = static_cast<uint32_t>(worldEventTemplate_.reward_cost_value() * 0.01f);

    TopRankerInfoSet::const_iterator pos = topRankerInfoSet_.begin();
    const TopRankerInfoSet::const_iterator end = topRankerInfoSet_.end();
    for (; pos != end; ++pos) {
        const TopRankerInfo& rankerInfo = *pos;
        if (rankerInfo.playerId_ == playerId) {
            rewardValue = rankerInfo.rewardValue_; 
        }
    }

    return rewardValue;
}

void WorldEvent::initWorldEventState()
{
    WorldEventState* waitState = new WorldEventWaitState(*this, worldEventTemplate_);
    WorldEventState* readyState = new WorldEventReadyState(eventPhaseMap_);
    WorldEventState* playState = new WorldEventPlayState(*this, worldEventTemplate_.world_event_code(),
        eventPhaseMap_, worldEventTemplate_.close_min() * 60, 
        worldEventTemplate_.fail_min() * 60, 
        worldEventTemplate_.event_wait_sec());
    stateMap_.emplace(WorldEventState::stWait, waitState);
    stateMap_.emplace(WorldEventState::stReady, readyState);
    stateMap_.emplace(WorldEventState::stPlay, playState);
    currentState_ = waitState;
}


bool WorldEvent::initWorldEventPhase(const datatable::RegionTable& regionTable,
    const datatable::RegionCoordinates& regionCoordinates)
{
    const datatable::WorldEventMissionTable::WorldEventPhaseMap* phaseMap =
        WORLD_EVENT_MISSION_TABLE->getWorldEventPhaseMap(worldEventTemplate_.world_event_code());
    if (! phaseMap) {
        return false;
    }

    datatable::WorldEventMissionTable::WorldEventPhaseMap::const_iterator pos = phaseMap->begin();
    const datatable::WorldEventMissionTable::WorldEventPhaseMap::const_iterator end = (*phaseMap).end();
    for (; pos != end; ++pos) {
        const datatable::WorldEventMissionTable::WorldEventMissions& missions = (*pos).second;
        uint32_t phase = (*pos).first;
        WorldEventPhase* eventPhase = new WorldEventPhase();
        if (! eventPhase->initialize(regionTable, regionCoordinates, missions, worldMap_, worldMapHelper_, *this)) {
            SNE_LOG_ERROR("Can't WorldEvent Mission Phase(M%u, S%u).",
                uint32_t(worldEventTemplate_.world_event_code()), phase);
            return false;
        }

        eventPhaseMap_.emplace(phase, eventPhase);
    }

    return true;
}


void WorldEvent::updateRanker(RankerInfo rankerInfo, uint32_t newscore)
{
    std::lock_guard<LockType> lock(playerLock_);

    RankerInfoSet::iterator pos = rankerInfoSet_.begin();
    RankerInfoSet::iterator end = rankerInfoSet_.end();
    for (;pos != end; ++pos) {
        const RankerInfo& info = (*pos);
        if (info.playerId_ == rankerInfo.playerId_) {
            rankerInfoSet_.erase(pos);
            break;
        }
    }
    rankerInfo.score_ = newscore;
    rankerInfoSet_.insert(rankerInfo);
    makeTopRanker();
}


void WorldEvent::fillWorldEventRankerInfo_i(WorldEventRankerInfos& infos) const
{
    for (const TopRankerInfoSet::value_type& value : topRankerInfoSet_) {
        const TopRankerInfo& topRankerInfo = value; 
        Nickname nickname;
        const PlayerScoreInfo* playerInfo = getPlayerScoreInfo(topRankerInfo.playerId_);
        if (playerInfo) {
            nickname = playerInfo->nickname_;
        }
        infos.push_back(WorldEventRankerInfo(topRankerInfo.playerId_, topRankerInfo.score_, 
            static_cast<uint8_t>(topRankerInfo.ranking_), nickname));
    }
}


void WorldEvent::makeTopRanker()
{
    TopRankerInfoSet topRankerInfoSet;
    uint8_t ranking = 0;
    topRankerInfoSet_.clear();
    RankerInfoSet::iterator pos1 = rankerInfoSet_.begin();
    RankerInfoSet::iterator end1 = rankerInfoSet_.end();
    for (; pos1 != end1; ++pos1) {
        const RankerInfo& rankerInfo = *pos1;        
        const PlayerScoreInfo* playerInfo = getPlayerScoreInfo(rankerInfo.playerId_);
        if (! playerInfo) {
            continue;
        }
        ++ranking;
        topRankerInfoSet.insert(TopRankerInfo(rankerInfo.playerId_, rankerInfo.score_, ranking));            
        if (rewardRanking <= ranking) {
            break;
        }
    }

    uint32_t topRanking = rewardRanking;
    TopRankerInfoSet::const_iterator pos2 = topRankerInfoSet.begin();
    TopRankerInfoSet::const_iterator end2 = topRankerInfoSet.end();
    for (; pos2 != end2; ++pos2) {
        const TopRankerInfo& rankerInfo = *pos2;  
        uint32_t rewardValue = getCalcRewardValue(topRanking, 
            static_cast<float32_t>(worldEventTemplate_.reward_cost_value()));
        topRankerInfoSet_.insert(TopRankerInfo(rankerInfo.playerId_, rankerInfo.score_, topRanking, rewardValue));
        --topRanking;
    }
}


void WorldEvent::rewardPlayer(go::Entity& player)
{
    player.getController().queryWorldEventCallback()->worldEventRewarded(worldEventTemplate_.world_event_code(),
        toCostType(worldEventTemplate_.reward_cost_type()), 
        getRewardValue(player.getObjectId()));
}



// = WorldEventScoreCallback overriding

void WorldEvent::addScore(go::Entity& killer, CreatureLevel monsterLevel)
{
    int score = monsterLevel - static_cast<go::Creature&>(killer).getCreatureLevel() + 
        worldEventTemplate_.max_level();
    score = score <= 0 ? 1 : score * 2;

    {
        std::lock_guard<LockType> lock(playerLock_);

        PlayerScoreInfo* info = getPlayerScoreInfo(killer.getObjectId());
        if (info) {
            RankerInfo rankerInfo(killer.getObjectId(), info->score_, GAME_TIMER->msec());

            uint32_t newscore = info->score_ + score;
            if (worldEventTemplate_.min_ranking_score() <= newscore) {
                updateRanker(rankerInfo, newscore);
            }
            info->score_ = newscore;
        }
        else {
            PlayerScoreInfo addInfo(&killer, score, killer.getNickname());
            playerScoreInfoMap_.emplace(killer.getObjectId(), addInfo);
        }
    }    
}


void WorldEvent::resetScore()
{
    std::lock_guard<LockType> lock(playerLock_);

    playerScoreInfoMap_.clear();
    topRankerInfoSet_.clear();
    rankerInfoSet_.clear();
}


void WorldEvent::reward(bool isCompleteEvent)
{
    PlayerScoreInfoMap rewarder;
    RankingRewardInfos rankerRewardInfos;
    {
        std::lock_guard<LockType> lock(playerLock_);

        if (isCompleteEvent) {
            for (const TopRankerInfoSet::value_type& value : topRankerInfoSet_) {
                const TopRankerInfo& topRankerInfo = value;          
                const PlayerScoreInfo* playerScore = getPlayerScoreInfo(topRankerInfo.playerId_);
                if (playerScore) {
                    rankerRewardInfos.push_back(RankingRewardInfo(playerScore->nickname_, topRankerInfo.rewardValue_));
                }
            }
        }        

        PlayerScoreInfoMap::iterator pos2 = playerScoreInfoMap_.begin();
        while (pos2 != playerScoreInfoMap_.end()) {
            const PlayerScoreInfo& scoreInfo = (*pos2).second;
            if (! scoreInfo.player_) {
                ++pos2;
                continue;
            }
            rewarder.emplace(scoreInfo.player_->getObjectId(), scoreInfo);
            pos2 = playerScoreInfoMap_.erase(pos2);
        }    
    }

    if (! rankerRewardInfos.empty()) {
        auto event = std::make_shared<WorldEventTopRankerEvent>(worldEventTemplate_.world_event_code(),
            rankerRewardInfos);
        WORLD->broadcast(event);
    }
    
    for (PlayerScoreInfoMap::value_type& value : rewarder) {
        PlayerScoreInfo& scoreInfo = value.second;
        rewardPlayer(*scoreInfo.player_);
    }
}

}}  // namespace gideon { namespace zoneserver {
