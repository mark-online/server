#pragma once

#include "../../zoneserver_export.h"
#include "Guild.h"
#include <sne/core/utility/Singleton.h>


namespace gideon { namespace zoneserver { namespace go {
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {


/**
 * @class GuildService
 *
 * 길드 서비스
 */
class ZoneServer_Export GuildService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(GuildService);

    typedef std::mutex LockType;
    typedef sne::core::HashMap<GuildId, GuildPtr> GuildMap;

public:
    GuildService();
    ~GuildService();

    bool initialize(const GuildInfos& guildInfos);
   
public:
    // 커뮤니티 서버 호출용
    void createGuild(const BaseGuildInfo& guildInfo);
	void removeGuild(GuildId guildId);
    
public:
	GuildPtr getGuildByGuildId(GuildId guildId);

private:
    bool isExist(GuildId guildId) const;


private:
    mutable LockType lock_;

    GuildMap guildMap_;
};


}} // namespace gideon { namespace zoneserver {


#define GUILD_SERVICE gideon::zoneserver::GuildService::instance()
