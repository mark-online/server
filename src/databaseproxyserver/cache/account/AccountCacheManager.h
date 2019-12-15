#pragma once

#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/container/Containers.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace sne { namespace base {
class IoContext;
}} // namespace sne { namespace base {

namespace gideon { namespace databaseproxyserver {

class AccountCache;

namespace
{
class AccountCachePool;
class CacheCollector;
} // namespace

/**
 * @class AccountCacheManager
 *
 * AccountCache 관리자
 */
class AccountCacheManager : public boost::noncopyable
{
    typedef sne::core::HashMap<AccountId, AccountCache*> AccountCacheMap;
    typedef sne::core::HashMap<ObjectId, AccountId> CharacterIdMap;

    SNE_DECLARE_SINGLETON(AccountCacheManager);

public:
    AccountCacheManager(sne::base::IoContext& ioContext);
    ~AccountCacheManager();

    ErrorCode initialize();

    AccountCache* loadCache(AccountId accountId);

    AccountId getAccountId(ObjectId characterId) const;
	
    void purgeCaches();

	void createAccountCache(AccountId accountId);
public:
    AccountCache* getCache(AccountId accountId);
    const AccountCache* getCache(AccountId accountId) const;

private:
    ErrorCode preloadCache();
    bool initCacheCollector();

    ErrorCode addCache(AccountId accountId);
    ErrorCode removeCache(AccountId accountId);

    AccountCache* getCache_i(AccountId accountId);
    const AccountCache* getCache_i(AccountId accountId) const;

    bool isExists(AccountId accountId) const {
        return cacheMap_.find(accountId) != cacheMap_.end();
    }

private:
    sne::base::IoContext& ioContext_;

    std::unique_ptr<AccountCachePool> cachePool_;
    AccountCacheMap cacheMap_;
    CharacterIdMap characterIdMap_;
    uint16_t oldCacheMinAccessPeriod_;

    std::unique_ptr<CacheCollector> cacheCollector_;

    mutable std::mutex lock_;
};

}} // namespace gideon { namespace databaseproxyserver {

#define ACCOUNTCACHE_MANAGER \
    gideon::databaseproxyserver::AccountCacheManager::instance()
