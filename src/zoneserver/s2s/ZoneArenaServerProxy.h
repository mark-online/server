#pragma once

#include "../zoneserver_export.h"
#include <gideon/server/rpc/ZoneArenaRpc.h>
#include <sne/server/s2s/ClientSideProxy.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace zoneserver {

/**
 * @class ZoneArenaServerProxy
 *
 *  투기장 서버 Proxy
 */
class ZoneServer_Export ZoneArenaServerProxy :
	public rpc::ZoneArenaRpc,
    public sne::server::ClientSideProxy,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(ZoneArenaServerProxy);

public:
    ZoneArenaServerProxy(const sne::server::ServerInfo& serverInfo,
        const sne::server::ClientSideSessionConfig& config =
            sne::server::ClientSideSessionConfig());
    virtual ~ZoneArenaServerProxy();

    void ready();

public:
    virtual bool isActivated() const {
        return sne::server::ClientSideProxy::isActivated() &&
            isRendezvoused_;
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
	OVERRIDE_SRPC_METHOD_1(z2a_rendezvous,
		ZoneId, zoneId);
	OVERRIDE_SRPC_METHOD_0(z2a_onRendezvous);

	OVERRIDE_SRPC_METHOD_0(z2a_ready);

    OVERRIDE_SRPC_METHOD_1(z2a_releaseDeserter,
        ObjectId, playerId);


	OVERRIDE_SRPC_METHOD_2(z2a_challengeSolo,
		ObjectId, playerId, MapCode, modeMapCode);
	OVERRIDE_SRPC_METHOD_3(z2a_onChallengeSolo,
		ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode);

	OVERRIDE_SRPC_METHOD_2(z2a_cancelChallenge,
		ObjectId, playerId, MapCode, modeMapCode);
	OVERRIDE_SRPC_METHOD_3(z2a_onCancelChallenge,
		ErrorCode, errorCode, ObjectId, playerId, MapCode, modeMapCode);

    OVERRIDE_SRPC_METHOD_1(z2a_logoutChallenger,
        ObjectId, playerId);

	OVERRIDE_SRPC_METHOD_4(z2a_evArenaMatched,
		ObjectId, arenaId, ObjectId, playerId, MapCode, modeMapCode, MapCode, arenaWorldMapCode);


private:
    // = sne::server::ClientSideProxy overriding
    virtual void onAuthenticated();
    virtual void onDisconnected();

private:
    bool isRendezvoused_;
    bool isReadied_;
};

}} // namespace gideon { namespace zoneserver {
