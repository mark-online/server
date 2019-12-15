#pragma once

#include <sne/server/s2s/ServerId.h>
#include <sne/server/common/SocketAddress.h>
#include <sne/core/container/Containers.h>

namespace gideon {

typedef sne::core::HashMap<sne::server::ServerId, sne::server::SocketAddress>
    ServerAddressMap;


inline std::string getLoginServerName()
{
    return "login_server";
}


inline std::string getZoneServerName()
{
    return "zone_server";
}


inline std::string getCommunityServerName()
{
    return "community_server";
}


inline std::string getArenaServerName()
{
	return "arena_server";
}


inline std::string getDatabaseProxyServerName()
{
    return "database_proxy_server";
}


inline std::string getS2sIoServiceName()
{
    return "s2s";
}


inline std::string getS2dIoServiceName()
{
    return "s2d";
}



} // namespace gideon {
