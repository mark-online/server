#pragma once

#include "CommunityServer/s2s/CommunityServerSideProxy.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace gideon;

/**
* @class MockCommunityServerSideProxy
*/
class MockCommunityServerSideProxy :
    public communityserver::CommunityServerSideProxy,
    public sne::test::CallCounter
{
public:
    MockCommunityServerSideProxy(sne::server::ServerId serverId) :
        communityserver::CommunityServerSideProxy(0),
        lastErrorCode_(ecWhatDidYouTest) {
        setServerId(serverId);
    }

    void unbridge() {
        unbridged();
    }


public:
    // = rpc::ZoneCommunityRpc overriding

    // = rpc::ZonePartyRpc overriding
    OVERRIDE_SRPC_METHOD_2(z2m_evPartyMemberJoined,
        PartyId, partyId, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_2(z2m_evPartyMemberLeft,
        PartyId, partyId, ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_3(z2m_evPartyMemberRejoined,
        PartyId, partyId, AccountId, accountId, ObjectId, playerId);

public:
    ErrorCode lastErrorCode_;
};
