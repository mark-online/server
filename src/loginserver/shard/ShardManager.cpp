#include "LoginServerPCH.h"
#include "ShardManager.h"
#include "../s2s/LoginServerSideProxyManager.h"
#include "../c2s/LoginClientSessionCallback.h"
#include "../user/LoginUserManager.h"
#include <gideon/serverbase/database/AccountDatabase.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <sne/database/DatabaseManager.h>
#include <boost/algorithm/string/classification.hpp>

namespace gideon { namespace loginserver {

namespace {

ErrorCode checkNickname(const Nickname& nickname)
{
    using namespace boost;

    if (nickname.size() < minNicknameLength) {
        return ecCharacterNicknameTooShort;
    }
    if (! all(nickname, ! is_space() && ! is_cntrl() && ! is_punct())) {
        return ecCharacterInvalidNickname;
    }
    return ecOk;
}

} // namespace

// = ShardManager

SNE_DEFINE_SINGLETON(ShardManager);


bool ShardManager::initialize()
{
    sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

    if (! db->getShardInfos(shardInfoMap_)) {
        return false;
    }

    if (shardInfoMap_.empty()) {
        return false;
    }

    /// FYI: 존서버가 직접적으로 연결하기 전에는 모두 비활성화 상태로 설정한다
    for (FullShardInfoMap::value_type& shardValue : shardInfoMap_) {
        FullShardInfo& shardInfo = shardValue.second;
        for (ZoneServerInfoMap::value_type& zoneValue : shardInfo.zoneServerInfoMap_) {
            const ZoneId zoneId = zoneValue.first;
            ZoneServerInfo& zsi = zoneValue.second;
            zsi.isEnabled_ = false;
            zoneMapCodeMap_[zoneId] = zsi.mapCode_;
        }
    }

    shardInfoList_.reserve(shardInfoMap_.size());
    for (FullShardInfoMap::value_type& shardValue : shardInfoMap_) {
        FullShardInfo& shardInfo = shardValue.second;
        shardInfoList_.push_back(shardInfo);
    }

    return true;
}


ErrorCode ShardManager::selectShard(ShardId shardId,
    AccountId accountId, LoginClientSessionCallback& callback)
{
    std::unique_lock<LockType> lock(lock_);

    SelectShardRequest* request = getSelectShardRequest(accountId);
    if (request != nullptr) {
        return ecLoginRequestPended;
    }

    const FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        return ecShardInvalid;
    }

    const sne::server::ServerId serverId = getMostFreeZoneServerId(*shardInfo);
    if (! sne::server::isValid(serverId)) {
        return ecZoneServerIsDown;
    }

    if (! LOGINSERVERSIDEPROXY_MANAGER->requestFullUserInfo(serverId, accountId)) {
        return ecZoneServerIsDown;
    }

    addSelectShardRequest(shardId, serverId, accountId, callback);

    return ecOk;
}


ErrorCode ShardManager::reserveNickname(ShardId shardId, AccountId accountId,
    const Nickname& nickname, LoginClientSessionCallback& callback)
{
    const ErrorCode errorCode = checkNickname(nickname);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    std::unique_lock<LockType> lock(lock_);

    CreateCharacterRequest* request = getCreateCharacterRequest(accountId);
    if (request != nullptr) {
        return ecLoginRequestPended;
    }

    const FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        return ecShardInvalid;
    }    

    const sne::server::ServerId serverId = getMostFreeZoneServerId(*shardInfo);
    if (! sne::server::isValid(serverId)) {
        return ecZoneServerIsDown;
    }

    if (! LOGINSERVERSIDEPROXY_MANAGER->requestReserveNickname(serverId, accountId, nickname)) {
        return ecZoneServerIsDown;
    }

    addReserveNicknameRequest(shardId, serverId, accountId, callback);

    return ecOk;
}


