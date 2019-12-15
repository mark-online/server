#pragma once

#include "../../Zoneserver_export.h"
#include <gideon/server/rpc/ZoneArenaRpc.h>
#include <gideon/cs/shared/data/ServerType.h>
#include <sne/server/s2s/ManagedServerSideProxy.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace gideon { namespace zoneserver {

/**
 * @class CommunityServerSideProxy
 *
 * ArenaServer-side proxy
 */
class ZoneServer_Export ArenaServerSideProxy :
    public sne::server::ManagedServerSideProxy,
	public rpc::ZoneArenaRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(ArenaServerSideProxy);

public:
    ArenaServerSideProxy(sne::server::ServerSideBridge* bridge);
    virtual ~ArenaServerSideProxy();

public:    
    ZoneId getZoneId() const {
        return zoneId_;
    }

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:

protected: // for Test
    // = sne::server::ManagedServerSideProxy overriding
    virtual void bridged();
    virtual void unbridged();

private:
    // = sne::server::ServerSideProxy overriding
    virtual bool verifyAuthentication(
        const sne::server::S2sCertificate& certificate) const;

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
    ZoneId zoneId_;
};

}} // namespace gideon { namespace zoneserver {
