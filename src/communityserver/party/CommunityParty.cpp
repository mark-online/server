#include "CommunityServerPCH.h"
#include "CommunityParty.h"
#include "SoloParty.h"
#include "GroupParty.h"
#include "../user/CommunityUser.h"
#include "../s2s/CommunityServerSideProxyManager.h"

namespace gideon { namespace communityserver {

namespace {

std::unique_ptr<Party> createParty(PartyId partyId, PartyType partyType)
{
    if (isGroupParty(partyType)) {
        return std::make_unique<GroupParty>(partyId, partyType);
    }
    return std::make_unique<SoloParty>(partyId);
}

} // namespace {

CommunityParty::CommunityParty()
{
}


CommunityParty::~CommunityParty()
{
}


ErrorCode CommunityParty::initialize(CommunityUser* user, PartyId partyId, PartyType partyType)
{
    std::lock_guard<LockType> lock(lock_);

    if (isRaidParty(partyType)) {
        assert(false);
        return ecPartyInvalidPartyType;
    }

    party_ = createParty(partyId, partyType);
    if (isSoloParty(partyType)) {
        const PartyMemberInfo memberInfo(user->getPlayerId(), user->getCharacterClass(),
            PartyPosition(startPartyGroupIndex, 0), user->getNickname(),
            user->getWorldMapCode(), user->getSubMapCode(), pmgMaster);
        party_->addMember(memberInfo, user);        
        party_->setMaster(user);

        // TODO: 필요 없을 경우 삭제 (왜 여기에서 호출하는가?)
        //COMMUNITYSERVERSIDEPROXY_MANAGER->partyMemberJoined(party_->getPartyId(), memberInfo.playerId_);
    }
       
    return ecOk;
}


ErrorCode CommunityParty::invite(CommunityUser* user, ObjectId masterId)
{
    std::lock_guard<LockType> lock(lock_);

    const PartyPosition position = party_->getJoinPartyPosition();
    if (! position.isValid()) {
        return ecPartyMaxOverUser;
    }

    if (! party_->canInviteGrade(masterId)) {
        return ecPartyInvalidInvite;
    }
    
    const PartyMemberGrade memberGrade = party_->getJoinMemberGrade();
    const PartyMemberInfo newMemberInfo(user->getPlayerId(), user->getCharacterClass(),
        position, user->getNickname(),
        user->getWorldMapCode(), user->getSubMapCode(), memberGrade);

    const ErrorCode errorCode = party_->addMember(newMemberInfo, user);
    if (isSucceeded(errorCode) && memberGrade == pmgMaster) {
        party_->setMaster(user);
    }
    return errorCode;
}


void CommunityParty::closePublicEvent()
{
    std::lock_guard<LockType> lock(lock_);

    party_->closePublicEvent();
}


ErrorCode CommunityParty::kick(ObjectId masterId, ObjectId kickId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        if (masterId == kickId) {
            return ecPartyNotKickSelf;
        }

        if (! party_->isMaster(masterId)) {
            return ecPartyNotEnoughMemberGrade;
        }
        
        for (const OnlineUserMap::value_type& value : party_->getOnlineUserMap()) {
            CommunityUser* memberUser = value.second;
            memberUser->partyMemberKicked(kickId);
        }

        party_->removeMember(kickId);
    }

