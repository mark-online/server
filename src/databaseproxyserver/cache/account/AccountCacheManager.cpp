#include "DatabaseProxyServerPCH.h"
#include "AccountCacheManager.h"
#include "detail/AccountCacheImpl.h"
#include <gideon/serverbase/database/GameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/utility/Profiler.h>
#include <sne/server/common/Property.h>
#include <sne/base/io_context/IoContext.h>
#include <sne/base/io_context/Timer.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/base/memory/ObjectPool.h>
#include <sne/core/utility/CppUtils.h>

namespace gideon { namespace databaseproxyserver {

namespace
{

/**
 * @class CacheCollector
 */
class CacheCollector : private sne::base::TimerCallback
{
public:
    CacheCollector() {}

    bool start(uint16_t oldCacheCheckPeriod,
        sne::base::IoContext& ioContext) {
        if (! timer_.get()) {
            timer_ = ioContext.createTimer(*this);
        }

        if (! timer_->repeat(oldCacheCheckPeriod * 1000)) {
            SNE_LOG_ERROR("CacheCollector::initialize(%u) FAILED",
                oldCacheCheckPeriod * 1000);
            return false;
        }
        return true;
    }

    void stop() {
        timer_->cancel();
    }

private:
    // = sne::base::TimerCallback overriding
    virtual void timedOut(const void* /*act*/) {
        ACCOUNTCACHE_MANAGER->purgeCaches();
    }

private:
    std::unique_ptr<sne::base::Timer> timer_;
};


/**
* @class AccountCacheAllocator
*/
class AccountCacheAllocator : public boost::noncopyable
{
public:
    AccountCacheAllocator() {}

    AccountCache* malloc() {
        return new AccountCacheImpl;
    }

