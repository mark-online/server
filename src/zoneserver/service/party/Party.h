#pragma once

#include "../../zoneserver_export.h"
#include "../../model/gameobject/EntityEvent.h"
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/LevelInfo.h>
#include <gideon/cs/shared/data/StatusInfo.h>
#include <gideon/cs/shared/data/ChaoInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/QuestInfo.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go

class PartyPlayerHelper;

/***
 * @class Party
 ***/
class ZoneServer_Export Party : public sne::core::ThreadSafeMemoryPoolMixin<Party>
{
    typedef std::mutex LockType;
    typedef sne::core::Set<ObjectId> Members;
    typedef sne::core::Map<ObjectId, uint8_t> MoveCountMap;
    typedef sne::core::Vector<go::Entity*> Entities;

public:
    Party(PartyPlayerHelper& partyPlayerHelper, PartyId partyId, PartyType partType);
    ~Party();
    
    void addMember(ObjectId playerId);
    void removeMember(ObjectId playerId);
    void characterClassChanged(ObjectId playerId, CharacterClass cc);
    void notifyEvent(go::EntityEvent::Ref event, ObjectId playerId, bool selfExcept = false);
    void notifyNearEvent(go::EntityEvent::Ref event, ObjectId playerId, bool selfExcept = false);
    void notifyMove(ObjectId playerId, const Position& position, bool isStop);
    void rewardExp(ExpPoint expPoint, MapCode mapCode, const Position& position);
    void killed(go::Entity& victim);
    void notifyPushQuest(go::Entity& questPusher, QuestCode questCode);
    void setPartyType(PartyType partyType);
    
    bool shouldDestroy() const;
    PartyId getPartyId() const {
        return partyId_;
    }

    go::Entity* getMember(ObjectId memberId);

private:
    bool isExist(ObjectId memberId) const;

    void fillExpRewardInfos(Entities& members, uint32_t& sumLevel, uint32_t& reawardMemberCount,
        MapCode mapCode, const Position& position) const;
private:
    mutable LockType lock_;
    PartyId partyId_;
    PartyType partyType_;

    Members members_;
    MoveCountMap moveCountMap_;
    PartyPlayerHelper& partyPlayerHelper_;
};

typedef std::shared_ptr<Party> PartyPtr;

}} // namespace gideon { namespace zoneserver {