ErrorCode ShardManager::createCharacter(ShardId shardId, 
    const CreateCharacterInfo& createCharacterInfo, LoginClientSessionCallback& callback)
{
    const ErrorCode errorCode = checkNickname(createCharacterInfo.nickname_);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    std::unique_lock<LockType> lock(lock_);

    CreateCharacterRequest* request = getCreateCharacterRequest(createCharacterInfo.accountId_);
    if (request != nullptr) {
        return ecLoginRequestPended;
    }

    const FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        return ecShardInvalid;
    }

    const CharacterCountPerShardMap characterCountMap =
        LOGINUSER_MANAGER->getCharacterCounts(createCharacterInfo.accountId_);
    const CharacterCountPerShardMap::const_iterator pos = characterCountMap.find(shardId);
    if (pos != characterCountMap.end()) {
        const uint8_t characterCount = (*pos).second;
        if (maxShardCharacterCount <= characterCount) {
            return ecCharacterTooMany;
        }
    }

    /// FYI: 캐릭터 생성은 무조건 첫번째 존서버에 해야 한다!
    const sne::server::ServerId serverId = shardInfo->getServerIdOfFirstZoneServer();
    if (! sne::server::isValid(serverId)) {
        return ecZoneServerIsDown;
    }

    if (! LOGINSERVERSIDEPROXY_MANAGER->requestCreateCharacter(serverId, createCharacterInfo)) {
        return ecZoneServerIsDown;
    }

    addCreateCharacterRequest(shardId, serverId, createCharacterInfo.accountId_, callback);

    return ecOk;
}


ErrorCode ShardManager::deleteCharacter(ShardId shardId, AccountId accountId,
    ObjectId characterId, LoginClientSessionCallback& callback)
{
    std::unique_lock<LockType> lock(lock_);

    DeleteCharacterRequest* request = getDeleteCharacterRequest(accountId);
    if (request != nullptr) {
        return ecLoginRequestPended;
    }

    const FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        return ecShardInvalid;
    }

    const sne::server::ServerId serverId = getMostFreeZoneServerId(*shardInfo);
    if (! sne::server::isValid(serverId)) {
        return ecZoneServerIsDown;
    }

    if (! LOGINSERVERSIDEPROXY_MANAGER->requestDeleteCharacter(serverId, accountId,
        characterId)) {
        return ecZoneServerIsDown;
    }

    addDeleteCharacterRequest(shardId, serverId, accountId, callback);

    return ecOk;
}


// TODO: 최적화 방안 모색
void ShardManager::cancelRequests(AccountId accountId, LoginClientSessionCallback* callback)
{
    std::unique_lock<LockType> lock(lock_);

    {
        EnterShardRequestMap::iterator pos = selectShardRequestMap_.begin();
        for (; pos != selectShardRequestMap_.end();) {
            if ((*pos).first == accountId) {
                SelectShardRequest& request = (*pos).second;
                if (&request.sessionCallback_ == callback) {
                    selectShardRequestMap_.erase(pos++);
                    continue;
                }
            }
            ++pos;
        }
    }

    {
        CreateCharacterRequestMap::iterator pos = createCharacterRequestMap_.begin();
        for (; pos != createCharacterRequestMap_.end();) {
            if ((*pos).first == accountId) {
                CreateCharacterRequest& request = (*pos).second;
                if (&request.sessionCallback_ == callback) {
                    createCharacterRequestMap_.erase(pos++);
                    continue;
                }
            }
            ++pos;
        }
    }

    {
        DeleteCharacterRequestMap::iterator pos = deleteCharacterRequestMap_.begin();
        for (; pos != deleteCharacterRequestMap_.end();) {
            if ((*pos).first == accountId) {
                DeleteCharacterRequest& request = (*pos).second;
                if (&request.sessionCallback_ == callback) {
                    deleteCharacterRequestMap_.erase(pos++);
                    continue;
                }
            }
            ++pos;
        }
    }

    {
        ReserveNicknameRequestMap::iterator pos = reserveNicknameRequestMap_.begin();
        while (pos != reserveNicknameRequestMap_.end()) {
            if ((*pos).first == accountId) {
                ReserveNicknameRequest& request = (*pos).second;
                if (&request.sessionCallback_ == callback) {
                    reserveNicknameRequestMap_.erase(pos++);
                    continue;
                }
            }
            ++pos;
        }
    }
}


