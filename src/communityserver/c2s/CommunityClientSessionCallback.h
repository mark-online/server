#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <sne/server/session/ClientId.h>

namespace sne { namespace base {
class Session;
}} // namespace sne { namespace base {

namespace gideon { namespace communityserver {

/**
 * @class CommunityClientSessionCallback
 *
 * Server-side client session callback
 */
class CommunityClientSessionCallback : public boost::noncopyable
{
public:
    virtual ~CommunityClientSessionCallback() {}

    /// 로그인 요청에 대한 응답이 도착하였다
    virtual void loginResponsed(ErrorCode errorCode,
        const Certificate& reissuedCertificate) = 0;

    /// 접속을 해제한다(강제 퇴장)
    virtual void expelledFromServer() = 0;

public:
    virtual sne::base::Session* getCurrentSession() = 0;

    virtual sne::server::ClientId getCurrentClientId() const = 0;
};

}} // namespace gideon { namespace communityserver {
