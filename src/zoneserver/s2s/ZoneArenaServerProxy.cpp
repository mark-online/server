#include "ZoneServerPCH.h"
#include "ZoneArenaServerProxy.h"
#include "../ZoneService.h"
#include "../world/World.h"
#include "../model/gameobject/Entity.h"
#include "../controller/EntityController.h"
#include "../controller/callback/ArenaCallback.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(ZoneArenaServerProxy);

ZoneArenaServerProxy::ZoneArenaServerProxy(
    const sne::server::ServerInfo& serverInfo,
    const sne::server::ClientSideSessionConfig& config) :
    sne::server::ClientSideProxy(serverInfo, config),
    isRendezvoused_(false),
    isReadied_(false)
{
    sne::base::Session* session = getSession();
    if (session != nullptr) {
        sne::sgp::RpcingExtension* extension =
            session->getImpl().getExtension<sne::sgp::RpcingExtension>();
        extension->registerRpcForwarder(*this);
        extension->registerRpcReceiver(*this);
    }
}


ZoneArenaServerProxy::~ZoneArenaServerProxy()
{
}


void ZoneArenaServerProxy::ready()
{
    isReadied_ = true;

    z2a_ready();
}


// = sne::server::ClientSideProxy overriding

void ZoneArenaServerProxy::onAuthenticated()
{
    sne::server::ClientSideProxy::onAuthenticated();

    SNE_LOG_INFO("ArenaServer is connected.");

    z2a_rendezvous(WORLD->getZoneId());
}


void ZoneArenaServerProxy::onDisconnected()
{
    sne::server::ClientSideProxy::onDisconnected();

    SNE_LOG_ERROR("LoginServer is disconnected.");

    isRendezvoused_ = false;
}


FORWARD_SRPC_METHOD_1(ZoneArenaServerProxy, z2a_rendezvous,
	ZoneId, zoneId);


RECEIVE_SRPC_METHOD_0(ZoneArenaServerProxy, z2a_onRendezvous)
{
	isRendezvoused_ = true;

	//ZONE_SERVICE->communityServerConnected();

	if (isReadied_) {
		ready();
	}
}


FORWARD_SRPC_METHOD_0(ZoneArenaServerProxy, z2a_ready);


FORWARD_SRPC_METHOD_1(ZoneArenaServerProxy, z2a_releaseDeserter,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_2(ZoneArenaServerProxy, z2a_challengeSolo,
	ObjectId, playerId, MapCode, modeMapCode);


RECEIVE_SRPC_METHOD_3(ZoneArenaServerProxy, z2a_onChallengeSolo,
	ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode)
{
	go::Entity* player = WORLD->getPlayer(playerId);
	if (! player) {
		return;
	}

	gc::ArenaCallback* callback = player->getController().queryArenaCallback();
	if (callback) {
		callback->soloChallenged(errorCode, modeMapCode);
	}
}


FORWARD_SRPC_METHOD_2(ZoneArenaServerProxy, z2a_cancelChallenge,
	ObjectId, playerId, MapCode, modeMapCode);


RECEIVE_SRPC_METHOD_3(ZoneArenaServerProxy, z2a_onCancelChallenge,
	ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode)
{
	go::Entity* player = WORLD->getPlayer(playerId);
	if (! player) {
		return;
	}

	gc::ArenaCallback* callback = player->getController().queryArenaCallback();
	if (callback) {
		callback->challengeCanceled(errorCode, modeMapCode);
	}
}


FORWARD_SRPC_METHOD_1(ZoneArenaServerProxy, z2a_logoutChallenger,
    ObjectId, playerId);


RECEIVE_SRPC_METHOD_4(ZoneArenaServerProxy, z2a_evArenaMatched,
	ObjectId, arenaId, ObjectId, playerId, MapCode, modeMapCode, MapCode, arenaWorldMapCode)
{
	go::Entity* player = WORLD->getPlayer(playerId);
	if (! player) {
		return;
	}

	gc::ArenaCallback* callback = player->getController().queryArenaCallback();
	if (callback) {
		callback->arenaMatched(arenaId, modeMapCode, arenaWorldMapCode);
	}
}


// = sne::srpc::RpcForwarder overriding

void ZoneArenaServerProxy::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneArenaServerProxy::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void ZoneArenaServerProxy::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneArenaServerProxy::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}} // namespace gideon { namespace zoneserver {
