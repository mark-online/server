#include "CommunityServerPCH.h"
#include "CommunityServerSideProxyManager.h"
#include "CommunityServerSideProxy.h"
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace communityserver {

namespace
{

inline CommunityServerSideProxy* toProxy(sne::server::ServerSideProxy* proxy)
{
    return static_cast<CommunityServerSideProxy*>(proxy);
}

} // namespace


CommunityServerSideProxyManager::CommunityServerSideProxyManager() :
    sne::server::ServerSideProxyManager()
{
}


void CommunityServerSideProxyManager::partyMemberJoined(PartyId partyId, ObjectId objectId)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_evPartyMemberJoined(partyId, objectId);
        }
    }
}


void CommunityServerSideProxyManager::partyMemberCharacterClassChanged(PartyId partyId, ObjectId objectId, CharacterClass cc)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_evPartyMemberCharacterClassChanged(partyId, objectId, cc);
        }
    }
}


void CommunityServerSideProxyManager::partyTypeChanged(PartyId partyId, PartyType partyType)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_evPartyTypeChanged(partyId, partyType);
        }
    }
}


void CommunityServerSideProxyManager::partyMemberLeft(PartyId partyId, ObjectId playerId)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_evPartyMemberLeft(partyId, playerId);
        }
    }
}


void CommunityServerSideProxyManager::partyMemberRejoined(sne::server::ServerId proxyId,
    PartyId partyId, AccountId accountId, ObjectId memberId)
{
    CommunityServerSideProxy* proxy = getCommunityServerSideProxy(proxyId);
    if (proxy) {
        proxy->z2m_evPartyMemberRejoined(partyId, accountId, memberId);
    }
}


void CommunityServerSideProxyManager::guildCreated(const GuildMemberInfo& guildMemberInfo, const BaseGuildInfo& guildInfo)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_createGuild(guildMemberInfo, guildInfo);
        }
    }
}


void CommunityServerSideProxyManager::guildMemberAdded(GuildId guildId, const GuildMemberInfo& guildMemberInfo)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_addGuildMember(guildId, guildMemberInfo);
        }
    }
}


void CommunityServerSideProxyManager::guildMemberRemoved(GuildId guildId, ObjectId characterId)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_removeGuildMember(guildId, characterId);
        }
    }
}


void CommunityServerSideProxyManager::guildMemberPositionChanged(GuildId guildId, ObjectId characterId, GuildMemberPosition position)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_changeGuildMemberPosition(guildId, characterId, position);
        }
    }
}


void CommunityServerSideProxyManager::guildRelationshipAdded(GuildId guildId, const GuildRelationshipInfo& info)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_addGuildRelationship(guildId, info);
        }
    }
}


void CommunityServerSideProxyManager::guildRelationshipRemoved(GuildId guildId, GuildId targetGuildId)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_removeGuildRelationship(guildId, targetGuildId);
        }
    }
}


void CommunityServerSideProxyManager::guildAddibleDayExpState(GuildId guildId, bool canExpAddible)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_updateGuildExpAddibleState(guildId, canExpAddible);
        }
    }
}


void CommunityServerSideProxyManager::guildLevelUpdated(GuildId guildId, GuildLevel guildLevel)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_guildLevelUpdated(guildId, guildLevel);
        }
    }
}


void CommunityServerSideProxyManager::guildSkillActivated(ErrorCode errorCode, GuildId guildId, 
    ObjectId playerId, SkillCode skillCode)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_onActivateGuildSkill(errorCode, guildId, playerId, skillCode);
        }
    }
}


void CommunityServerSideProxyManager::guildSkillsDeactivated(ErrorCode errorCode, GuildId guildId, 
    ObjectId playerId)
{
    Proxies proxies = getCopiedProxies();
    for (Proxies::value_type value : proxies) {
        CommunityServerSideProxy* proxy = toProxy(value.second);
        if (proxy) {
            proxy->z2m_onDeactivateGuildSkills(errorCode, guildId, playerId);
        }
    }
}


void CommunityServerSideProxyManager::recallRequested(sne::server::ServerId proxyId,
    const Nickname& callName, const Nickname& calleeName, const WorldPosition& position)
{
    CommunityServerSideProxy* proxy = getCommunityServerSideProxy(proxyId);
    if (proxy) {
        proxy->z2m_evRecallRequested(callName, calleeName, position);
    }
}


void CommunityServerSideProxyManager::buddyAdded(sne::server::ServerId proxyId, ObjectId playerId,
    size_t buddyCount)
{
    CommunityServerSideProxy* proxy = getCommunityServerSideProxy(proxyId);
    if (proxy) {
        proxy->z2m_onBuddyAdded(playerId, static_cast<uint32_t>(buddyCount));
    }
}


CommunityServerSideProxy* CommunityServerSideProxyManager::getCommunityServerSideProxy(
    sne::server::ServerId proxyId)
{
    Proxies proxies = getCopiedProxies();
    Proxies::iterator pos = proxies.find(proxyId);
    if (pos != proxies.end()) {
        return toProxy((*pos).second);
    }
    return nullptr;
}


}} // namespace gideon { namespace communityserver {
