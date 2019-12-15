#include "ZoneServerPCH.h"
#include "ZoneUserImpl.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneLoginServerProxy.h"
#include "../../c2s/ZoneClientSessionCallback.h"
#include "../../ZoneService.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../world/WorldClock.h"
#include "../../world/region/MapRegion.h"
#include "../../model/gameobject/Player.h"
#include "../../model/gameobject/allocator/EntityAllocator.h"
#include "../../model/gameobject/status/CreatureStatus.h"
#include "../../controller/PlayerController.h"
#include "../../service/item/ItemIdGenerator.h"
#include "../../service/arena/ArenaService.h"
#include "../../service/arena/mode/Arena.h"
#include <gideon/server/data/CreateCharacterInfo.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/base/concurrent/Future.h>
#include <sne/base/session/impl/SessionImpl.h>
#include <sne/base/session/Session.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

IMPLEMENT_SRPC_EVENT_DISPATCHER(ZoneUserImpl);

ZoneUserImpl::ZoneUserImpl(go::EntityAllocator& playerAllocator)
{
    activePlayer_.reset(static_cast<go::Player*>(playerAllocator.malloc()));
    reset();
}


ZoneUserImpl::~ZoneUserImpl()
{
}


void ZoneUserImpl::initialize(const AccountInfo& accountInfo, const FullUserInfo& userInfo,
    AuthId authId, RegionCode spawnRegionCode, ObjectId arenaId, MapCode preGlobalWorldMapCode,
    const Position& position, ZoneClientSessionCallback& sessionCallback)
{
    SNE_ASSERT(userInfo.isValid());

    std::lock_guard<LockType> lock(lock_);

    userInfo_ = userInfo;
    accountInfo_ = accountInfo;
    authId_ = authId;
    spawnRegionCode_ = spawnRegionCode;
    arenaId_ = arenaId;
    preGlobalWorldMapCode_ = preGlobalWorldMapCode;
    position_ = position;
    sessionCallback_ = &sessionCallback;
    clientId_ = sessionCallback_->getCurrentClientId();

    sne::base::Session* session = sessionCallback_->getCurrentSession();
    if (session != nullptr) {
        sne::sgp::RpcingExtension* rpcExtension =
            session->getExtension<sne::sgp::RpcingExtension>();
        rpcExtension->registerRpcForwarder(*this);
        rpcExtension->registerRpcReceiver(*this);
    }
}


void ZoneUserImpl::finalize()
{
    std::lock_guard<LockType> lock(lock_);

    sne::base::Session* session = sessionCallback_->getCurrentSession();
    if (session != nullptr) {
        sne::sgp::RpcingExtension* rpcExtension =
            session->getExtension<sne::sgp::RpcingExtension>();
        rpcExtension->unregisterRpcForwarder(*this);
        rpcExtension->unregisterRpcReceiver(*this);
    }

    reset();
}


void ZoneUserImpl::characterAdded(const FullCharacterInfo& characterInfo) 
{
    std::lock_guard<LockType> lock(lock_);

    userInfo_.characters_.insert(
        FullCharacterInfos::value_type(characterInfo.objectId_, characterInfo));
}


void ZoneUserImpl::characterDeleted(ObjectId characterId) 
{
    std::lock_guard<LockType> lock(lock_);

    userInfo_.characters_.erase(characterId);
}


void ZoneUserImpl::expelledFromServer(ExpelReason expelReason)
{
    evExpelled(expelReason);

    std::lock_guard<LockType> lock(lock_);

    if (! userInfo_.isValid()) {
        return;
    }

    sessionCallback_->expelledFromServer();
}


void ZoneUserImpl::reserveMigration()
{
    if (! userInfo_.isValid()) {
        return;
    }

    sessionCallback_->reserveMigration();
}


bool ZoneUserImpl::leaveFromWorld()
{
    // this를 동기화하면 dead-lock 가능성이 생김

    if (! activePlayer_->isValid()) {
        return false;
    }

    activePlayer_->saveCharacterStats();

    const ErrorCode errorCode = static_cast<go::Entity*>(activePlayer_.get())->despawn();
    if (isFailed(errorCode)) {       
        SNE_LOG_ERROR(__FUNCTION__ "(A%" PRIu64 ",%u)",
            static_cast<go::Entity*>(activePlayer_.get())->getAccountId(), errorCode);
        return false;
    }

    return true;
}


ObjectId ZoneUserImpl::getActivateCharacterId() const
{
    std::lock_guard<LockType> lock(lock_);

    if( activePlayer_.get() == nullptr) {
        return invalidObjectId;
    }
    return activePlayer_->getObjectId();
}


FullUserInfo ZoneUserImpl::getUserInfo() const
{
    std::lock_guard<LockType> lock(lock_);

    return userInfo_;
}


bool ZoneUserImpl::isCharacterSelected() const
{
    return activePlayer_->isValid();
}


bool ZoneUserImpl::hasNickname(const Nickname& nickname) const
{
    std::lock_guard<LockType> lock(lock_);

    for (const FullCharacterInfos::value_type& value : userInfo_.characters_) {
        const FullCharacterInfo& character = value.second;
        if (character.nickname_ == nickname) {
            return true;
        }
    }
    return false;
}


