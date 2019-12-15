#include "ZoneServerPCH.h"
#include "ZoneLoginServerProxy.h"
#include "../ZoneService.h"
#include "../service/shard/ShardService.h"
#include "../world/World.h"
#include "../user/ZoneUserManager.h"
#include "../user/ZoneUser.h"
#include <gideon/cs/shared/data/ServerType.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace zoneserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(ZoneLoginServerProxy);

ZoneLoginServerProxy::ZoneLoginServerProxy(
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


ZoneLoginServerProxy::~ZoneLoginServerProxy()
{
}


void ZoneLoginServerProxy::ready()
{
    isReadied_ = true;

    z2l_ready();
}

// = sne::server::ClientSideProxy overriding

void ZoneLoginServerProxy::onAuthenticated()
{
    sne::server::ClientSideProxy::onAuthenticated();

    SNE_LOG_INFO("LoginServer is connected.");

    z2l_rendezvous(WORLD->getShardId(), WORLD->getZoneId(),
        ZONEUSER_MANAGER->getOnlineUsers(), uint16_t(ZONEUSER_MANAGER->getMaxUserCount()));
}


void ZoneLoginServerProxy::onDisconnected()
{
    sne::server::ClientSideProxy::onDisconnected();

    SNE_LOG_ERROR("LoginServer is disconnected.");

    isRendezvoused_ = false;
}

// = sne::srpc::RpcForwarder overriding

void ZoneLoginServerProxy::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneLoginServerProxy::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void ZoneLoginServerProxy::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("ZoneLoginServerProxy::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::ZoneLoginRpc overriding

FORWARD_SRPC_METHOD_4(ZoneLoginServerProxy, z2l_rendezvous,
    ShardId, shardId, ZoneId, zoneId, AccountIds, onlineUsers, uint16_t, maxUserCount);


RECEIVE_SRPC_METHOD_0(ZoneLoginServerProxy, z2l_onRendezvous)
{
    isRendezvoused_ = true;

    if (isReadied_) {
        z2l_ready();
    }
}


FORWARD_SRPC_METHOD_0(ZoneLoginServerProxy, z2l_ready);


FORWARD_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_loginZoneUser,
    Certificate, certificate);


RECEIVE_SRPC_METHOD_3(ZoneLoginServerProxy, z2l_onLoginZoneUser,
    ErrorCode, errorCode, AccountInfo, accountInfo, Certificate, reissuedCertificate)
{
    ZONEUSER_MANAGER->loginResponsed(errorCode, accountInfo, reissuedCertificate);
}


FORWARD_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_reserveMigration,
    AccountId, accountId);


FORWARD_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_logoutUser,
    AccountId, accountId);


RECEIVE_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_getFullUserInfo,
    AccountId, accountId)
{
    ShardService::fullUserInfoRequested(accountId);
}


FORWARD_SRPC_METHOD_2(ZoneLoginServerProxy, z2l_onGetFullUserInfo,
    ErrorCode, errorCode, FullUserInfo, userInfo);


RECEIVE_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_createCharacter,
    CreateCharacterInfo, createCharacterInfo)
{
    ShardService::createCharacterRequested(createCharacterInfo);
}


FORWARD_SRPC_METHOD_3(ZoneLoginServerProxy, z2l_onCreateCharacter,
    ErrorCode, errorCode, AccountId, accountId, FullCharacterInfo, characterInfo);


RECEIVE_SRPC_METHOD_2(ZoneLoginServerProxy, z2l_deleteCharacter,
    AccountId, accountId, ObjectId, characterId)
{
    ShardService::deleteCharacterRequested(accountId, characterId);
}


FORWARD_SRPC_METHOD_3(ZoneLoginServerProxy, z2l_onDeleteCharacter,
    ErrorCode, errorCode, AccountId, accountId, ObjectId, characterId);


RECEIVE_SRPC_METHOD_2(ZoneLoginServerProxy, z2l_checkDuplicateNickname,
    AccountId, accountId, Nickname, nickname)
{
    ShardService::checkDuplicateNicknameRequested(accountId, nickname);
}


FORWARD_SRPC_METHOD_3(ZoneLoginServerProxy, z2l_onCheckDuplicateNickname,
    ErrorCode, errorCode, AccountId, accountId, Nickname, nickname);



RECEIVE_SRPC_METHOD_2(ZoneLoginServerProxy, z2l_evUserExpelled,
    AccountId, accountId, ExpelReason, expelReason)
{
    ZONEUSER_MANAGER->userExpelled(accountId, expelReason);
}


RECEIVE_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_evZoneServerActivated,
    ZoneId, zoneId)
{
    ZONE_SERVICE->zoneServerActivated(zoneId);
}


RECEIVE_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_evZoneServerDeactivated,
    ZoneId, zoneId)
{
    ZONE_SERVICE->zoneServerDeactivated(zoneId);
}


FORWARD_SRPC_METHOD_1(ZoneLoginServerProxy, z2l_evAddBanInfo,
	BanInfo, banInfo);

FORWARD_SRPC_METHOD_4(ZoneLoginServerProxy, z2l_evRemoveBanInfo,
	BanMode, banMode, AccountId, accountId, ObjectId, characterId, std::string, ipAddress);


}} // namespace gideon { namespace zoneserver {