void ShardManager::zoneServerConnected(sne::server::ServerId serverId,
    ShardId shardId, ZoneId zoneId,
    uint16_t currentUserCount, uint16_t maxUserCount)
{
    std::unique_lock<LockType> lock(lock_);

    FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        assert(false);
        return;
    }

    currentUserCountMap_[zoneId] = currentUserCount;
    maxUserCountMap_[zoneId] = maxUserCount;

    shardInfo->increaseUserCount(currentUserCount, maxUserCount);
    shardInfo->zoneServerIdMap_[zoneId] = serverId;

    {
        ShardInfo* aShardInfo = gideon::getShardInfo(shardInfoList_, shardId);
        if (aShardInfo != nullptr) {
            aShardInfo->status_ = ssOffline;
            aShardInfo->increaseUserCount(currentUserCount, maxUserCount);
        }
    }
}


void ShardManager::zoneServerReadied(ShardId shardId, ZoneId zoneId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        ZoneServerInfo* zoneServerInfo = getZoneServerInfo_i(shardId, zoneId);
        if (! zoneServerInfo) {
            assert(false);
            return;
        }
        zoneServerInfo->isEnabled_ = true;

        FullShardInfo* shardInfo = getShardInfo(shardId);
        assert(shardInfo != nullptr);
        shardInfo->status_ = ssOnline;

        {
            ShardInfo* aShardInfo = gideon::getShardInfo(shardInfoList_, shardId);
            if (aShardInfo != nullptr) {
                aShardInfo->status_ = ssOnline;
            }
        }
    }

    LOGINSERVERSIDEPROXY_MANAGER->zoneServerConnected(shardId, zoneId);
}


void ShardManager::zoneServerDisconnected(ShardId shardId, ZoneId zoneId,
    sne::server::ServerId serverId)
{
    {
        std::unique_lock<LockType> lock(lock_);

        ZoneServerInfo* zoneServerInfo = getZoneServerInfo_i(shardId, zoneId);
        if (! zoneServerInfo) {
            assert(false);
            return;
        }
        zoneServerInfo->isEnabled_ = false;

        FullShardInfo* shardInfo = getShardInfo(shardId);
        assert(shardInfo != nullptr);

        const uint16_t currentUserCount = currentUserCountMap_[zoneId];
        const uint16_t maxUserCount = maxUserCountMap_[zoneId];

        currentUserCountMap_.erase(zoneId);
        maxUserCountMap_.erase(zoneId);

        shardInfo->decreaseUserCount(currentUserCount, maxUserCount);
        shardInfo->zoneServerIdMap_[zoneId] = sne::server::ServerId::invalid;

        const bool isAnyZoneServerEnabled = shardInfo->isAnyZoneServerEnabled();
        if (! isAnyZoneServerEnabled) {
            shardInfo->status_ = ssOffline;
        }

        {
            ShardInfo* aShardInfo = gideon::getShardInfo(shardInfoList_, shardId);
            if (aShardInfo != nullptr) {
                aShardInfo->status_ = ssOffline;
                aShardInfo->decreaseUserCount(currentUserCount, maxUserCount);
            }
        }

        removeAllRequest(serverId);
    }

    LOGINSERVERSIDEPROXY_MANAGER->zoneServerDisconnected(shardId, zoneId);
}


void ShardManager::userLoggedIn(ShardId shardId, ZoneId zoneId)
{
    std::unique_lock<LockType> lock(lock_);

    FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        SNE_LOG_ERROR("ShardManager::userLoggedIn(S%u,Z%u) - shard not found.",
            shardId, zoneId);
        return;
    }

    ++currentUserCountMap_[zoneId];

    shardInfo->increaseUserCount(1, 0);

    {
        ShardInfo* aShardInfo = gideon::getShardInfo(shardInfoList_, shardId);
        if (aShardInfo != nullptr) {
            aShardInfo->increaseUserCount(1, 0);
        }
    }
}


void ShardManager::userLoggedOut(ShardId shardId, ZoneId zoneId)
{
    std::unique_lock<LockType> lock(lock_);

    FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        SNE_LOG_ERROR("ShardManager::userLoggedOut(S%u,Z%u) - shard not found.",
            shardId, zoneId);
        return;
    }

    if (currentUserCountMap_[zoneId] > 0) {
        --currentUserCountMap_[zoneId];
    }
    shardInfo->decreaseUserCount(1, 0);

    {
        ShardInfo* aShardInfo = gideon::getShardInfo(shardInfoList_, shardId);
        if (aShardInfo != nullptr) {
            aShardInfo->decreaseUserCount(1, 0);
        }
    }
}