bool ZoneUserImpl::isLoggingOut() const
{
    return sessionCallback_->isLogoutReserved();
}


void ZoneUserImpl::reset()
{
    activePlayer_->finalize();

    sessionCallback_ = nullptr;
    clientId_ = sne::server::ClientId::invalid;
    userInfo_.reset();
    authId_ = invalidAuthId;
    spawnRegionCode_ = invalidRegionCode;
    arenaId_ = invalidObjectId;
    preGlobalWorldMapCode_ = invalidMapCode;
}


ErrorCode ZoneUserImpl::prepareCharacter(ObjectId characterId)
{
    std::lock_guard<LockType> lock(lock_);

    assert(sessionCallback_ != nullptr);

    FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
    if (! characterInfo) {
        return ecCharacterNotFound;
    }

    if (isCharacterSelected()) {
        return ecWorldMapAlreadyEntered;
    }

    if (activePlayer_->isEnteredAtWorld()) {
        return ecWorldMapAlreadyEntered;
    }
    
    const ErrorCode errorCode = activePlayer_->initialize(userInfo_.accountId_,
        *characterInfo, sessionCallback_->getCurrentSession(), accountInfo_.accountGrade_);
    if (isFailed(errorCode)) {
        activePlayer_->finalize();
        // TODO:
        assert(false);
        return errorCode;
    }

    return ecOk;
}


void ZoneUserImpl::unprepareCharacter()
{
    std::lock_guard<LockType> lock(lock_);

    activePlayer_->finalize();
}


ErrorCode ZoneUserImpl::enterWorld(ObjectPosition& spawnPosition)
{
    std::lock_guard<LockType> lock(lock_);

    if (! activePlayer_->isValid()) {
        return ecCharacterNotFound;
    }

    if (activePlayer_->isEnteredAtWorld()) {
        return ecWorldMapAlreadyEntered;
    }

    const ErrorCode enterErrorCode =
        WORLD->enterWorld(spawnPosition, *activePlayer_, spawnRegionCode_, arenaId_, preGlobalWorldMapCode_,
        activePlayer_->getZoneId(), position_);
    if (isFailed(enterErrorCode)) {
        return enterErrorCode;
    }

    activePlayer_->queryPositionable()->setPosition(spawnPosition);
    //characterInfo->zoneId_ = WORLD->getZoneId();
    return ecOk;
}


bool ZoneUserImpl::canRequest() const
{
    std::lock_guard<LockType> lock(lock_);

    if (! isValid()) {
        return false;
    }

    if (! userInfo_.isValid()) {
        return false;
    }

    if (! sessionCallback_) {
        return false;
    }

    return true;
}

// = sne::srpc::RpcForwarder overriding

void ZoneUserImpl::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG(__FUNCTION__ "(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void ZoneUserImpl::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG(__FUNCTION__ "(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = rpc::AccountRpc overriding

RECEIVE_SRPC_METHOD_1(ZoneUserImpl, selectCharacter,
    ObjectId, characterId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! canRequest()) {
        return;
    }

    const ErrorCode errorCode = prepareCharacter(characterId);
    if (isSucceeded(errorCode)) {
        const FullCharacterInfo* characterInfo = userInfo_.getCharacter(characterId);
        assert(characterInfo != nullptr);
        onSelectCharacter(errorCode, *characterInfo);	
    }
    else {
        unprepareCharacter();
        onSelectCharacter(errorCode, FullCharacterInfo());	
    }
}


RECEIVE_SRPC_METHOD_0(ZoneUserImpl, enterWorld)
{
    sne::server::Profiler profiler(__FUNCTION__);

    if (! canRequest()) {
        return;
    }

    ObjectPosition spawnPosition;
    ErrorCode errorCode = enterWorld(spawnPosition);
    if (isSucceeded(errorCode)) {
        WorldMap* lastWorldMap = activePlayer_->getCurrentWorldMap();
        if (! lastWorldMap) {
            errorCode = ecZoneInvalidSpawnPosition;
        }
    }

    if (isSucceeded(errorCode)) {
        onEnterWorld(errorCode, spawnPosition, activePlayer_->getWorldPosition().mapCode_,
            WORLD->getWorldClock().getWorldTime());	
    }
    else {
        unprepareCharacter();
        onEnterWorld(errorCode, spawnPosition, invalidMapCode, minWorldTime);	
    }
}


FORWARD_SRPC_METHOD_2(ZoneUserImpl, onSelectCharacter,
    ErrorCode, errorCode, FullCharacterInfo, characterInfo);


FORWARD_SRPC_METHOD_4(ZoneUserImpl, onEnterWorld,
    ErrorCode, errorCode, ObjectPosition, spawnPosition, MapCode, mapCode,
    WorldTime, worldTime);


FORWARD_SRPC_METHOD_1(ZoneUserImpl, evExpelled,
    ExpelReason, reason);

}} // namespace gideon { namespace zoneserver {
