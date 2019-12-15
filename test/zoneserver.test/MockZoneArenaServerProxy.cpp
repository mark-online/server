#include "ZoneServerTestPCH.h"
#include "MockZoneArenaServerProxy.h"

// = zoneserver::ZoneCommunityServerProxy overriding

MockZoneArenaServerProxy::MockZoneArenaServerProxy() :
	zoneserver::ZoneArenaServerProxy(sne::server::ServerInfo())
{
}


DEFINE_SRPC_METHOD_1(MockZoneArenaServerProxy, z2a_rendezvous,
    ZoneId, zoneId)
{
    addCallCount("z2a_rendezvous");
    zoneId;

    z2a_onRendezvous();
}


DEFINE_SRPC_METHOD_0(MockZoneArenaServerProxy, z2a_ready)
{
    addCallCount("z2a_ready");
}

