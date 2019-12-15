#pragma once

#include "../communityserver_export.h"
#include "WorldMapChannel.h"
#include <gideon/cs/shared/data/ChannelInfo.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/PlayerInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/server/s2s/ServerId.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace communityserver {

/**
 * @class WorldMapChannelManager
 *
 * WorldMapChannel 관리자
 */
class CommunityServer_Export WorldMapChannelManager : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(WorldMapChannelManager);

    typedef sne::core::HashMap<ChannelId, WorldMapChannel::Ref> WorldMapChannelMap;
    typedef sne::core::HashMap<ObjectId, ChannelId> WorldMapChannelMapById;
    typedef sne::core::HashMap<ZoneId, WorldMapChannelMapById> WorldMapChannelMapByZone;

    typedef std::mutex LockType;

public:
    WorldMapChannelManager();
    ~WorldMapChannelManager();

    void initialize();

public:
    void noticeToShard(const ChatMessage& message);

public:
    void worldMapOpened(ZoneId zoneId, MapCode worldMapCode, ObjectId mapId);
    void worldMapClosed(ZoneId zoneId, MapCode worldMapCode, ObjectId mapId);

    bool worldMapEntered(sne::server::ServerId serverId, ZoneId zoneId, ObjectId mapId,
        ObjectId accountId, AccountId playerId, const Position& position);
    bool worldMapLeft(AccountId accountId, ObjectId playerId);

    void zoneServerDisconnected(ZoneId zoneId);

public:
    void userLoggedOut(AccountId accountId);

public:
    size_t getChannelCount() const;

    WorldMapChannel::Ref getShardChannel() {
        return shardChannel_;
    }

    WorldMapChannel::Ref getChannel(ChannelId channelId);
    WorldMapChannel::Ref getChannel(ZoneId zoneId, ObjectId mapId);

private:
    void addShardChannel();

private:
    WorldMapChannel::Ref getChannel_i(ChannelId channelId) {
        WorldMapChannelMap::iterator pos = channelMap_.find(channelId);
        if (pos != channelMap_.end()) {
            return (*pos).second;
        }

        return WorldMapChannel::Ref();
    }

private:
    ChannelId generateChannelId(ZoneId zoneId);

private:
    WorldMapChannel::Ref shardChannel_;
    WorldMapChannelMap channelMap_;
    WorldMapChannelMapByZone channelMapByZone_;

    uint16_t channelIndex_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace communityserver {

#define WORLDMAP_CHANNEL_MANAGER \
    gideon::communityserver::WorldMapChannelManager::instance()
