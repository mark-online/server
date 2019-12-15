#include "ZoneServerPCH.h"
#include "ArenaServerSideProxy.h"
#include "../../ZoneService.h"
#include "../../service/arena/ArenaService.h"
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(ArenaServerSideProxy);

ArenaServerSideProxy::ArenaServerSideProxy(
    sne::server::ServerSideBridge* bridge) :
    sne::server::ManagedServerSideProxy(bridge),
    zoneId_(invalidZoneId)
{
    if (hasSession()) {
        assert(getSession().hasImpl());
        sne::sgp::RpcingExtension* extension =
            getSession().getImpl().getExtension<sne::sgp::RpcingExtension>();
        extension->registerRpcForwarder(*this);
        extension->registerRpcReceiver(*this);
    }
}


ArenaServerSideProxy::~ArenaServerSideProxy()
{
}

// = sne::server::ManagedServerSideProxy overriding

void ArenaServerSideProxy::bridged()
{
    sne::server::ManagedServerSideProxy::bridged();

    SNE_LOG_WARNING("CommunityServerSideProxy::bridged(S%u).",
        getServerId());
}


void ArenaServerSideProxy::unbridged()
{
    SNE_LOG_WARNING("CommunityServerSideProxy::unbridged(S%u).",
        getServerId());

    if (! sne::server::isValid(getServerId())) {
        return;
    }

    //if (isZoneServerProxy()) {
    //    WORLDMAP_CHANNEL_MANAGER->zoneServerDisconnected(zoneId_);

    //    SNE_LOG_INFO("ZoneServer(S%u) connected", getServerId());
    //}
    //else {
    //    assert(false);
    //}

    sne::server::ManagedServerSideProxy::unbridged();

    zoneId_ = invalidZoneId;
}

// = sne::server::ServerSideProxy overriding

bool ArenaServerSideProxy::verifyAuthentication(
    const sne::server::S2sCertificate& certificate) const
{
    return isSucceeded(ZONE_SERVICE->authenticate(certificate));
}


RECEIVE_SRPC_METHOD_1(ArenaServerSideProxy, z2a_rendezvous,
	ZoneId, zoneId)
{
	zoneId_ = zoneId;

	z2a_onRendezvous();

	SNE_LOG_INFO("ZoneServer(S%u) connected", getServerId());
}


FORWARD_SRPC_METHOD_0(ArenaServerSideProxy, z2a_onRendezvous);


RECEIVE_SRPC_METHOD_0(ArenaServerSideProxy, z2a_ready)
{
}


RECEIVE_SRPC_METHOD_1(ArenaServerSideProxy, z2a_releaseDeserter,
    ObjectId, playerId)
{
    ARENA_SERVICE->releaseDeserter(playerId);
}


RECEIVE_SRPC_METHOD_2(ArenaServerSideProxy, z2a_challengeSolo,
	ObjectId, playerId, MapCode, modeMapCode)
{
	const ErrorCode errorCode = ARENA_SERVICE->challenge(getServerId(), playerId, modeMapCode);
	z2a_onChallengeSolo(errorCode, playerId, modeMapCode);
}


FORWARD_SRPC_METHOD_3(ArenaServerSideProxy, z2a_onChallengeSolo,
	ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode);


RECEIVE_SRPC_METHOD_2(ArenaServerSideProxy, z2a_cancelChallenge,
	ObjectId, playerId, MapCode, modeMapCode)
{
	const ErrorCode errorCode = ARENA_SERVICE->cancelChallenge(playerId, modeMapCode);
	z2a_onCancelChallenge(errorCode, playerId, modeMapCode);
}


FORWARD_SRPC_METHOD_3(ArenaServerSideProxy, z2a_onCancelChallenge,
	ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode);


RECEIVE_SRPC_METHOD_1(ArenaServerSideProxy, z2a_logoutChallenger,
    ObjectId, playerId)
{
    ARENA_SERVICE->removeChallenger(playerId);
}


FORWARD_SRPC_METHOD_4(ArenaServerSideProxy, z2a_evArenaMatched,
	ObjectId, arenaId, ObjectId, playerId, MapCode, modeMapCode, MapCode, arenaWorldMapCode);

// = sne::srpc::RpcForwarder overriding

void ArenaServerSideProxy::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityServerSideProxy::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void ArenaServerSideProxy::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("CommunityServerSideProxy::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}} // namespace gideon { namespace zoneserver {
