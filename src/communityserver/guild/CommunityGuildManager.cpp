#include "CommunityServerPCH.h"
#include "CommunityGuildManager.h"
#include "CommunityGuild.h"
#include "../user/CommunityUser.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <boost/algorithm/string/classification.hpp>
#include <sne/base/concurrent/TaskScheduler.h>
#pragma warning (push)
#pragma warning (disable: 4819)
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/searching/knuth_morris_pratt.hpp>
#pragma warning (pop)

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;


namespace gideon { namespace communityserver {

SNE_DEFINE_SINGLETON(CommunityGuildManager);

CommunityGuildManager::CommunityGuildManager()
{
}

CommunityGuildManager::~CommunityGuildManager()
{
    sne::base::Future::Ref task = guildSaveTask_.lock();
    if (task.get() != nullptr) {
        task->cancel();
    }
}


bool CommunityGuildManager::initialize(const GuildInfos& guildInfos, GuildId maxGuildId)
{
    generatorGuildId_ = maxGuildId;

    for (const GuildInfo& guildInfo : guildInfos) {
        auto guild = std::make_shared<CommunityGuild>();
        guild->initialize(guildInfo);
        communityGuildPtrMap_.emplace(guildInfo.guildId_, guild);

        for (const GuildApplicantInfoMap::value_type& value : guildInfo.guildApplicantInfoMap_) {
            guildApplicantMap_.emplace(value.first, guildInfo.guildId_);
        }

        guildNameMap_.emplace(boost::to_lower_copy(guildInfo.guildName_.ref()), guildInfo.guildId_);
    }

    if (TASK_SCHEDULER != nullptr) {
        const GameTime saveDelay = 1000 * 30;
        const GameTime saveInterval = 1000 * 30;
        guildSaveTask_ = TASK_SCHEDULER->schedule(*this, saveDelay, saveInterval);
    }

    CommunityGuildManager::instance(this);
    return true;
}


ErrorCode CommunityGuildManager::createGuild(
    CommunityUser& user, const GuildName& guildName, GuildMarkCode guildMarkCode)
{
    const ErrorCode errorCode = checkGuildName(guildName);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    const GuildId guildId = ++generatorGuildId_;
    const BaseGuildInfo guildInfo(guildId, guildName, guildMarkCode);
    auto guild = std::make_shared<CommunityGuild>();
    guild->initialize(guildInfo, user);

    {
        std::lock_guard<LockType> lock(lock_);

        communityGuildPtrMap_.emplace(guildInfo.guildId_, guild);

        guildNameMap_.emplace(boost::to_lower_copy(guildInfo.guildName_.ref()), guildInfo.guildId_);
    }

    return ecOk;
}


void CommunityGuildManager::removeGuild(GuildId guildId)
{
    {
        std::lock_guard<LockType> lock(lock_);

        communityGuildPtrMap_.erase(guildId);
    }
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveGuild(guildId);
    }
}


SearchGuildInfos CommunityGuildManager::searchGuilds(const GuildName& searchGuildName,
    GuildId selfGuildId) const
{
    SearchGuildInfos searchInfos;
    const auto loweredGuildName = boost::to_lower_copy(searchGuildName.ref());
    auto searcher = boost::algorithm::make_knuth_morris_pratt(loweredGuildName);

    {
        std::lock_guard<LockType> lock(lock_);

        for (const GuildNameMap::value_type& value : guildNameMap_) {
            const auto& guildName = value.first;
            const auto guildId = value.second;
            if (selfGuildId == guildId) {
                continue;
            }
            const auto result = searcher(guildName);
            if (result.first != guildName.end()) {
                searchInfos.push_back(getSearchGuildInfo_i(guildId));
            }
        }
    }
    
    return searchInfos;
}


SearchGuildInfo CommunityGuildManager::getSearchGuildInfo(GuildId guildId)  const
{
    std::lock_guard<LockType> lock(lock_);

    return getSearchGuildInfo_i(guildId);
}


bool CommunityGuildManager::isGuildApplicant(ObjectId playerId) const
{
    std::lock_guard<LockType> lock(lock_);

    return guildApplicantMap_.find(playerId) != guildApplicantMap_.end();
}


GuildId CommunityGuildManager::getGuildApplicantId(ObjectId playerId) const
{
    std::lock_guard<LockType> lock(lock_);

    GuildApplicantMap::const_iterator pos = guildApplicantMap_.find(playerId);
    if (pos != guildApplicantMap_.end()) {
        return (*pos).second;
    }
    return invalidGuildId;
}


void CommunityGuildManager::addGuildApplicant(ObjectId playerId, GuildId guildId)
{
    std::lock_guard<LockType> lock(lock_);

    guildApplicantMap_.emplace(playerId, guildId);
}


void CommunityGuildManager::removeGuildApplicant(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    guildApplicantMap_.erase(playerId);
}


const CommunityGuildPtr CommunityGuildManager::getGuild(GuildId guildId) const
{
    std::lock_guard<LockType> lock(lock_);

    return getGuild_i(guildId);
}


CommunityGuildPtr CommunityGuildManager::getGuild(GuildId guildId)
{
    std::lock_guard<LockType> lock(lock_);

    return getGuild_i(guildId);
}


gideon::ErrorCode CommunityGuildManager::checkGuildName(
    const GuildName& newGuildName) const
{
    using namespace boost;

    if (newGuildName.size() < minGuildNameLength) {
        return ecGuildNameTooShort;
    }
    if (!all(newGuildName, !is_space() && !is_cntrl() && !is_punct())) {
        return ecGuildInvalidName;
    }

    const auto loweredGuildName = boost::to_lower_copy(newGuildName.ref());

    {
        std::lock_guard<LockType> lock(lock_);

        for (const GuildNameMap::value_type& value : guildNameMap_) {
            const auto& guildName = value.first;
            if (guildName == loweredGuildName) {
                return ecGuildInvalidName; // duplicated
            }
        }
    }

    return ecOk;
}


const CommunityGuildPtr CommunityGuildManager::getGuild_i(GuildId guildId) const
{
    CommunityGuildPtrMap::const_iterator pos = communityGuildPtrMap_.find(guildId);

    if (pos != communityGuildPtrMap_.end()) {
        return (*pos).second;
    }
    const CommunityGuildPtr nullPtr;
    return nullPtr;
}


CommunityGuildPtr CommunityGuildManager::getGuild_i(GuildId guildId)
{
    CommunityGuildPtrMap::iterator pos = communityGuildPtrMap_.find(guildId);

    if (pos != communityGuildPtrMap_.end()) {
        return (*pos).second;
    }
    const CommunityGuildPtr nullPtr;
    return nullPtr;
}


SearchGuildInfo CommunityGuildManager::getSearchGuildInfo_i(GuildId guildId)  const
{
    SearchGuildInfo searchInfo;
    CommunityGuildPtr ptr = getGuild_i(guildId);
    if (ptr.get()) {
        const BaseGuildInfo& baseGuildInfo = ptr->getBaseGuildInfo();
        searchInfo.guildId_ = baseGuildInfo.guildId_;
        searchInfo.guildMarkCode_ = baseGuildInfo.guildMarkCode_;
        searchInfo.guildName_ = baseGuildInfo.guildName_;
        searchInfo.guildMasterName_ = ptr->getGuildMasterName();
    }
    return searchInfo;
}


void CommunityGuildManager::run()
{
    std::lock_guard<LockType> lock(lock_);

    for (CommunityGuildPtrMap::value_type value : communityGuildPtrMap_) {
        CommunityGuildPtr ptr = value.second;
        ptr->handleTimeout();
    }
}

}} // namespace gideon { namespace communityserver {