    void free(AccountCache* cache) {
        boost::checked_delete(cache);
    }
};

/**
 * @class AccountCachePool
 */
class AccountCachePool :
    public sne::base::ObjectPool<AccountCache, AccountCacheAllocator>
{
    const static bool growable = true;
    const static size_t defaultPoolSize = 100;
public:
    AccountCachePool(AccountCacheAllocator& allocator, size_t poolSize = defaultPoolSize) :
        sne::base::ObjectPool<AccountCache, AccountCacheAllocator>(
            poolSize, allocator, growable) {}
};


inline AccountIds getRecentLoginUserList(uint16_t maxLoadCount)
{
    AccountIds userList;
    sne::database::Guard<serverbase::GameDatabase> db(SNE_DATABASE_MANAGER);
    (void)db->getRecentLoginUserList(userList, maxLoadCount);
    return userList;
}

} // namespace

// = AccountCacheManager

SNE_DEFINE_SINGLETON(AccountCacheManager);

AccountCacheManager::AccountCacheManager(sne::base::IoContext& ioContext) :
    ioContext_(ioContext),
    oldCacheMinAccessPeriod_(0)
{
    static AccountCacheAllocator allocator;

    cachePool_ = std::make_unique<AccountCachePool>(allocator);
    cachePool_->initialize();
}


AccountCacheManager::~AccountCacheManager()
{
    cacheCollector_->stop();
}


ErrorCode AccountCacheManager::initialize()
{
    const ErrorCode errorCode = preloadCache();
    if (isFailed(errorCode)) {
        return errorCode;
    }

    if (! initCacheCollector()) {
        return ecServerInternalError;
    }

    return ecOk;
}


AccountCache* AccountCacheManager::loadCache(AccountId accountId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    AccountCache* cache = getCache(accountId);
    if (cache != nullptr) {
        return cache;
    }

    if (isFailed(addCache(accountId))) {
        return 0;
    }

    return getCache(accountId);
}


AccountId AccountCacheManager::getAccountId(ObjectId characterId) const
{
    std::unique_lock<std::mutex> lock(lock_);

    CharacterIdMap::const_iterator pos = characterIdMap_.find(characterId);
    if (pos != characterIdMap_.end()) {
        return (*pos).second;
    }
    return invalidAccountId;
}


void AccountCacheManager::createAccountCache(AccountId accountId)
{
	addCache(accountId);
}


void AccountCacheManager::purgeCaches()
{
    sne::server::Profiler profiler(__FUNCTION__);

    AccountIds oldCaches;
    {
        std::unique_lock<std::mutex> lock(lock_);

        oldCaches.reserve(cacheMap_.size());

        AccountCacheMap::iterator pos = cacheMap_.begin();
        const AccountCacheMap::iterator end = cacheMap_.end();
        for (; pos != end; ++pos) {
            const AccountId accountId = (*pos).first;
            const AccountCache* cache = (*pos).second;
            if (cache->shouldCollect(oldCacheMinAccessPeriod_)) {
                oldCaches.push_back(accountId);
            }
        }
    }

    {
        AccountIds::const_iterator pos = oldCaches.begin();
        const AccountIds::const_iterator end = oldCaches.end();
        for (; pos != end; ++pos) {
            const AccountId accountId = *pos;
            (void)removeCache(accountId);
        }
    }
}


AccountCache* AccountCacheManager::getCache(AccountId accountId)
{
    std::unique_lock<std::mutex> lock(lock_);

    return getCache_i(accountId);
}


const AccountCache* AccountCacheManager::getCache(AccountId accountId) const
{
    std::unique_lock<std::mutex> lock(lock_);

    return getCache_i(accountId);
}


ErrorCode AccountCacheManager::preloadCache()
{
    sne::server::Profiler profiler(__FUNCTION__);

    const uint16_t maxLoadCount = SNE_PROPERTIES::getProperty<uint16_t>(
        "database_proxy_server.cache.account_load_count");

    const AccountIds userList = getRecentLoginUserList(maxLoadCount);
    AccountIds::const_iterator pos = userList.begin();
    const AccountIds::const_iterator end = userList.end();
    for (; pos != end; ++pos) {
        const AccountId accountId = *pos;
        (void)addCache(accountId);
    }

    return ecOk;
}


bool AccountCacheManager::initCacheCollector()
{
    const uint16_t oldCacheCheckPeriod = SNE_PROPERTIES::getProperty<uint16_t>(
            "database_proxy_server.cache.old_cache_check_period");
    SNE_ASSERT(oldCacheCheckPeriod > 0);

    oldCacheMinAccessPeriod_ = SNE_PROPERTIES::getProperty<uint16_t>(
        "database_proxy_server.cache.old_cache_min_access_period");
    SNE_ASSERT(oldCacheMinAccessPeriod_ > 0);

    cacheCollector_ = std::make_unique<CacheCollector>();
    return cacheCollector_->start(oldCacheCheckPeriod, ioContext_);
}


ErrorCode AccountCacheManager::addCache(AccountId accountId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    AccountCache* newCache = nullptr;
    {
        std::unique_lock<std::mutex> lock(lock_);

        newCache = cachePool_->acquire();
        if (! newCache) {
            SNE_LOG_ERROR("Too many account caches(%u).",
                cacheMap_.size());
            return ecServerInternalError;
        }
    }

    const ErrorCode errorCode = newCache->initialize(accountId);

    {
        std::unique_lock<std::mutex> lock(lock_);

        if (isFailed(errorCode)) {
            cachePool_->release(newCache);
            SNE_LOG_ERROR("Failed to add AccountCache(A%" PRIu64 ")[E%d].",
                accountId, errorCode);
            return errorCode;
        }

        cacheMap_.emplace(accountId, newCache);
        ObjectIdSet characterIds = newCache->getCharacterIds();
        ObjectIdSet::const_iterator pos = characterIds.begin();
        ObjectIdSet::const_iterator end = characterIds.end();
        for (; pos != end; ++pos) {
            characterIdMap_.emplace((*pos), accountId);
        }
        
        SNE_LOG_INFO("AccountCache(A%" PRIu64 ") added. total=%u.",
            accountId, cacheMap_.size());
    }

    return ecOk;
}


ErrorCode AccountCacheManager::removeCache(AccountId accountId)
{
    AccountCache* cache = nullptr;
    {
        std::unique_lock<std::mutex> lock(lock_);

        cache = getCache_i(accountId);
        if (! cache) {
            assert(false);
            return ecOk;
        }

        ObjectIdSet characterIds = cache->getCharacterIds();
        ObjectIdSet::const_iterator pos = characterIds.begin();
        ObjectIdSet::const_iterator end = characterIds.end();
        for (; pos != end; ++pos) {
            characterIdMap_.erase((*pos));
        }
    }

    cache->finalize();

    {
        std::unique_lock<std::mutex> lock(lock_);

        cacheMap_.erase(accountId);
        cachePool_->release(cache);

        SNE_LOG_INFO("AccountCache(A%" PRIu64 ") removed. total=%u.",
            accountId, cacheMap_.size());
    }
    return ecOk;
}


AccountCache* AccountCacheManager::getCache_i(AccountId accountId)
{
    AccountCache* cache = sne::core::search_map(cacheMap_, accountId, 0);
    if (cache != nullptr) {
        cache->touched();
    }
    return cache;
}


const AccountCache* AccountCacheManager::getCache_i(AccountId accountId) const
{
    const AccountCache* cache = sne::core::search_map(cacheMap_, accountId, 0);
    if (cache != nullptr) {
        const_cast<AccountCache*>(cache)->touched();
    }
    return cache;
}

}} // namespace gideon { namespace databaseproxyserver {
