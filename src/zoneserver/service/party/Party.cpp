#include "ZoneServerPCH.h"
#include "Party.h"
#include "PartyPlayerHelper.h"
#include "../../model/gameobject/Creature.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/status/CreatureStatus.h"
#include "../../model/gameobject/ability/Questable.h"
#include "../../model/gameobject/ability/Liveable.h"
#include "../../model/quest/QuestRepository.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../controller/callback/PartyCallback.h"
#include "../../controller/callback/QuestCallback.h"
#include "../../world/WorldMap.h"
#include <gideon/cs/datatable/PropertyTable.h>
#include <sne/base/concurrent/TaskScheduler.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class MemberRewardEvent
 */
class MemberRewardEvent: public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<MemberRewardEvent>
{
public:
    MemberRewardEvent(ExpPoint exp) :
        exp_(exp) {}

private:
    virtual void call(go::Entity& entity) {
        gc::RewardCallback* rewardCallbck = entity.getController().queryRewardCallback();
        if (rewardCallbck) {
            rewardCallbck->expRewarded(exp_);
        }
    }

private:
    const ExpPoint exp_;
};

/**
 * @class MemberCharacterClassChangedEvent
 */
class MemberCharacterClassChangedEvent: public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<MemberCharacterClassChangedEvent>
{
public:
    MemberCharacterClassChangedEvent(ObjectId characterId, CharacterClass cc) :
        characterId_(characterId),
        cc_(cc) {}

private:
    virtual void call(go::Entity& entity) {
        gc::RewardCallback* rewardCallbck = entity.getController().queryRewardCallback();
        if (rewardCallbck) {
            rewardCallbck->characterClassChanged(characterId_, cc_);
        }
    }

private:
    ObjectId characterId_;
    CharacterClass cc_;
};

inline float32_t getRewardExpRate(uint32_t memberCount, bool isRaid)
{
    if (isRaid) {
        return 1.0f;
    }
    else
    {
        switch (memberCount)
        {
        case 1:
        case 2:
            return 1.0f;
        case 3:
            return 1.166f;
        case 4:
            return 1.3f;            
        case 5:
            return 1.4f;
        }
    }
    return 1.0f;
}

} // namespace

Party::Party(PartyPlayerHelper& partyPlayerHelper, PartyId partyId, PartyType partyType) :
    partyPlayerHelper_(partyPlayerHelper),    
    partyId_(partyId),
    partyType_(partyType)
{

}


Party::~Party()
{
}


void Party::addMember(ObjectId playerId)
{   
    std::lock_guard<LockType> lock(lock_);

    members_.insert(playerId);
    moveCountMap_.emplace(playerId, uint8_t(0));    
}


void Party::removeMember(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    members_.erase(playerId);
    moveCountMap_.erase(playerId);    
}


void Party::characterClassChanged(ObjectId playerId, CharacterClass cc)
{
    auto event = std::make_shared<MemberCharacterClassChangedEvent>(playerId, cc);
    notifyEvent(event, playerId, true);
}

void Party::notifyEvent(go::EntityEvent::Ref event, ObjectId playerId, bool selfExcept)
{
    Members members;
    {
        std::lock_guard<LockType> lock(lock_);

        members = members_;
    }

    Members::const_iterator pos = members.begin();
    Members::const_iterator end = members.end();
    for (; pos != end; ++pos) {
        ObjectId memberId = (*pos);
        if (selfExcept && memberId == playerId) {
            continue;
        }
       
        go::Entity* member = partyPlayerHelper_.getPlayer(memberId);
        if (member) {
            if (member->isValid()) {
                if (! member->queryLiveable()->getCreatureStatus().isDied()) {
                    (void)TASK_SCHEDULER->schedule(
                        std::make_unique<go::EventCallTask>(*member, event));
                }
            }
        }
    }
}


void Party::notifyMove(ObjectId playerId, const Position& position, bool isStop)
{
    const uint8_t notifyCount = 20;

    Members members;
    {
        std::lock_guard<LockType> lock(lock_);

        const MoveCountMap::iterator pos = moveCountMap_.find(playerId);
        if (pos == moveCountMap_.end()) {
            return;
        }
        uint8_t& moveCount = (*pos).second;
        if (isStop) {
            moveCount = 0;
        }
        else if (moveCount < notifyCount) {
            ++moveCount;
            return;
        }      

        members = members_; 
    }


    Members::const_iterator pos = members.begin();
    Members::const_iterator end = members.end();
    for (; pos != end; ++pos) {
        ObjectId memberId = (*pos);
        if (memberId == playerId) {
            continue;
        }
        go::Entity* member = partyPlayerHelper_.getPlayer(memberId);
        if (member) {
            if (member->isValid()) {        
                gc::PartyCallback* callback = member->getController().queryPartyCallback();
                if (callback) {
                    callback->partyMemberMoved(playerId, position);
                }
            }
        }
    }
}


