#pragma once

#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/ServerType.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <sne/server/s2s/ServerId.h>
#include "esut/Random.h"

namespace gideon { namespace loginserver {

class LoginClientSessionCallback;

/**
 * @class LoginUser
 */
class LoginUser
{
    typedef sne::core::HashMap<ObjectId, ZoneId> CharacterZoneMap;

public:
    LoginUser(LoginClientSessionCallback* callback,
        const AccountInfo& accountInfo,
        const CharacterCountPerShardMap& characterCounts) :
        callback_(callback),
		accountInfo_(accountInfo),
        characterCounts_(characterCounts),
        zoneServerId_(sne::server::ServerId::invalid),
        communityServerId_(sne::server::ServerId::invalid),
        loginAt_(getTime()),
        loginCount_(0),
        migratingAt_(-1) {
        issueCertificates();
    }

    void disconnect();

    void expelled(ExpelReason reason);

    void reserveMigration() {
        migratingAt_ = getTime();
    }

public:
    void loginLoginServer() {
        increateLoginCount();
    }

    ErrorCode loginZoneServer(sne::server::ServerId zoneServerId, ShardId shardId,
        const Certificate& certificate);

    ErrorCode loginCommunityServer(sne::server::ServerId communityServerId,
        const Certificate& certificate);

    void logout(ServerType serverType, LoginClientSessionCallback* callback = nullptr) {
        if ((serverType == stLoginServer) && (callback_ != nullptr) && (callback_ != callback)) {
            return;
        }

        if (serverType == stZoneServer) {
            zoneServerId_ = sne::server::ServerId::invalid;
        }

        decreaseLoginCount();
    }

    ErrorCode relogin(const Certificate& certificate) {
        const ServerType serverType = stLoginServer;
        return checkCertificate(certificate, serverType);
    }

public:
    void loggedInZoneServer(sne::server::ServerId zoneServerId, ShardId shardId) {
        zoneServerId_ = zoneServerId;
        accountInfo_.lastAccessShardId_ = shardId;
        migratingAt_ = -1;
        increateLoginCount();
    }

    void loggedInCommunityServer(sne::server::ServerId communityServerId) {
        communityServerId_ = communityServerId;
        increateLoginCount();
    }

    void migrationFailed() {
        migratingAt_ = -1;
    }

    uint8_t characterCreated(ShardId shardId, const FullCharacterInfo& characterInfo) {
        characterZoneMap_[characterInfo.objectId_] = characterInfo.zoneId_;
        return ++characterCounts_[shardId];
    }

    uint8_t characterDeleted(ShardId shardId, ObjectId characterId) {
        characterZoneMap_.erase(characterId);
        if (characterCounts_[shardId] > 0) {
            return --characterCounts_[shardId];
        }
        return 0;
    }

    void shardChanged(ShardId shardId) {
        if (isValidShardId(shardId)) {
            accountInfo_.lastAccessShardId_ = shardId;
        }
    }

    void userInfoChanged(const FullUserInfo& userInfo);

public:
    /// @return 로그인 서버, 존 서버 및 커뮤니티 서버에 접속해 있지 않을 경우 true
    bool shouldLogout() const {
        return (loginCount_ == 0) && (migratingAt_ == -1);
    }

    bool isMigrationExpired(sec_t effectivePeriod) const {
        if (migratingAt_ == -1) {
            return false;
        }

        return (getTime() - migratingAt_) > effectivePeriod;
    }

    uint32_t getSessionSeconds() const {
        return static_cast<uint32_t>(getTime() - loginAt_);
    }

    AccountId getAccountId() const {
        return accountInfo_.accountId_;
    }

	const AccountInfo& getAccountInfo() const {
		return accountInfo_;
	}

    ShardId getCurrentShardId() const {
        return accountInfo_.lastAccessShardId_;
    }

    ZoneId getLastAccessZoneId(ObjectId characterId) const {
        const CharacterZoneMap::const_iterator pos = characterZoneMap_.find(characterId);
        if (pos != characterZoneMap_.end()) {
            return (*pos).second;
        }
        return invalidZoneId;
    }

    bool isConnectedToZoneServer() const {
        return sne::server::isValid(zoneServerId_);
    }

    bool isConnectedToCommunityServer() const {
        return sne::server::isValid(communityServerId_);
    }

    sne::server::ServerId getZoneServerId() const {
        return zoneServerId_;
    }

    const CertificateMap& getCertificateMap() const {
        return certificateMap_;
    }

    const Certificate& getCertificate(ServerType serverType) const {
        assert(isClientConnectableServer(serverType));
        const Certificate* certificate = gideon::getCertificate(certificateMap_, serverType);
        if (certificate != nullptr) {
            return *certificate;
        }
        assert(false);
        static const Certificate nullCertificate;
        return nullCertificate;
    }

    const CharacterCountPerShardMap& getCharacterCounts() const {
        return characterCounts_;
    }

    LoginClientSessionCallback* getCallback() {
        return callback_;
    }

private:
    void increateLoginCount() {
        assert(loginCount_ < (std::numeric_limits<uint8_t>::max)());
        ++loginCount_;
    }

    void decreaseLoginCount() {
        if (loginCount_ > 0) {
            --loginCount_;
        }
    }

private:
    void issueCertificates() {
        issueCertificate(stLoginServer);
        issueCertificate(stZoneServer);
        issueCertificate(stCommunityServer);
    }

    void issueCertificate(ServerType serverType) {
        assert(isClientConnectableServer(serverType));
        Certificate& certificate = certificateMap_[serverType];
        certificate = Certificate::issue(serverType, accountInfo_.accountId_, esut::random());
    }

    ErrorCode checkCertificate(const Certificate& certificate, ServerType serverType) {
        if (certificate.serverType_ != serverType) {
            return ecLoginInvalidCertificate;
        }
        const Certificate* serverCertificate = gideon::getCertificate(certificateMap_, serverType);
        if (! serverCertificate) {
            return ecLoginInvalidCertificate;
        }
        return serverCertificate->check(certificate);
    }

private:
    LoginClientSessionCallback* callback_;
    CertificateMap certificateMap_;
	AccountInfo accountInfo_;
    CharacterCountPerShardMap characterCounts_;
    CharacterZoneMap characterZoneMap_;

    sne::server::ServerId zoneServerId_;
    sne::server::ServerId communityServerId_;
    sec_t loginAt_;
    sec_t migratingAt_;

    uint8_t loginCount_;

    bool migrationReserved_;
};

}} // namespace gideon { namespace loginserver {
