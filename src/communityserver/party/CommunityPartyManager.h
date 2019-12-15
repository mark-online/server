#pragma once

#include "../communityserver_export.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <sne/core/utility/Singleton.h>
#include <atomic>

namespace gideon { namespace communityserver {

class CommunityParty;
class CommunityUser;

/***
 * @class CommunityPartyManager
 ***/
class CommunityServer_Export CommunityPartyManager
{
public:
    typedef std::shared_ptr<CommunityParty> CommunityPartyPtr;
    typedef sne::core::HashMap<Nickname, PartyId> NicknameMap;
    typedef sne::core::HashMap<PartyId, CommunityPartyPtr> CommunityPartyMap;
    typedef std::mutex LockType;

    SNE_DECLARE_SINGLETON(CommunityPartyManager);
public:
    void initialize();
public:
    ErrorCode createParty(PartyId& partyId, PartyType partyType, CommunityUser* master);

    void destroyParty(PartyId partyId);

    const CommunityPartyPtr getParty(PartyId partyId) const;
    CommunityPartyPtr getParty(PartyId partyId);

private:
    mutable LockType lock_;
    
    std::atomic<PartyId> generatorPartyId_;
    NicknameMap nicknameMap_;
    CommunityPartyMap partyMap_;
};

}} // namespace gideon { namespace communityserver {

#define COMMUNITYPARTY_MANAGER gideon::communityserver::CommunityPartyManager::instance()
