#pragma once

#include "StateBrain.h"
#include "../service/time/GameTimer.h"
#include <gideon/cs/shared/data/NpcInfo.h>

namespace gideon { namespace datatable {
class NpcTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace ai {

class BrainEvent;
class GlobalBrainState;
class NpcBrainStateFactory;

namespace {
class HandleEventTask;
class EnumerateCommandForWatchMovement;
class EnumerateCommandForWatchPlayerWorldEventRegionJoinable;
class EnumerateCommandForTalkTo;
} // namespace {

/**
 * @class NpcStateBrain
 */
class NpcStateBrain : public StateBrain
{
    typedef sne::core::HashMap<ObjectId, GameTime> TalkToPlayerMap;

public:
    NpcStateBrain(go::Entity& owner, std::unique_ptr<evt::EventTrigger> eventTrigger);
    virtual ~NpcStateBrain();

public:
    void callLink();
    void callAssist(go::Entity& assist);

    /// 관심 영역 내에서 Entity의 움직임을 감지한다
    bool watchMovement(GameTime diff);

    /// 관심 영역 내에서 플레이어가 월드 이벤트에 미션에 참여할 수 있는지 감지한다
    bool watchPlayerWorldEventRegionJoinable(GameTime diff);

    bool talkTo(GameTime diff);

private:
    // = StateBrain overriding
    virtual void initialize();
    virtual void finalize();

    //virtual void spawned();
    virtual void attacked(const go::Entity& attacker);
    virtual void died();
    virtual void pathNodeArrived();

    virtual AggroList* getAggroList() {
        return aggroList_.get();
    }

    virtual bool canStrikeBack() const;
    virtual bool shouldFlee() const;

private:
    // = Stateable overriding
    virtual void asyncHandleEvent(int eventId);
    virtual void handleEvent(int eventId);

    virtual bool isInState(int stateId) const;
    virtual BrainState& getEntryState();
    virtual BrainState* getState(int stateId);

private:
    friend class EnumerateCommandForWatchMovement;
    bool perceiveEntity(go::Entity& entity);

    friend class EnumerateCommandForWatchPlayerWorldEventRegionJoinable;
    bool perceivePlayerWorldEventRegionJoinable(go::Entity& entity);

    friend class EnumerateCommandForTalkTo;
    bool talkTo(go::Entity& entity);

private:
    bool canWatchMovement(GameTime diff) const;
    bool canWatchPlayerWorldEventRegionJoinable(GameTime diff) const;
    bool canTalkTo(GameTime diff) const;

    /// 상호 작용할 수 있는가? (현재는 단순히 거리 검사)
    bool canInteract(const ObjectPosition& position) const;

    /// 상대에게 반응해야 하는가?
    bool shouldReact(go::Entity& entity) const;

    bool shouldGuardBuildingAgainst(go::Entity& entity) const;

private:
    friend class HandleEventTask;
    void handleEvent(BrainEvent& event);

private:
    std::unique_ptr<NpcBrainStateFactory> stateFactory_;
    std::unique_ptr<GlobalBrainState> globalState_;
    std::unique_ptr<AggroList> aggroList_;

    const datatable::NpcTemplate& npcTemplate_;
    AiReactiveType reactiveType_;
    float32_t reactiveDistanceSq_;

    mutable GameTimeTracker nextWatchMovementTime_;
    mutable GameTimeTracker nextWatchPlayerWorldEventRegionJoinableTime_;

    TalkToPlayerMap talkToPlayerMap_;
    mutable GameTimeTracker nextTalkToTime_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
