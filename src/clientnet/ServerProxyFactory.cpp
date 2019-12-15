#include "ClientNetPCH.h"
#include <gideon/clientnet/ServerProxyFactory.h>
#include "ServerProxyImpl.h"

namespace gideon { namespace clientnet {

std::unique_ptr<ServerProxy> ServerProxyFactory::create(
    ServerProxyCallback& callback, const ServerProxyConfig& config)
{
    assert(config.isValid());

    return std::make_unique<ServerProxyImpl>(callback, config);
}

}} // namespace gideon { namespace clientnet {
