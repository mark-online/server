#pragma once

#include "ZoneServer/s2s/ZoneArenaServerProxy.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
* @class MockZoneArenaServerProxy
*/
class MockZoneArenaServerProxy :
	public zoneserver::ZoneArenaServerProxy,
	public sne::test::CallCounter
{
public:
	MockZoneArenaServerProxy();

private:
	virtual bool isActivated() const {
		return true;
	}

private:
	// = zoneserver::ZoneCommunityServerProxy overriding
	OVERRIDE_SRPC_METHOD_1(z2a_rendezvous,
		ZoneId, zoneId);
	OVERRIDE_SRPC_METHOD_0(z2a_ready);
};
