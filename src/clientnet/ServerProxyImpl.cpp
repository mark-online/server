#include "ClientNetPCH.h"
#include "ServerProxyImpl.h"
#include "session/LoginSession.h"
#include "session/CommunitySession.h"
#include "session/ZoneSession.h"
#include <gideon/clientnet/ServerProxyCallback.h>

namespace gideon { namespace clientnet {

ServerProxyImpl::ServerProxyImpl(ServerProxyCallback& callback,
    const ServerProxyConfig& config) :
    callback_(callback),
    config_(config)
{
    initSessions();
}


ServerProxyImpl::~ServerProxyImpl()
{
    finiSessions();
}


void ServerProxyImpl::tick()
{
    sessionTick_.tick();
}


void ServerProxyImpl::handleMessages()
{
    if (config_.usePolling_) {
        if (loginSession_->isConnected()) {
            loginSession_->handleMessages();
        }
        if (communitySession_->isConnected()) {
            communitySession_->handleMessages();
        }
        if (zoneSession_->isConnected()) {
            zoneSession_->handleMessages();
        }
    }
}


void ServerProxyImpl::disconnect()
{
    loginSession_->disconnect();
    communitySession_->disconnect();
    zoneSession_->disconnect();
}


LoginResult ServerProxyImpl::logInLoginServer(const UserId& userId,
    const UserPassword& password)
{
    const LoginResult result = loginSession_->login(userId, password);
    if (isSucceeded(result.errorCode_)) {
        certificateMap_ = result.certificateMap_;
    }

    return result;
}


void ServerProxyImpl::logOutLoginServer()
{
    loginSession_->disconnect();
}


QueryShardInfoResult ServerProxyImpl::queryShardInfo()
{
    if (! loginSession_->isConnected()) {
        const ErrorCode errorCode = relogInLoginServer();
        if (isFailed(errorCode)) {
            return QueryShardInfoResult(errorCode);
        }
    }
    return loginSession_->queryShardInfo();
}


SelectShardResult ServerProxyImpl::selectShard(ShardId shardId)
{
    if (! loginSession_->isConnected()) {
        const ErrorCode errorCode = relogInLoginServer();
        if (isFailed(errorCode)) {
            return SelectShardResult(errorCode);
        }
    }
    return loginSession_->selectShard(shardId);
}


ReserveNicknameResult ServerProxyImpl::reserveNickname(const Nickname& nickname)
{
    if (! loginSession_->isConnected()) {
        const ErrorCode errorCode = relogInLoginServer();
        if (isFailed(errorCode)) {
            return ReserveNicknameResult(errorCode);
        }
    }
    return loginSession_->reserveNickname(nickname);
}



CreateCharacterResult ServerProxyImpl::createCharacter(const CreateCharacterInfo& createCharacterInfo)
{
    if (! loginSession_->isConnected()) {
        const ErrorCode errorCode = relogInLoginServer();
        if (isFailed(errorCode)) {
            return CreateCharacterResult(errorCode);
        }
    }
    return loginSession_->createCharacter(createCharacterInfo);
}


DeleteCharacterResult ServerProxyImpl::deleteCharacter(ObjectId characterId)
{
    if (! loginSession_->isConnected()) {
        const ErrorCode errorCode = relogInLoginServer();
        if (isFailed(errorCode)) {
            return DeleteCharacterResult(errorCode);
        }
    }
    return loginSession_->deleteCharacter(characterId);
}


EnterShardResult ServerProxyImpl::enterShard(ObjectId characterId)
{
    if (! loginSession_->isConnected()) {
        const ErrorCode errorCode = relogInLoginServer();
        if (isFailed(errorCode)) {
            return EnterShardResult(errorCode);
        }
    }
    return loginSession_->enterShard(characterId);
}


ErrorCode ServerProxyImpl::logInCommunityServer(const std::string& ip, uint16_t port)
{
    const Certificate* certificate = getCertificate(certificateMap_, stCommunityServer);
    if (! certificate) {
        assert(false);
        return ecLoginInvalidCertificate;
    }

    const CommunityLoginResult result = communitySession_->login(ip, port, *certificate);
    if (isSucceeded(result.errorCode_)) {
        certificateMap_[stCommunityServer] = result.reissuedCertificate_;
    }
    return result.errorCode_;
}


ErrorCode ServerProxyImpl::logInZoneServer(const std::string& ip, uint16_t port,
    RegionCode spawnRegionCode, ObjectId arenaId, MapCode preGlobalMapCode,
    const Position& position)
{
    const Certificate* certificate = getCertificate(certificateMap_, stZoneServer);
    if (! certificate) {
        assert(false);
        return ecLoginInvalidCertificate;
    }

    const ZoneLoginResult result = zoneSession_->login(ip, port, *certificate, spawnRegionCode, 
        arenaId, preGlobalMapCode, position);
    if (isSucceeded(result.errorCode_)) {
        certificateMap_[stZoneServer] = result.reissuedCertificate_;
    }
    return result.errorCode_;
}


bool ServerProxyImpl::reserveZoneServerLogout()
{
    if (! zoneSession_->isConnected()) {
        return false;
    }

    zoneSession_->reserveLogout();
    return true;
}


bool ServerProxyImpl::cancelZoneServerLogout()
{
    if (! zoneSession_->isConnected()) {
        return false;
    }

    zoneSession_->cancelLogout();
    return true;
}


void ServerProxyImpl::registerRpcForwarder(sne::srpc::RpcForwarder& forwarder)
{
    zoneSession_->registerRpcForwarder(forwarder);
}


void ServerProxyImpl::registerRpcReceiver(sne::srpc::RpcReceiver& receiver)
{
    zoneSession_->registerRpcReceiver(receiver);
}


void ServerProxyImpl::unregisterRpcForwarder(sne::srpc::RpcForwarder& forwarder)
{
    zoneSession_->unregisterRpcForwarder(forwarder);
}


void ServerProxyImpl::unregisterRpcReceiver(sne::srpc::RpcReceiver& receiver)
{
    zoneSession_->unregisterRpcReceiver(receiver);
}


void ServerProxyImpl::registerRpcForwarderForCommunityServer(sne::srpc::RpcForwarder& forwarder)
{
    communitySession_->registerRpcForwarder(forwarder);
}


void ServerProxyImpl::registerRpcReceiverForCommunityServer(sne::srpc::RpcReceiver& receiver)
{
    communitySession_->registerRpcReceiver(receiver);
}


void ServerProxyImpl::unregisterRpcForwarderForCommunityServer(sne::srpc::RpcForwarder& forwarder)
{
    communitySession_->unregisterRpcForwarder(forwarder);
}


void ServerProxyImpl::unregisterRpcReceiverForCommunityServer(sne::srpc::RpcReceiver& receiver)
{
    communitySession_->unregisterRpcReceiver(receiver);
}


bool ServerProxyImpl::isLoggingInCommunityServer() const
{
    return communitySession_->isLoggingIn();
}


bool ServerProxyImpl::isLoggedInCommunityServer() const
{
    return communitySession_->isLoggedIn();
}


bool ServerProxyImpl::isLoggingInZoneServer() const
{
    return zoneSession_->isLoggingIn();
}


bool ServerProxyImpl::isLoggedInZoneServer() const
{
    return zoneSession_->isLoggedIn();
}


void ServerProxyImpl::initSessions()
{
    LoginSessionCallback& loginSessionCallback = *this;
    loginSession_= std::make_unique<LoginSession>(config_,
        loginSessionCallback, sessionTick_);

    CommunitySessionCallback& communitySessionCallback = *this;
    communitySession_= std::make_unique<CommunitySession>(config_,
        communitySessionCallback, sessionTick_);

    ZoneSessionCallback& zoneSessionCallback = *this;
    zoneSession_= std::make_unique<ZoneSession>(config_,
        zoneSessionCallback, sessionTick_);
}


void ServerProxyImpl::finiSessions()
{
    if (loginSession_.get() != nullptr) {
        loginSession_->finalize();
        loginSession_.reset();
    }
    if (communitySession_.get() != nullptr) {
        communitySession_->finalize();
        communitySession_.reset();
    }
    if (zoneSession_.get() != nullptr) {
        zoneSession_->finalize();
        zoneSession_.reset();
    }
}


ErrorCode ServerProxyImpl::relogInLoginServer()
{
    const Certificate* certificate = getCertificate(certificateMap_, stLoginServer);
    if (! certificate) {
        assert(false);
        return ecLoginInvalidCertificate;
    }

    const ReloginResult result = loginSession_->relogin(*certificate);
    if (isSucceeded(result.errorCode_)) {
        certificateMap_[stLoginServer] = result.reissuedCertificate_;
    }

    return result.errorCode_;
}

// = LoginSessionCallback implementing

void ServerProxyImpl::expelled(ExpelReason reason)
{
    callback_.onExpelled(reason);
}

// = CommunitySessionCallback implementing

void ServerProxyImpl::communityServerDisconnected()
{
    callback_.onCommunityServerDisconnected();
}

// = ZoneSessionCallback implementing

void ServerProxyImpl::zoneServerDisconnected()
{
    callback_.onZoneServerDisconnected();
}

}} // namespace gideon { namespace clientnet {