void ShardManager::shardSelected(ErrorCode errorCode, const FullUserInfo& userInfo)
{
    assert(isValidAccountId(userInfo.accountId_));

    LoginClientSessionCallback* sessionCallback = nullptr;
    ShardId shardId = invalidShardId;
    {
        std::unique_lock<LockType> lock(lock_);

        SelectShardRequest* request = getSelectShardRequest(userInfo.accountId_);
        if (! request) {
            return;
        }

        sessionCallback = &request->sessionCallback_;
        shardId = request->shardId_;

        deleteSelectShardRequest(userInfo.accountId_);
    }

    if (! sessionCallback) {
        return;
    }

    if (isSucceeded(errorCode)) {
        LOGINUSER_MANAGER->shardChanged(userInfo, shardId);
    }

    sessionCallback->shardSelected(errorCode, userInfo);
}


void ShardManager::characterCreated(ErrorCode errorCode,
    AccountId accountId, const FullCharacterInfo& characterInfo)
{
    assert(isValidAccountId(accountId));

    LoginClientSessionCallback* sessionCallback = nullptr;
    ShardId shardId = invalidShardId;
    {
        std::unique_lock<LockType> lock(lock_);

        CreateCharacterRequest* request = getCreateCharacterRequest(accountId);
        if (! request) {
            return;
        }

        sessionCallback = &request->sessionCallback_;
        shardId = request->shardId_;

        deleteCreateCharacterRequest(accountId);
    }

    if (! sessionCallback) {
        return;
    }

    if (isSucceeded(errorCode)) {
        LOGINUSER_MANAGER->characterCreated(shardId, accountId, characterInfo);
    }

    sessionCallback->characterCreated(errorCode, characterInfo);
}


void ShardManager::characterDeleted(ErrorCode errorCode,
    AccountId accountId, ObjectId characterId)
{
    assert(isValidAccountId(accountId));

    LoginClientSessionCallback* sessionCallback = nullptr;
    ShardId shardId = invalidShardId;
    {
        std::unique_lock<LockType> lock(lock_);

        DeleteCharacterRequest* request = getDeleteCharacterRequest(accountId);
        if (! request) {
            return;
        }

        sessionCallback = &request->sessionCallback_;
        shardId = request->shardId_;

        deleteDeleteCharacterRequest(accountId);
    }

    if (! sessionCallback) {
        return;
    }

    if (isSucceeded(errorCode)) {
        LOGINUSER_MANAGER->characterDeleted(shardId, accountId, characterId);
    }

    sessionCallback->characterDeleted(errorCode, characterId);
}


void ShardManager::nicknameReserved(ErrorCode errorCode, AccountId accountId, const Nickname& nickname)
{
    assert(isValidAccountId(accountId));

    LoginClientSessionCallback* sessionCallback = nullptr;
    ShardId shardId = invalidShardId;
    {
        std::unique_lock<LockType> lock(lock_);

        ReserveNicknameRequest* request = getReserveNicknameRequest(accountId);
        if (! request) {
            return;
        }

        sessionCallback = &request->sessionCallback_;
        shardId = request->shardId_;

        deleteReserveNicknameRequest(accountId);
    }

    if (! sessionCallback) {
        return;
    }

    sessionCallback->nicknameReserved(errorCode, nickname);
}


ShardInfoList ShardManager::getShardInfoList() const
{
    std::unique_lock<LockType> lock(lock_);

    return shardInfoList_;
}


ZoneMapCodeMap ShardManager::getZoneMapCodeMap() const
{
    std::unique_lock<LockType> lock(lock_);

    return zoneMapCodeMap_;
}


ZoneServerInfo ShardManager::getZoneServerInfo(ShardId shardId, ZoneId zoneId) const
{
    std::unique_lock<LockType> lock(lock_);

    const ZoneServerInfo* zoneServerInfo = getZoneServerInfo_i(shardId, zoneId);
    if (zoneServerInfo != nullptr) {
        return *zoneServerInfo;
    }
    return ZoneServerInfo();
}


