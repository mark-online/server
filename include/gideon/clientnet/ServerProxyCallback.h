#ifndef CLIENTNET_SERVERPROXYCALLBACK_H
#define CLIENTNET_SERVERPROXYCALLBACK_H

#pragma once

#include <sne/Common.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace clientnet {

/**
 * @struct LoginResult
 * 로그인 서버 로그인 요청에 대한 결과
 */
struct LoginResult
{
    ErrorCode errorCode_;

	AccountInfo accountInfo_;
    CommunityServerInfo communityServerInfo_;
    CertificateMap certificateMap_;

    explicit LoginResult(ErrorCode errorCode = ecOk) :
        errorCode_(errorCode) {}

    void reset() {
        errorCode_ = ecOk;
        accountInfo_.reset();
        communityServerInfo_.reset();
        certificateMap_.clear();
    }

	bool isValid() const {
		return accountInfo_.isValid();
	}

    bool isReponsed() const {
        return isFailed(errorCode_) || accountInfo_.isValid();
    }
};


/**
 * @struct QueryShardInfoResult
 * Shard 정보 요청에 대한 결과
 */
struct QueryShardInfoResult
{
    bool isResponsed_;
    ErrorCode errorCode_;
	ShardInfoList shardInfos_;
    ZoneMapCodeMap zoneMapCodeMap_;
    CharacterCountPerShardMap characterCounts_;

    explicit QueryShardInfoResult(ErrorCode errorCode = ecOk) :
        isResponsed_(false),
        errorCode_(errorCode) {}

    void reset() {
        isResponsed_ = false;
        errorCode_ = ecOk;
        shardInfos_.clear();
        zoneMapCodeMap_.clear();
        characterCounts_.clear();
    }

    bool isReponsed() const {
        return isResponsed_;
    }
};


/**
 * @struct SelectShardResult
 * Shard 선택 요청에 대한 결과
 */
struct SelectShardResult
{
    bool isResponsed_;
    ErrorCode errorCode_;
    FullUserInfo fullUserInfo_;

    explicit SelectShardResult(ErrorCode errorCode = ecOk) :
        isResponsed_(false),
        errorCode_(errorCode) {}

    void reset() {
        isResponsed_ = false;
        errorCode_ = ecOk;
        fullUserInfo_.reset();
    }

    bool isReponsed() const {
        return isResponsed_;
    }
};


/**
 * @struct ReserveNicknameResult
 * 닉네임 예약 요청에 대한 결과
 */
struct ReserveNicknameResult
{
    bool isResponsed_;
    ErrorCode errorCode_;
    Nickname nickname_;

    explicit ReserveNicknameResult(ErrorCode errorCode = ecOk) :
        isResponsed_(false),
        errorCode_(errorCode) {}

    void reset() {
        isResponsed_ = false;
        errorCode_ = ecOk;
        nickname_ = L"";
    }

    bool isReponsed() const {
        return isResponsed_;
    }
};


/**
 * @struct CreateCharacterResult
 * 캐릭터 생성 요청에 대한 결과
 */
struct CreateCharacterResult
{
    bool isResponsed_;
    ErrorCode errorCode_;
    FullCharacterInfo fullCharacterInfo_;

    explicit CreateCharacterResult(ErrorCode errorCode = ecOk) :
        isResponsed_(false),
        errorCode_(errorCode) {}

    void reset() {
        isResponsed_ = false;
        errorCode_ = ecOk;
        fullCharacterInfo_.reset();
    }

    bool isReponsed() const {
        return isResponsed_;
    }
};


/**
 * @struct DeleteCharacterResult
 * 캐릭터 삭제 요청에 대한 결과
 */
struct DeleteCharacterResult
{
    bool isResponsed_;
    ErrorCode errorCode_;
    ObjectId characterId_;

    explicit DeleteCharacterResult(ErrorCode errorCode = ecOk) :
        isResponsed_(false),
        errorCode_(errorCode),
        characterId_(invalidObjectId) {}

    void reset() {
        isResponsed_ = false;
        errorCode_ = ecOk;
        characterId_ = invalidObjectId;
    }

    bool isReponsed() const {
        return isResponsed_;
    }
};


/**
 * @struct EnterShardResult
 * Shard 입장 요청에 대한 결과
 */
struct EnterShardResult
{
    bool isResponsed_;
    ErrorCode errorCode_;
	ZoneServerInfo zoneServerInfo_;

    explicit EnterShardResult(ErrorCode errorCode = ecOk) :
        isResponsed_(false),
        errorCode_(errorCode) {}

    void reset() {
        isResponsed_ = false;
        errorCode_ = ecOk;
        zoneServerInfo_.reset();
    }

    bool isReponsed() const {
        return isResponsed_;
    }
};


/**
 * @class ServerProxyCallback
 *
 * ServerProxy event callback
 */
class ServerProxyCallback : public boost::noncopyable
{
public:
    virtual ~ServerProxyCallback() {}

    /// 강제 퇴장 당했다
    virtual void onExpelled(ExpelReason reason) = 0;

    /// 커뮤니티 서버와의 연결이 끊겼다.
    virtual void onCommunityServerDisconnected() = 0;

    /// 존 서버와의 연결이 끊겼다.
    virtual void onZoneServerDisconnected() = 0;
};

}} // namespace gideon { namespace clientnet {

#endif // CLIENTNET_SERVERPROXYCALLBACK_H
