#include "ZoneServerPCH.h"
#include "PartyService.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"

namespace gideon { namespace zoneserver {

SNE_DEFINE_SINGLETON(PartyService);

PartyService::PartyService()
{
}


PartyService::~PartyService() 
{
     partyMap_.clear();
     playerPartyMap_.clear();
}

bool PartyService::initialize()
{
    PartyService::instance(this);
    return true;
}


PartyPtr PartyService::createParty(PartyId partyId, PartyType partyType)
{
    std::lock_guard<LockType> lock(lock_);

    if (isExist(partyId)) {
        return PartyPtr();
    }

    auto party = std::make_shared<Party>(*this, partyId, partyType);
    partyMap_.emplace(partyId, party);

    return party;
}


void PartyService::addPartyMember(PartyId partyId, ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    PartyPtr party = getPartyByPartyId_i(partyId);
    playerPartyMap_.emplace(playerId, party);
}


void PartyService::removePartyMember(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    playerPartyMap_.erase(playerId);
}


void PartyService::destoryParty(PartyId partyId)
{
    std::lock_guard<LockType> lock(lock_);

    partyMap_.erase(partyId);
}


PartyPtr PartyService::getPartyByPartyId(PartyId partyId)
{
    std::lock_guard<LockType> lock(lock_);

    return getPartyByPartyId_i(partyId);
}


PartyPtr PartyService::getPartyByPlayerId(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    return getPartyByPlayerId_i(playerId);
}


bool PartyService::isExist(PartyId partyId) const
{
    std::lock_guard<LockType> lock(lock_);

    PartyMap::const_iterator pos = partyMap_.find(partyId);
    if (pos != partyMap_.end()) {
        return true;
    }
    return false;
}


PartyPtr PartyService::getPartyByPartyId_i(PartyId partyId)
{
    PartyMap::const_iterator pos = partyMap_.find(partyId);
    if (pos != partyMap_.end()) {
        return (*pos).second;
    }
    return PartyPtr();
}


PartyPtr PartyService::getPartyByPlayerId_i(ObjectId playerId)
{
    PlayerPartyMap::const_iterator pos = playerPartyMap_.find(playerId);
    if (pos != playerPartyMap_.end()) {
        return (*pos).second;
    }
    return PartyPtr();
}


go::Entity* PartyService::getPlayer(ObjectId playerId)
{
    return WORLD->getPlayer(playerId);
}


const go::Entity* PartyService::getPlayer(ObjectId playerId) const
{
    return WORLD->getPlayer(playerId);
}


}} // namespace gideon { namespace zoneserver {
