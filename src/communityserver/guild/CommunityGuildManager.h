#pragma once

#include "../communityserver_export.h"
#include "CommunityGuild.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/utility/Singleton.h>
#include <sne/base/concurrent/Runnable.h>
#include <sne/base/concurrent/Future.h>
#include <atomic>

namespace gideon { namespace communityserver {

class CommunityUser;

/***
 * @class CommunityGuildManager
 ***/
class CommunityServer_Export CommunityGuildManager  : private sne::base::Runnable
{
    typedef std::mutex LockType;
    typedef sne::core::HashMap<std::wstring, GuildId> GuildNameMap;
    typedef sne::core::HashMap<ObjectId, GuildId> GuildApplicantMap;

    SNE_DECLARE_SINGLETON(CommunityGuildManager);
public:
    CommunityGuildManager();
    ~CommunityGuildManager();

    bool initialize(const GuildInfos& guildInfos, GuildId maxGuildId);

    ErrorCode createGuild(CommunityUser& user,
        const GuildName& guildName, GuildMarkCode guildMarkCode);

    void removeGuild(GuildId guildId);
    
    const CommunityGuildPtr getGuild(GuildId guildId) const;

    CommunityGuildPtr getGuild(GuildId guildId);

    SearchGuildInfos searchGuilds(const GuildName& searchGuildName, GuildId selfGuildId) const;

    SearchGuildInfo getSearchGuildInfo(GuildId guildId) const;

    bool isGuildApplicant(ObjectId playerId) const; 
    GuildId getGuildApplicantId(ObjectId playerId) const;

    void addGuildApplicant(ObjectId playerId, GuildId guildId); 
    void removeGuildApplicant(ObjectId playerId);

private:
    ErrorCode checkGuildName(const GuildName& newGuildName) const;

private:
    const CommunityGuildPtr getGuild_i(GuildId guildId) const;

    CommunityGuildPtr getGuild_i(GuildId guildId);
    SearchGuildInfo getSearchGuildInfo_i(GuildId guildId)  const;

private: // = sne::base::Runnable overriding
    virtual void run();

private:
    typedef sne::core::HashMap<GuildId, CommunityGuildPtr> CommunityGuildPtrMap;
    mutable LockType lock_;

    std::atomic<GuildId> generatorGuildId_;
    CommunityGuildPtrMap communityGuildPtrMap_;
    GuildNameMap guildNameMap_;
    GuildApplicantMap guildApplicantMap_;

    sne::base::Future::WeakRef guildSaveTask_;
};

}} // namespace gideon { namespace communityserver {


#define COMMUNITYGUILD_MANAGER gideon::communityserver::CommunityGuildManager::instance()