FullShardInfo* ShardManager::getShardInfo(ShardId shardId)
{
    const FullShardInfoMap::iterator pos = shardInfoMap_.find(shardId);
    if (pos != shardInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


const FullShardInfo* ShardManager::getShardInfo(ShardId shardId) const
{
    const FullShardInfoMap::const_iterator pos = shardInfoMap_.find(shardId);
    if (pos != shardInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


ZoneServerInfo* ShardManager::getZoneServerInfo_i(ShardId shardId, ZoneId zoneId)
{
    FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        return nullptr;
    }

    return shardInfo->getZoneServerInfo(zoneId);
}


const ZoneServerInfo* ShardManager::getZoneServerInfo_i(ShardId shardId, ZoneId zoneId) const
{
    const FullShardInfo* shardInfo = getShardInfo(shardId);
    if (! shardInfo) {
        return nullptr;
    }

    return shardInfo->getZoneServerInfo(zoneId);
}


sne::server::ServerId ShardManager::getMostFreeZoneServerId(const FullShardInfo& shardInfo) const
{
    sne::server::ServerId mostFreeZoneServerId = sne::server::ServerId::invalid;
    size_t mostFreeUserCount = (std::numeric_limits<size_t>::max)();
    for (const CountMap::value_type& value : currentUserCountMap_) {
        const ZoneId zoneId = value.first;
        const size_t userCount = value.second;
        if (userCount > mostFreeUserCount) {
            continue;
        }

        const ZoneServerInfo* zoneServerInfo = shardInfo.getZoneServerInfo(value.first);
        if (! zoneServerInfo) {
            assert(false);
            continue;
        }

        if (! zoneServerInfo->isEnabled_) {
            continue;
        }

        if (! isWorldMap(getMapType(zoneServerInfo->mapCode_))) {
            continue;
        }

        mostFreeZoneServerId = shardInfo.getServerId(zoneId);
        mostFreeUserCount = userCount;
    }

    return mostFreeZoneServerId;
}


void ShardManager::removeAllRequest(sne::server::ServerId serverId)
{
    EnterShardRequestMap::iterator posEnter = selectShardRequestMap_.begin();
    while (posEnter != selectShardRequestMap_.end()) {
        SelectShardRequest& request = (*posEnter).second;
        if (request.serverId_ == serverId) {
            request.sessionCallback_.shardSelected(ecZoneServerIsDown, FullUserInfo());
            selectShardRequestMap_.erase(posEnter++);
        }
        else {
            ++posEnter;
        }
    }

    CreateCharacterRequestMap::iterator posCreate = createCharacterRequestMap_.begin();
    while (posCreate != createCharacterRequestMap_.end()) {
        CreateCharacterRequest& request = (*posCreate).second;
        if (request.serverId_ == serverId) {
            request.sessionCallback_.characterCreated(ecZoneServerIsDown, FullCharacterInfo());
            createCharacterRequestMap_.erase(posCreate++);
        }
        else {
            ++posCreate;
        }
    }

    DeleteCharacterRequestMap::iterator posDelete = deleteCharacterRequestMap_.begin();
    while (posDelete != deleteCharacterRequestMap_.end()) {
        DeleteCharacterRequest& request = (*posDelete).second;
        if (request.serverId_ == serverId) {
            request.sessionCallback_.characterDeleted(ecZoneServerIsDown, invalidObjectId);
            deleteCharacterRequestMap_.erase(posDelete++);
        }
        else {
            ++posDelete;
        }
    }

    ReserveNicknameRequestMap::iterator posReserve = reserveNicknameRequestMap_.begin();
    while (posReserve != reserveNicknameRequestMap_.end()) {
        ReserveNicknameRequest& request = (*posReserve).second;
        if (request.serverId_ == serverId) {
            request.sessionCallback_.nicknameReserved(ecZoneServerIsDown, L"");
            reserveNicknameRequestMap_.erase(posReserve++);
        }
        else {
            ++posReserve;
        }
    }
}

}} // namespace gideon { namespace loginserver {
