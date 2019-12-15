#include "LoginServerPCH.h"
#include "LoginServerSideProxyManager.h"
#include "LoginServerSideProxy.h"
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace loginserver {

namespace
{

inline LoginServerSideProxy* toProxy(sne::server::ServerSideProxy* proxy)
{
    return static_cast<LoginServerSideProxy*>(proxy);
}

} // namespace

LoginServerSideProxyManager::LoginServerSideProxyManager() :
    sne::server::ServerSideProxyManager()
{
}


void LoginServerSideProxyManager::expelUser(AccountId accountId, ExpelReason reason,
    sne::server::ServerId proxyId)
{
    LoginServerSideProxy* proxy = toProxy(getProxy(proxyId));
    if (! proxy) {
        return;
    }

    proxy->expelUser(accountId, reason);
}


void LoginServerSideProxyManager::expelUser(AccountId accountId, ExpelReason reason)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type& value : proxies) {
        LoginServerSideProxy* proxy = toProxy(value.second);
        if (proxy != nullptr) {
            proxy->expelUser(accountId, reason);
        }
        else {
            assert(false);
        }
    }
}


bool LoginServerSideProxyManager::requestFullUserInfo(sne::server::ServerId proxyId,
    AccountId accountId)
{
    LoginServerSideProxy* proxy = toProxy(getProxy(proxyId));
    if (! proxy) {
        return false;
    }

    proxy->z2l_getFullUserInfo(accountId);
    return true;
}


bool LoginServerSideProxyManager::requestCreateCharacter(sne::server::ServerId proxyId,
    const CreateCharacterInfo& createCharacterInfo)
{
    LoginServerSideProxy* proxy = toProxy(getProxy(proxyId));
    if (! proxy) {
        return false;
    }

    proxy->z2l_createCharacter(createCharacterInfo);
    return true;
}


bool LoginServerSideProxyManager::requestDeleteCharacter(sne::server::ServerId proxyId,
    AccountId accountId, ObjectId characterId)
{
    LoginServerSideProxy* proxy = toProxy(getProxy(proxyId));
    if (! proxy) {
        return false;
    }

    proxy->z2l_deleteCharacter(accountId, characterId);
    return true;
}


bool LoginServerSideProxyManager::requestReserveNickname(sne::server::ServerId proxyId, AccountId accountId,
    const Nickname& nickname)
{
    LoginServerSideProxy* proxy = toProxy(getProxy(proxyId));
    if (! proxy) {
        return false;
    }

    proxy->z2l_checkDuplicateNickname(accountId, nickname);
    return true;
}


void LoginServerSideProxyManager::zoneServerConnected(ShardId shardId, ZoneId zoneId)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type& value : proxies) {
        LoginServerSideProxy* proxy = toProxy(value.second);
        if (proxy != nullptr) {
            proxy->zoneServerConnected(shardId, zoneId);
        }
        else {
            assert(false);
        }
    }
}


void LoginServerSideProxyManager::zoneServerDisconnected(ShardId shardId, ZoneId zoneId)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type& value : proxies) {
        LoginServerSideProxy* proxy = toProxy(value.second);
        if (proxy != nullptr) {
            proxy->zoneServerDisconnected(shardId, zoneId);
        }
        else {
            assert(false);
        }
    }
}

}} // namespace gideon { namespace loginserver {