    if (shouldDestroy()) {
        COMMUNITYSERVERSIDEPROXY_MANAGER->partyMemberLeft(
            party_->getMaster()->getPartyId(), masterId);
    }
    return ecOk;
}


ErrorCode CommunityParty::delegateMaster(ObjectId masterId, ObjectId newMasterId)
{
    std::lock_guard<LockType> lock(lock_);

    if (! party_->isMaster(masterId)) {
        return ecPartyNotEnoughMemberGrade;
    }

    PartyMemberInfo* preMasterInfo = party_->getMemberInfo(masterId);
    if (! preMasterInfo) {
        return ecPartyNotFindUser;
    }

    PartyMemberInfo* newMasterInfo = party_->getMemberInfo(newMasterId);
    if (! newMasterInfo) {
        return ecPartyNotFindUser;
    }

    CommunityUser* newMaster =  party_->getOnlineUser(newMasterId);
    if (newMaster == nullptr) {
        return ecPartyNotFindUser;
    }

    preMasterInfo->grade_ = pmgGeneral;
    newMasterInfo->grade_ = pmgMaster;
    party_->setMaster(newMaster); 
   
    for (const OnlineUserMap::value_type& value : party_->getOnlineUserMap()) {
        CommunityUser* memberUser = value.second;
        memberUser->partyMasterChanged(newMasterId);
    }

    return ecOk;
}


ErrorCode CommunityParty::moveRaidPartyGroupPosition(ObjectId masterId, ObjectId memberId, const PartyPosition& position)
{
    std::lock_guard<LockType> lock(lock_);

    if (! party_->isMaster(masterId)) {
        return ecPartyNotEnoughMemberGrade;
    }
    return party_->moveRaidPartyGroupPosition(memberId, position);
}


ErrorCode CommunityParty::switchRaidPartyGroupPosition(ObjectId masterId, ObjectId srcId, ObjectId descId)
{
    std::lock_guard<LockType> lock(lock_);

    if (! party_->isMaster(masterId)) {
        return ecPartyNotEnoughMemberGrade;
    }
    return party_->switchRaidPartyGroupPosition(srcId, descId);
}


void CommunityParty::leave(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    party_->removeMember(playerId);
    const bool shouldDestroyParty = party_->shouldDestroy();

    ObjectId changeMasterId = invalidObjectId;
    if ((! shouldDestroyParty) && party_->isMaster(playerId)) {
        CommunityUser* newMaster = party_->getFirstOnlineUser();
        if (newMaster != nullptr) {
            PartyMemberInfo* newMasterInfo = party_->getMemberInfo(newMaster->getPlayerId());
            if (newMasterInfo != nullptr) {
                newMasterInfo->grade_ = pmgMaster; 
                changeMasterId = newMasterInfo->playerId_;
                party_->setMaster(newMaster);
            }
        }
    }

    for (const OnlineUserMap::value_type& value : party_->getOnlineUserMap()) {
        CommunityUser* memberUser = value.second;
        memberUser->partyMemberLeft(playerId, shouldDestroyParty);
        if (shouldDestroyParty) {
            COMMUNITYSERVERSIDEPROXY_MANAGER->partyMemberLeft(
                memberUser->getPartyId(), memberUser->getPlayerId());
        }
        if (isValidObjectId(changeMasterId)) {
            memberUser->partyMasterChanged(changeMasterId);
        }                
    }
}


void CommunityParty::say(const PlayerInfo& playerInfo, const ChatMessage& message)
{
    std::lock_guard<LockType> lock(lock_);

    for (const OnlineUserMap::value_type& value : party_->getOnlineUserMap()) {
        CommunityUser* memberUser = value.second;
        memberUser->partyMemberSaid(playerInfo, message);
    }
}


void CommunityParty::setWaypoint(ObjectId memberId, const Waypoint& waypoint)
{
    std::lock_guard<LockType> lock(lock_);

    PartyMemberInfo* memberInfo = party_->getMemberInfo(memberId);
    if (! memberInfo) {
        assert(false);
        return;
    }
    memberInfo->setWaypoint(waypoint);

    for (const OnlineUserMap::value_type& value : party_->getOnlineUserMap()) {
        CommunityUser* memberUser = value.second;
        memberUser->evWaypointSet(memberId, waypoint);
    }
}


void CommunityParty::moveMap(ObjectId memberId, MapCode worldMapCode, MapCode subMapCode)
{
    std::lock_guard<LockType> lock(lock_);

    PartyMemberInfo* memberInfo = party_->getMemberInfo(memberId);
    if (memberInfo) {
        memberInfo->worldMapCode_ = worldMapCode; 
        memberInfo->subMapCode_ = subMapCode;
    }

    for (const OnlineUserMap::value_type& value : party_->getOnlineUserMap()) {
        CommunityUser* memberUser = value.second;
        if (memberUser->getPlayerId() != memberId) {
            memberUser->partyMemberMapMoved(memberId, worldMapCode, subMapCode);
        }
    }
}


void CommunityParty::memberRejoined(CommunityUser& member)
{
    PartyType partyType;
    PartyMemberInfos memberInfos;
    {
        std::lock_guard<LockType> lock(lock_);

        memberInfos = party_->getPartyMembers();
        partyType = party_->getPartyType();
    }

    member.partyJoined(memberInfos, partyType);
}


void CommunityParty::characterClassChanged(ObjectId memberId, CharacterClass characterClass)
{
    std::lock_guard<LockType> lock(lock_);

    PartyMemberInfo* memberInfo = party_->getMemberInfo(memberId);
    if (memberInfo) {
        memberInfo->characterClass_ = characterClass;
    }

    COMMUNITYSERVERSIDEPROXY_MANAGER->partyMemberCharacterClassChanged(party_->getPartyId(), memberId, characterClass);
}


ErrorCode CommunityParty::changePartyType(ObjectId memberId, PartyType partyType)
{
    PartyMemberInfo* memberInfo = party_->getMemberInfo(memberId);
    if (! memberInfo) {
        return ecPartyIsNotMember;
    }

    if (! party_->isMaster(memberId)) {
        return ecPartyNotEnoughMemberGrade;
    }

    const ErrorCode errorCode = party_->canChangeParty(partyType);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    for (const OnlineUserMap::value_type& value : party_->getOnlineUserMap()) {
        CommunityUser* memberUser = value.second;
        if (memberUser->getPlayerId() != memberId) {
            memberUser->partyTypeChanged(partyType);
        }
    }

    std::unique_ptr<Party> newParty = createParty(party_->getPartyId(), partyType);
    newParty->initialize(party_->getOnlineUserMap(), party_->getPartyMemberInfoMap());
    newParty->setMaster(party_->getMaster());
    party_ = std::move(newParty);

    COMMUNITYSERVERSIDEPROXY_MANAGER->partyTypeChanged(party_->getPartyId(), partyType);

    return ecOk;
}


bool CommunityParty::shouldDestroy() const
{
    std::lock_guard<LockType> lock(lock_);

    return party_->shouldDestroy();
}


ErrorCode CommunityParty::canInvite(ObjectId memberId) const
{
    std::lock_guard<LockType> lock(lock_);

    if (! party_->canAddMember()) {
        return ecPartyMaxOverUser;
    }

    const PartyMemberInfo* memberInfo = party_->getMemberInfo(memberId);
    if (! memberInfo) {
        return ecPartyIsNotMember;
    }

    return memberInfo->grade_ == pmgMaster ? ecOk : ecPartyNotEnoughMemberGrade;
}


bool CommunityParty::canMoveOtherParty() const
{
    return party_->canMoveOtherParty();
}


bool CommunityParty::isEmptyMember() const
{
    std::lock_guard<LockType> lock(lock_);

    return party_->isEmptyMember();
}

}} // namespace gideon { namespace communityserver {
