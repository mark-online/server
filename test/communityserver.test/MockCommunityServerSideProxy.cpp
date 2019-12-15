#include "CommunityServerTestPCH.h"
#include "MockCommunityServerSideProxy.h"

// = rpc::ZoneCommunityRpc overriding

// = rpc::ZonePartyRpc overriding
DEFINE_SRPC_METHOD_2(MockCommunityServerSideProxy,z2m_evPartyMemberJoined,
    PartyId, partyId, ObjectId, playerId)
{
    addCallCount("z2m_evPartyMemberJoined");
    partyId, playerId;
}


DEFINE_SRPC_METHOD_2(MockCommunityServerSideProxy, z2m_evPartyMemberLeft,
    PartyId, partyId, ObjectId, playerId)
{
    addCallCount("z2m_evPartyMemberLeft");
    partyId, playerId;
}


DEFINE_SRPC_METHOD_3(MockCommunityServerSideProxy, z2m_evPartyMemberRejoined,
    PartyId, partyId, AccountId, accountId, ObjectId, playerId)
{
    partyId, accountId, playerId;
}
