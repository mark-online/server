#include "CommunityServerPCH.h"
#include "WorldMapChannelManager.h"
#include "WorldMapChannel.h"
#include "../s2s/CommunityServerSideProxyManager.h"
#include "../party/CommunityPartyManager.h"
#include "../user/CommunityUserManager.h"
#include "../user/CommunityUser.h"
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/utility/CppUtils.h>

namespace gideon { namespace communityserver {

SNE_DEFINE_SINGLETON(WorldMapChannelManager);

WorldMapChannelManager::WorldMapChannelManager() :
    channelIndex_(0)
{
}


WorldMapChannelManager::~WorldMapChannelManager()
{
}


void WorldMapChannelManager::initialize()
{
    addShardChannel();
}


void WorldMapChannelManager::noticeToShard(const ChatMessage& message)
{
    std::lock_guard<LockType> lock(lock_);

    shardChannel_->notice(message);
}


void WorldMapChannelManager::worldMapOpened(ZoneId zoneId, MapCode worldMapCode, ObjectId mapId)
{
    if (! isValidZoneId(zoneId)) {
        assert(false);
        return;
    }
    if (! isValidMapCode(worldMapCode)) {
        assert(false);
        return;
    }

    const ChannelId channelId = generateChannelId(zoneId);

    size_t channelCount = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        WorldMapChannel::Ref channel(
            new WorldMapChannel(channelId, zoneId, worldMapCode, mapId));
        channel->initialize();

        channelMap_.emplace(channelId, channel);
        channelMapByZone_[zoneId].emplace(mapId, channelId);

        channelCount = channelMap_.size();
    }

    SNE_LOG_INFO("WorldMapChannel(I%u,Z%u,M%u,W%" PRIu64 ") opened. total = %u.",
        channelId, zoneId, worldMapCode, mapId, channelCount);
}


void WorldMapChannelManager::worldMapClosed(ZoneId zoneId, MapCode worldMapCode, ObjectId mapId)
{
    if (! isValidZoneId(zoneId)) {
        assert(false);
        return;
    }
    if (! isValidMapCode(worldMapCode)) {
        assert(false);
        return;
    }

    ChannelId channelId = invalidChannelId;
    size_t channelCount = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        // TODO: 최적화
        WorldMapChannelMap::iterator pos = channelMap_.begin();
        const WorldMapChannelMap::iterator end = channelMap_.end();
        for (; pos != end; ++pos) {
            WorldMapChannel::Ref channel = (*pos).second;
            if ((channel->getZoneId() == zoneId) && (channel->getMapId() == mapId)) {
                channelId = channel->getChannelId();
                channelMapByZone_[zoneId].erase(channel->getMapId());
                channelMap_.erase(pos);
                break;
            }
        }

        channelCount = channelMap_.size();
    }

    SNE_LOG_INFO("WorldMapChannel(I%u,Z%u,M%u,W%" PRIu64 ") closed. total = %u.",
        channelId, zoneId, worldMapCode, mapId, channelCount);
}


bool WorldMapChannelManager::worldMapEntered(sne::server::ServerId serverId,
    ZoneId zoneId, ObjectId mapId,
    AccountId accountId, ObjectId playerId, const Position& position)
{
    if (! isValidZoneId(zoneId)) {
        assert(false);
        return false;
    }

    WorldMapChannel::Ref channel = getChannel(zoneId, mapId);
    if (! channel) {
        return false;
    }

    if (! channel->enter(accountId)) {
        assert(false);
        return false;
    }

    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
    if (! user) {
        channel->leave(accountId);
        SNE_LOG_WARNING("Player(E%" PRIu64 ") is entered WorldMap(W%" PRIu64 "), but user(A%" PRIu64 ") not found.",
            playerId, mapId, accountId);
        return false;
    }

    const PartyId partyId = user->getPartyId();
    CommunityPartyManager::CommunityPartyPtr party = COMMUNITYPARTY_MANAGER->getParty(partyId);
    if (party.get() != nullptr) {
        COMMUNITYSERVERSIDEPROXY_MANAGER->partyMemberRejoined(serverId,
            partyId, accountId, playerId);
    }

    user->channelEntered(zoneId, channel->getChannelId(),
        channel->getMapCode(), accountId, playerId, position);

    return true;
}


