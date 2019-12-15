#include "ZoneServerTestPCH.h"
#include "MockZoneCommunityServerProxy.h"

// = zoneserver::ZoneCommunityServerProxy overriding

MockZoneCommunityServerProxy::MockZoneCommunityServerProxy() :
	zoneserver::ZoneCommunityServerProxy(sne::server::ServerInfo())
{
}


DEFINE_SRPC_METHOD_1(MockZoneCommunityServerProxy, z2m_rendezvous,
    ZoneId, zoneId)
{
    addCallCount("z2m_rendezvous");
    zoneId;

    z2m_onRendezvous();
}


DEFINE_SRPC_METHOD_0(MockZoneCommunityServerProxy, z2m_ready)
{
    addCallCount("z2m_ready");
}


DEFINE_SRPC_METHOD_2(MockZoneCommunityServerProxy, z2m_worldMapOpened,
    MapCode, worldMapCode, ObjectId, mapId)
{
    addCallCount("z2m_worldMapOpened");

    worldMapCode, mapId;
}


DEFINE_SRPC_METHOD_2(MockZoneCommunityServerProxy, z2m_worldMapClosed,
    MapCode, worldMapCode, ObjectId, mapId)
{
    addCallCount("z2m_worldMapClosed");

    worldMapCode, mapId;
}


DEFINE_SRPC_METHOD_1(MockZoneCommunityServerProxy, z2m_initPlayerInfo,
    PlayerInfo, playerInfo)
{
    addCallCount("z2m_initPlayerInfo");

    playerInfo;
}


DEFINE_SRPC_METHOD_4(MockZoneCommunityServerProxy, z2m_worldMapEntered,
    ObjectId, mapId, AccountId, accountId, ObjectId, playerId,
    Position, position)
{
    addCallCount("z2m_worldMapEntered");

    mapId, accountId, playerId, position;
}


DEFINE_SRPC_METHOD_2(MockZoneCommunityServerProxy, z2m_worldMapLeft,
    AccountId, accountId, ObjectId, playerId)
{
    addCallCount("z2m_worldMapLeft");

    accountId, playerId;
}
