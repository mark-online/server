#pragma once

#include <gideon/cs/shared/data/PlayerInfo.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace communityserver {

class CommunityUser;
class Party;
/***
 * @class CommunityParty
 ***/
class CommunityParty
{
    typedef sne::core::Map<ObjectId, CommunityUser*> OnlineUserMap;
    typedef std::mutex LockType;
public:
    CommunityParty();
    ~CommunityParty();

    ErrorCode initialize(CommunityUser* user, PartyId partyId, PartyType partyType);
    ErrorCode invite(CommunityUser* user, ObjectId masterId);
    
    /// 퍼플릭 파티의 경우 이벤트가 닫히고 사람이 업어야 닫힌다.
    void closePublicEvent();

public:
    ErrorCode kick(ObjectId masterId, ObjectId kickId);
    ErrorCode delegateMaster(ObjectId masterId, ObjectId newMasterId);
    ErrorCode moveRaidPartyGroupPosition(ObjectId masterId, ObjectId memberId, const PartyPosition& position);
    ErrorCode switchRaidPartyGroupPosition(ObjectId masterId, ObjectId srcId, ObjectId descId);
    void moveMap(ObjectId memberId, MapCode worldMapCode, MapCode subMapCode);
    void leave(ObjectId memberId);
    void say(const PlayerInfo& playerInfo, const ChatMessage& message);
    void setWaypoint(ObjectId memberId, const Waypoint& waypoint);
    void memberRejoined(CommunityUser& member);
    void characterClassChanged(ObjectId memberId, CharacterClass characterClass);
    
    ErrorCode changePartyType(ObjectId memberId, PartyType partyType);

public:
    virtual bool shouldDestroy() const;
    ErrorCode canInvite(ObjectId memberId) const;
    
    bool canMoveOtherParty() const;
    bool isEmptyMember() const;

protected:
    mutable LockType lock_;
    std::unique_ptr<Party> party_;
};

}} // namespace gideon { namespace communityserver {