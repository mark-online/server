#pragma once

#include "../communityserver_export.h"
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <sne/server/common/SocketAddress.h>
#include <sne/server/s2s/ServerSideProxyManager.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/core/container/Containers.h>

namespace gideon { namespace communityserver {

class CommunityServerSideProxy;

/**
 * @class CommunityServerSideProxyManager
 *
 * S2S(Zone) Server 관리자
 * - 목록 유지
 * - 이벤트 전파(broadcast)
 * - Composite Pattern
 */
class CommunityServer_Export CommunityServerSideProxyManager :
    public sne::server::ServerSideProxyManager
{
public:
    CommunityServerSideProxyManager();

    void partyMemberJoined(PartyId partyId, ObjectId objectId);
    void partyMemberLeft(PartyId partyId, ObjectId objectId);
    void partyMemberCharacterClassChanged(PartyId partyId, ObjectId objectId, CharacterClass cc);
    void partyTypeChanged(PartyId partyId, PartyType partyType);

    void partyMemberRejoined(sne::server::ServerId proxyId,
        PartyId partyId, AccountId accountId, ObjectId memberId);

    void guildCreated(const GuildMemberInfo& guildMemberInfo, const BaseGuildInfo& guildInfo);
    void guildMemberAdded(GuildId guildId, const GuildMemberInfo& guildMemberInfo);
    void guildMemberRemoved(GuildId guildId, ObjectId characterId);
	void guildMemberPositionChanged(GuildId guildId, ObjectId characterId, GuildMemberPosition position);

    void guildRelationshipAdded(GuildId guildId, const GuildRelationshipInfo& info);
    void guildRelationshipRemoved(GuildId guildId, GuildId targetGuildId);
    
    void guildAddibleDayExpState(GuildId guildId, bool canExpAddible);
    void guildLevelUpdated(GuildId guildId, GuildLevel guildLevel);
    void guildSkillActivated(ErrorCode errorCode, GuildId guildId, 
        ObjectId playerId, SkillCode skillCode);
    void guildSkillsDeactivated(ErrorCode errorCode, GuildId guildId, 
        ObjectId playerId);

	void recallRequested(sne::server::ServerId proxyId, const Nickname& callName,
		const Nickname& calleeName, const WorldPosition& position);

    void buddyAdded(sne::server::ServerId proxyId, ObjectId playerId, size_t buddyCount);

private:
    virtual void proxyRemoved(sne::server::ServerId /*proxyId*/) {}
    virtual void connected() {}
    
    CommunityServerSideProxy* getCommunityServerSideProxy(sne::server::ServerId proxyId);

};

}} // namespace gideon { namespace communityserver {

#define COMMUNITYSERVERSIDEPROXY_MANAGER \
    static_cast<gideon::communityserver::CommunityServerSideProxyManager*>( \
        SERVERSIDEPROXY_MANAGER)
