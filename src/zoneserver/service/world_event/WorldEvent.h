#pragma once

#include "../../zoneserver_export.h"
#include "state/WorldEventState.h"
#include "callback/WorldEventScoreCallback.h"
#include "callback/WorldEventPlayCallback.h"
#include <gideon/cs/shared/data/WorldEventInfo.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gdt {
class world_event_t;
} // namespace gideon { namespace datatable {

namespace gideon { namespace datatable {
class RegionTable;
class RegionCoordinates;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

class WorldMap;
class WorldMapHelper;
class WorldEventPhase;
class WorldEventState;

namespace go {
class Entity;
} // namespace go


/***
 * @class WorldEvent
 ***/
class ZoneServer_Export WorldEvent : public WorldEventScoreCallback,
    public WorldEventPlayCallback
{
    typedef std::mutex LockType;
public:
    struct RankerInfo
    {
        ObjectId playerId_;
        uint32_t score_;
        GameTime lastKillTime_;

        RankerInfo(ObjectId playerId = invalidObjectId,
            uint32_t score = 0, GameTime lastKillTime = 0) :
            playerId_(playerId),
            score_(score),
            lastKillTime_(lastKillTime)
        {
        }

        bool operator<(const RankerInfo& rhs) const {
            if (score_ != rhs.score_) {
                return score_ > rhs.score_;
            }
            return lastKillTime_ > rhs.lastKillTime_;
        }
    };

    struct TopRankerInfo
    {
        ObjectId playerId_;
        uint32_t score_;
        uint32_t ranking_;
        uint32_t rewardValue_;

        TopRankerInfo(ObjectId playerId = invalidObjectId,
            uint32_t score = 0, uint32_t ranking = 0, uint32_t rewardValue = 0) :
            playerId_(playerId),
            score_(score),
            ranking_(ranking),
            rewardValue_(rewardValue)
        {
        }

        bool operator<(const TopRankerInfo& rhs) const {
            return score_ < rhs.score_;            
        }
    };
    typedef sne::core::HashMap<uint32_t, WorldEventPhase*> EventPhaseMap;
    typedef sne::core::HashMap<WorldEventState::StateType, WorldEventState*> StateMap;
    typedef sne::core::Set<RankerInfo> RankerInfoSet;
    typedef sne::core::MultiSet<TopRankerInfo> TopRankerInfoSet;

public:
    WorldEvent(WorldMap& worldMap, WorldMapHelper& worldMapHelper,
        const gdt::world_event_t& worldEventTemplate);
    ~WorldEvent();

    bool initialize();
    void setInitWaitState();
    void fillWorldEventInfo(WorldEventInfos& worldEventInfos) const;
    void fillWorldEventRankerInfo(WorldEventRankerInfos& infos, 
        uint32_t& myScore, ObjectId playerId) const;
    void update();

    void enter(go::Entity& player, RegionCode regionCode);
    void leave(ObjectId playerId, RegionCode regionCode);

    void login(go::Entity& player);
    void logout(ObjectId playerId);
    
    void cheatOpen();
    void cheatClose();
    
    void setParty(WorldEventMissionCode missionCode, PartyId partyId);
    
    bool isPlaying() const;
    bool isReady() const;
    bool isWaiting() const;
    bool isUniqueEvent() const;

private:
    PlayerScoreInfo* getPlayerScoreInfo(ObjectId playerId);
    const PlayerScoreInfo* getPlayerScoreInfo(ObjectId playerId) const;
    uint32_t getRewardValue(ObjectId playerId) const;

    void initWorldEventState();
    bool initWorldEventPhase(const datatable::RegionTable& regionTable,
        const datatable::RegionCoordinates& regionCoordinates);
    void updateRanker(RankerInfo rankerInfo, uint32_t newscore);
    void fillWorldEventRankerInfo_i(WorldEventRankerInfos& infos) const;
    void rewardPlayer(go::Entity& player);
    void makeTopRanker();

private:
    // = WorldEventScoreCallback overriding
    virtual void addScore(go::Entity& killer, CreatureLevel level);
    virtual void resetScore();
    
    virtual void reward(bool isCompleteEvent);

private:
    mutable LockType lock_;
    mutable LockType playerLock_;

    const gdt::world_event_t& worldEventTemplate_;
    WorldMap& worldMap_;
    WorldMapHelper& worldMapHelper_;
    EventPhaseMap eventPhaseMap_;
    WorldEventState* currentState_;
    StateMap stateMap_;
    PlayerScoreInfoMap playerScoreInfoMap_;
    RankerInfoSet rankerInfoSet_;
    TopRankerInfoSet topRankerInfoSet_;
};

}} // namespace gideon { namespace zoneserver {