void Party::notifyNearEvent(go::EntityEvent::Ref event, ObjectId playerId, bool selfExcept)
{
    go::Entity* player = partyPlayerHelper_.getPlayer(playerId);
    if (! player) {
        assert(false);
        return;
    }
    go::Knowable* knowable = player->queryKnowable();
    if (! knowable) {
        assert(false);
    }

    Members members;
    {
        std::lock_guard<LockType> lock(lock_);

        members = members_;
    }

    Members::const_iterator pos = members.begin();
    Members::const_iterator end = members.end();
    for (; pos != end; ++pos) {
        ObjectId memberId = (*pos);
        if (selfExcept && memberId == playerId) {
            continue;
        }
        
        go::Entity* member = partyPlayerHelper_.getPlayer(memberId);
        if (member) {
            if (! knowable->doesKnow(member->getGameObjectInfo())) {
                continue;
            }
            if (member->isValid()) {
                if (! member->queryLiveable()->getCreatureStatus().isDied()) {
                    (void)TASK_SCHEDULER->schedule(
                        std::make_unique<go::EventCallTask>(*member, event));
                }
            }
        }
    }
}


void Party::rewardExp(ExpPoint expPoint, MapCode mapCode, const Position& position)
{
    Entities members;
    uint32_t sumLevel = 0;
    uint32_t reawardMemberCount = 0;     
    {
        std::lock_guard<LockType> lock(lock_);
        fillExpRewardInfos(members, sumLevel, reawardMemberCount, mapCode, position);
    }
    if (reawardMemberCount == 0 || sumLevel == 0) {
        return;
    }

    const ExpPoint baseRewardPoint = 
        toExpPoint(static_cast<uint32_t>(expPoint * getRewardExpRate(reawardMemberCount, false))); 

    for (go::Entity* member : members) {
        if (member->isValid()) {
            const CreatureLevel memberLevel = static_cast<go::Creature*>(member)->getCreatureLevel();
            ExpPoint rewardExpPoint = toExpPoint(baseRewardPoint * memberLevel / sumLevel);
            rewardExpPoint = rewardExpPoint == 0 ? toExpPoint(1) : rewardExpPoint;
            if (! member->queryLiveable()->getCreatureStatus().isDied()) {
                auto event = std::make_shared<MemberRewardEvent>(rewardExpPoint);
                (void)TASK_SCHEDULER->schedule(
                    std::make_unique<go::EventCallTask>(*member, event));
            }
        }
    }
}


go::Entity* Party::getMember(ObjectId memberId)
{
    if (! isExist(memberId)) {
        return nullptr;
    }

    return  partyPlayerHelper_.getPlayer(memberId);
}


void Party::killed(go::Entity& victim)
{
    Entities members;
    uint32_t sumLevel = 0;
    uint32_t reawardMemberCount = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        WorldMap* currentMap = victim.getCurrentWorldMap();
        if (currentMap) {
            fillExpRewardInfos(members, sumLevel, reawardMemberCount, currentMap->getMapCode(), victim.getPosition());  
        }
    }

    if (members.empty()) {
        return;
    }

    for (go::Entity* member : members) {
        if (member->isValid()) {
            go::Questable* questable = member->queryQuestable();
            if (questable != nullptr) {
                questable->killed(victim);            
            }
        }
    }
}


void Party::notifyPushQuest(go::Entity& questPusher, QuestCode questCode)
{
    Members members;
    {
        std::lock_guard<LockType> lock(lock_);

        members = members_;
    }
    const GameObjectInfo& pusherInfo = questPusher.getGameObjectInfo();
    for (ObjectId memberId : members) {
        go::Entity* member = partyPlayerHelper_.getPlayer(memberId);
        if (! member) {
            continue;
        }

        if (member->isSame(pusherInfo)) {
            continue;
        }

        QuestToPartyResultType resultType = qtprtOk;
        if (member->getCurrentWorldMap() != questPusher.getCurrentWorldMap()) {
            resultType = qtprtIsNotSameWorld;
        }
        else {
            resultType = member->queryQuestable()->getQuestRepository().getAcceptConditionResult(questCode);
        }

        if (isSucceeded(resultType)) {
            member->getController().queryQuestCallback()->questToPartyPushed(pusherInfo, questCode);     
        }
        
        questPusher.getController().queryQuestCallback()->questToPartyResult(member->getGameObjectInfo(), questCode, resultType);        
    }
}


void Party::setPartyType(PartyType partyType)
{
    std::lock_guard<LockType> lock(lock_);

    partyType_ = partyType;
}


bool Party::shouldDestroy() const
{
    std::lock_guard<LockType> lock(lock_);

    return members_.empty();
}


bool Party::isExist(ObjectId memberId) const
{
    std::lock_guard<LockType> lock(lock_);

    return members_.find(memberId) != members_.end();
}



void Party::fillExpRewardInfos(Entities& members, uint32_t& sumLevel, uint32_t& reawardMemberCount,
    MapCode mapCode, const Position& position) const
{
    const float32_t rewardDistance = GIDEON_PROPERTY_TABLE->getPropertyValue<float32_t>(L"party_reward_exp_distance");
    const float32_t rewardDistanceSq = rewardDistance * rewardDistance;
    for (ObjectId memberId : members_) {
        go::Entity* member = partyPlayerHelper_.getPlayer(memberId);
        if (! member) {
            continue;
        }
        const float32_t distanceToTargetSq = member->getSquaredLength(position);
        if (distanceToTargetSq > rewardDistanceSq) {
            continue;;
        }
        if (mapCode != member->getCurrentWorldMap()->getMapCode()) {
            continue;
        }
        if (member->queryLiveable()->getCreatureStatus().isDied()) {
            continue;
        }
        sumLevel += static_cast<go::Creature*>(member)->getCreatureLevel();
        ++reawardMemberCount;
        members.push_back(member);
    }
}

}} // namespace gideon { namespace zoneserver {
