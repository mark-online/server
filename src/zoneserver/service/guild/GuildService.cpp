#include "ZoneServerPCH.h"
#include "GuildService.h"

namespace gideon { namespace zoneserver {

SNE_DEFINE_SINGLETON(GuildService);

GuildService::GuildService()
{
}


GuildService::~GuildService() 
{
}


bool GuildService::initialize(const GuildInfos& guildInfos)
{
    for (const GuildInfo& guildInfo : guildInfos) {
        auto guild = std::make_shared<Guild>(guildInfo);
        guildMap_.emplace(guildInfo.guildId_, guild);
    }


    GuildService::instance(this);
    return true;
}


void GuildService::createGuild(const BaseGuildInfo& guildInfo)
{    
    std::lock_guard<LockType> lock(lock_);

    if (isExist(guildInfo.guildId_)) {
        assert(false);
        return;
    }

    auto guild = std::make_shared<Guild>(guildInfo);
    guildMap_.emplace(guildInfo.guildId_, guild);
}


void GuildService::removeGuild(GuildId guildId)
{
    std::lock_guard<LockType> lock(lock_);

    guildMap_.erase(guildId);
}


GuildPtr GuildService::getGuildByGuildId(GuildId guildId)
{
    std::lock_guard<LockType> lock(lock_);

    GuildMap::const_iterator pos = guildMap_.find(guildId);
    if (pos != guildMap_.end()) {
        return (*pos).second;
    }
    return GuildPtr();
}




bool GuildService::isExist(GuildId guildId) const
{
    std::lock_guard<LockType> lock(lock_);

    GuildMap::const_iterator pos = guildMap_.find(guildId);
    if (pos != guildMap_.end()) {
        return true;
    }
    return false;
}

}} // namespace gideon { namespace zoneserver {