bool WorldMapChannelManager::worldMapLeft(AccountId accountId, ObjectId playerId)
{
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
    if (! user) {
        SNE_LOG_WARNING("Player(E%" PRIu64 ") is left WorldMap, but user(A%" PRIu64 ") not found.",
            playerId, accountId);
        return false;
    }

    WorldMapChannel::Ref channel = getChannel(user->getWorldMapChannelId());
    if (! channel) {
        SNE_LOG_WARNING("Player(E%" PRIu64 ") is left WorldMap, but channel(%u) not found.",
            playerId, user->getWorldMapChannelId());
        return false;
    }

    if (! channel->leave(accountId)) {
        //assert(false);
        return false;
    }

    user->channelLeft();

    return true;
}


void WorldMapChannelManager::zoneServerDisconnected(ZoneId zoneId)
{
    if (! isValidZoneId(zoneId)) {
        return;
    }

    {
        std::lock_guard<LockType> lock(lock_);

        WorldMapChannelMapById& channelMapById = channelMapByZone_[zoneId];
        for (WorldMapChannelMapById::value_type& value : channelMapById) {
            const ChannelId channelId = value.second;

            WorldMapChannel::Ref channel = getChannel_i(channelId);
            if (channel.get() != nullptr) {
                channel->finalize();

                channelMap_.erase(channelId);

                SNE_LOG_INFO("WorldMapChannel(I%u,Z%u) closed. total = %u.",
                    channelId, zoneId, channelMap_.size());
            }
            else {
                assert(false);
            }
        }

        channelMapByZone_.erase(zoneId);
    }
}


void WorldMapChannelManager::userLoggedOut(AccountId accountId)
{
    CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
    if (! user) {
        return;
    }
    COMMUNITYUSER_MANAGER->removeNickname(user->getNickname());

    shardChannel_->leave(accountId);

    WorldMapChannel::Ref channel = getChannel(user->getWorldMapChannelId());
    if (! channel) {
        return;
    }
    (void)channel->leave(accountId);
}


size_t WorldMapChannelManager::getChannelCount() const
{
    std::lock_guard<LockType> lock(lock_);

    return channelMap_.size();
}


WorldMapChannel::Ref WorldMapChannelManager::getChannel(ChannelId channelId)
{
    std::lock_guard<LockType> lock(lock_);

    return getChannel_i(channelId);
}


WorldMapChannel::Ref WorldMapChannelManager::getChannel(ZoneId zoneId, ObjectId mapId)
{
    WorldMapChannel::Ref channel;

    if (! isValidZoneId(zoneId)) {
        return channel;
    }

    {
        std::lock_guard<LockType> lock(lock_);

        WorldMapChannelMapById& channelMapById = channelMapByZone_[zoneId];
        const WorldMapChannelMapById::iterator pos = channelMapById.find(mapId);
        if (pos != channelMapById.end()) {
            const ChannelId channelId = (*pos).second;
            channel = getChannel_i(channelId);
        }
    }
    return channel;
}


void WorldMapChannelManager::addShardChannel()
{
    const ChannelId channelId = invalidChannelId;
    const ZoneId zoneId = invalidZoneId;
    const ObjectId mapId = invalidObjectId;

    {
        std::lock_guard<LockType> lock(lock_);

        shardChannel_.reset(
            new WorldMapChannel(channelId, zoneId, invalidMapCode, mapId));
        shardChannel_->initialize();
    }

    SNE_LOG_INFO("Shard WorldMapChannel opened.");
}


ChannelId WorldMapChannelManager::generateChannelId(ZoneId zoneId)
{
    return (zoneId << 16) | (++channelIndex_);
}

}} // namespace gideon { namespace communityserver {
