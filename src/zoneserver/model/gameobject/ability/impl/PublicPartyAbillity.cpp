#include "ZoneServerPCH.h"
#include "PublicPartyAbillity.h"
#include "../Partyable.h"
#include "../../Entity.h"
#include "../../../../controller/EntityController.h"
#include "../../../../controller/callback/PartyCallback.h"
#include "../../../../world/WorldMap.h"
#include "../../../../service/party/Party.h"
#include "../../../../ZoneService.h"
#include "../../../../s2s/ZoneCommunityServerProxy.h"

namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class LeavePublicPartyEvent
 */
class LeavePublicPartyEvent : public EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<LeavePublicPartyEvent>
{
public:
    LeavePublicPartyEvent(ObjectId playerId) :
        playerId_(playerId) {}

private:
    virtual void call(go::Entity& entity) {
        if (! entity.isPlayer()) {
            return;
        }
        
        gc::PartyCallback* callback =
            entity.getController().queryPartyCallback();
        if (callback != nullptr) {
            callback->publicPartyMemberLeft(playerId_);
        }
    }

private:
    ObjectId playerId_;
};

} // namespace {

PublicPartyAbillity::PublicPartyAbillity()
{
    party_.reset(new Party(*this, invalidPartyId));
}


PublicPartyAbillity::~PublicPartyAbillity()
{
    destory();
}


void PublicPartyAbillity::setOwnerInfo(const GameObjectInfo& info)
{
    gameObjectInfo_ = info;
}


ErrorCode PublicPartyAbillity::addMember(go::Entity& member)
{
    {
        ACE_GUARD_RETURN(PartyLockType, lock, lockPublicPartyable_, ecServerInternalError);

        if (maxPublicPartyMember <= memberInfos_.size()) {
            return ecPartyMaxOverUser;
        }
        party_->addMember(member.getObjectId());
    }

    gc::PartyCallback* callback = member.getController().queryPartyCallback();
    if (callback) {
        callback->publicPartyMemberInfos(memberInfos_);
    }

    WorldPosition position = member.getWorldPosition();
    MapCode subMapCode = invalidMapCode;
    WorldMap* worldMap = member.getCurrentWorldMap();
    if (worldMap) {
        subMapCode = worldMap->getMapCode();
    }

    {
        ACE_GUARD_RETURN(PartyLockType, lock, lockPublicPartyable_, ecServerInternalError);
        memberInfos_.push_back(PartyMemberInfo(member.getObjectId(), 0, member.getNickname(), position.mapCode_, 
            subMapCode, pmgGeneral));
        memberMap_.insert(MemberMap::value_type(member.getObjectId(), &member));
    }

    ZONE_SERVICE->getCommunityServerProxy().z2m_publicPartyMemberJoined(member.getObjectId());

    member.queryPartyable()->setParty(party_);
    member.queryPartyable()->setPublicPartyable(this);

    return ecOk;
}


void PublicPartyAbillity::removeMember(ObjectId playerId)
{
    ZONE_SERVICE->getCommunityServerProxy().z2m_publicPartyMemberLeft(playerId);


    go::Entity* member = party_->getMember(playerId);
    if (member) {
        member->queryPartyable()->setParty(PartyPtr());
        member->queryPartyable()->setPublicPartyable(nullptr);
    }

    {
        ACE_GUARD(PartyLockType, lock, lockPublicPartyable_);
        party_->removeMember(playerId);
        memberMap_.erase(playerId);
        PartyMemberInfos::iterator pos = getPartyMemberIterator(playerId);
        if (pos != memberInfos_.end()) {
            memberInfos_.erase(pos);
        }        
    }

    go::EntityEvent::Ref event(new LeavePublicPartyEvent(playerId));
    party_->notifyEvent(event, playerId, true);
}


void PublicPartyAbillity::reservePartyInstance()
{
    ZONE_SERVICE->getCommunityServerProxy().z2m_createRandumDungeonPublicParty(gameObjectInfo_);
}


void PublicPartyAbillity::setPartyId(PartyId partyId)
{
    partyId_ = partyId;
}


PartyMemberInfos::iterator PublicPartyAbillity::getPartyMemberIterator(ObjectId playerId)
{
    PartyMemberInfos::iterator pos = memberInfos_.begin();
    PartyMemberInfos::iterator end = memberInfos_.end();
    for (; pos != end; ++pos) {
        PartyMemberInfo& info = (*pos);
        if (info.playerId_ == playerId) {
            break;
        }
    }
    return pos;
}

void PublicPartyAbillity::destory()
{
    PartyMemberInfos infos;
    {
        ACE_GUARD(PartyLockType, lock, lockPublicPartyable_);
        infos = memberInfos_;
    }

    PartyMemberInfos::iterator pos = infos.begin();
    PartyMemberInfos::iterator end = infos.end();
    for (; pos != end; ++pos) {
        PartyMemberInfo& info = (*pos);
        go::Entity* member = party_->getMember(info.playerId_);
        if (member) {
            Partyable* partyable = member->queryPartyable();
            if (partyable) {
                partyable->setParty(PartyPtr());
                partyable->setPublicPartyable(nullptr);
            }
            gc::PartyCallback* callback = member->getController().queryPartyCallback();
            if (callback) {
                callback->publicPartyMemberLeft(info.playerId_);
            }            
        }
        party_->removeMember(info.playerId_);
    }    
}



go::Entity* PublicPartyAbillity::getPlayer(ObjectId playerId)
{
    MemberMap::iterator pos = memberMap_.find(playerId);
    if (pos != memberMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


const go::Entity* PublicPartyAbillity::getPlayer(ObjectId playerId) const
{
    MemberMap::const_iterator pos = memberMap_.find(playerId);
    if (pos != memberMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}



}}} // namespace gideon { namespace zoneserver { namespace go {