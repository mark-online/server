#pragma once

#include "ZoneServer/s2s/ZoneCommunityServerProxy.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
* @class MockZoneCommunityServerProxy
*/
class MockZoneCommunityServerProxy :
    public zoneserver::ZoneCommunityServerProxy,
    public sne::test::CallCounter
{
public:
    MockZoneCommunityServerProxy();

private:
    virtual bool isActivated() const {
        return true;
    }

private:
    // = zoneserver::ZoneCommunityServerProxy overriding
    OVERRIDE_SRPC_METHOD_1(z2m_rendezvous,
        ZoneId, zoneId);
    OVERRIDE_SRPC_METHOD_0(z2m_ready);

    OVERRIDE_SRPC_METHOD_2(z2m_worldMapOpened,
        MapCode, worldMapCode, ObjectId, mapId);
    OVERRIDE_SRPC_METHOD_2(z2m_worldMapClosed,
        MapCode, worldMapCode, ObjectId, mapId);

    OVERRIDE_SRPC_METHOD_1(z2m_initPlayerInfo,
        PlayerInfo, playerInfo);
    OVERRIDE_SRPC_METHOD_4(z2m_worldMapEntered,
        ObjectId, mapId, AccountId, accountId, ObjectId, playerId, Position, position);
    OVERRIDE_SRPC_METHOD_2(z2m_worldMapLeft,
        AccountId, accountId, ObjectId, playerId);
};
