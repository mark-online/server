#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/ExpelReason.h>

namespace gideon {
struct Certificate;
struct FullUserInfo;
struct FullCharacterInfo;
} // namespace gideon {


namespace gideon { namespace loginserver {

/**
 * @class LoginClientSessionCallback
 */
class LoginClientSessionCallback : public boost::noncopyable
{
public:
    virtual ~LoginClientSessionCallback() {}

    /// 접속을 해제한다
    virtual void disconnectNow() = 0;

    /// 강제 퇴장 당했다
    virtual void expelled(ExpelReason reason) = 0;

    virtual void shardSelected(ErrorCode errorCode, const FullUserInfo& userInfo) = 0;

    virtual void characterCreated(ErrorCode errorCode,
        const FullCharacterInfo& characterInfo) = 0;

    virtual void characterDeleted(ErrorCode errorCode, ObjectId characterId) = 0;

    virtual void nicknameReserved(ErrorCode errorCode, const Nickname& nickname) = 0;
};

}} // namespace gideon { namespace loginserver {
