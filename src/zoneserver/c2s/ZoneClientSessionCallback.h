#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <sne/server/session/ClientId.h>

namespace sne { namespace base {
class Session;
}} // namespace sne { namespace base {

namespace gideon { namespace zoneserver {

/**
 * @class ZoneClientSessionCallback
 *
 * Server-side client session callback
 */
class ZoneClientSessionCallback : public boost::noncopyable
{
public:
    virtual ~ZoneClientSessionCallback() {}

    /// 로그인 요청에 대한 응답이 도착하였다
    virtual void loginResponsed(ErrorCode errorCode,
        const Certificate& reissuedCertificate) = 0;

    /// 접속을 해제한다(강제 퇴장)
    virtual void expelledFromServer() = 0;

    virtual void reserveMigration() = 0;

public:
    virtual sne::base::Session* getCurrentSession() = 0;

    virtual sne::server::ClientId getCurrentClientId() const = 0;

    /// 로그아웃 진행 중?
    virtual bool isLogoutReserved() const = 0;
};

}} // namespace gideon { namespace zoneserver {
