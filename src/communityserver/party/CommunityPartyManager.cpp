#include "CommunityServerPCH.h"
#include "CommunityPartyManager.h"
#include "CommunityParty.h"
#include <intrin.h>

namespace gideon { namespace communityserver {

SNE_DEFINE_SINGLETON(CommunityPartyManager);

void CommunityPartyManager::initialize()
{
    generatorPartyId_ = invalidPartyId;
}


ErrorCode CommunityPartyManager::createParty(PartyId& partyId, PartyType partyType, CommunityUser* master)    
{
    partyId = ++generatorPartyId_;

    auto party = std::make_shared<CommunityParty>();
    const ErrorCode errorCode = party->initialize(master, partyId, partyType);
    if (isFailed(errorCode)) {
        assert(false);
        return errorCode;
    }

    {
        std::lock_guard<LockType> lock(lock_);

        partyMap_.emplace(partyId, party);
    }

    return ecOk;
}


void CommunityPartyManager::destroyParty(PartyId partyId)
{
    std::lock_guard<LockType> lock(lock_);

    partyMap_.erase(partyId);
}


const CommunityPartyManager::CommunityPartyPtr CommunityPartyManager::getParty(PartyId partyId) const
{
    std::lock_guard<LockType> lock(lock_);

    const CommunityPartyMap::const_iterator pos = partyMap_.find(partyId);
    if (pos != partyMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}


CommunityPartyManager::CommunityPartyPtr CommunityPartyManager::getParty(PartyId partyId)
{
    std::lock_guard<LockType> lock(lock_);

    CommunityPartyMap::iterator pos = partyMap_.find(partyId);
    if (pos != partyMap_.end()) {
        return (*pos).second;
    }
    return nullptr;
}

}} // namespace gideon { namespace communityserver {
