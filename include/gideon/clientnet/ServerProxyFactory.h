#ifndef CLIENTNET_SERVERPROXYFACTORY_H
#define CLIENTNET_SERVERPROXYFACTORY_H

#pragma once

#include <gideon/Common.h>
#include <boost/noncopyable.hpp>
#include <memory>

namespace gideon { namespace clientnet {

class ServerProxy;
class ServerProxyCallback;
struct ServerProxyConfig;

/**
 * @class ServerProxyFactory
 *
 * ServerProxy 팩토리
 */
class GIDEON_SERVER_API ServerProxyFactory : public boost::noncopyable
{
public:
    /// ServerProxy 인스턴스를 생성한다
    /// - 호출자는 메모리 소유권을 갖는다
    /// @pre confg.isValid() == true
    static std::unique_ptr<ServerProxy> create(
        ServerProxyCallback& callback, const ServerProxyConfig& config);
};

}} // namespace gideon { namespace clientnet {

#endif // CLIENTNET_SERVERPROXYFACTORY_H
