#pragma once

#include "../communityserver_export.h"
#include <gideon/cs/shared/data/ChannelInfo.h>
#include <gideon/cs/shared/data/ZoneInfo.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <gideon/cs/shared/data/PlayerInfo.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace communityserver {

/**
 * @class WorldMapChannel
 *
 * 월드맵 채팅 채널
 */
class CommunityServer_Export WorldMapChannel :
    public sne::core::ThreadSafeMemoryPoolMixin<WorldMapChannel>
{
    typedef std::mutex LockType;

    typedef sne::core::HashSet<AccountId> CommunityUserSet;

public:
    typedef std::shared_ptr<WorldMapChannel> WorldMapChannel::Ref;

public:
    WorldMapChannel(ChannelId channelId, ZoneId zoneId, MapCode worldMapCode,
        ObjectId mapId) :
        channelId_(channelId),
        zoneId_(zoneId),
        worldMapCode_(worldMapCode),
        mapId_(mapId) {}

    ~WorldMapChannel() {}

    void initialize() {}
    void finalize() {
        // TODO: 모든 플레이어를 퇴장시켜야 한다
    }

public:
    bool enter(AccountId accountId);
    bool leave(AccountId accountId);

    void say(const PlayerInfo& fromPlayerInfo, const ChatMessage& message);

    void notice(const ChatMessage& message);

public:
    bool isEntered(AccountId accountId) const;

public:
    ChannelId getChannelId() const {
        return channelId_;
    }

    ZoneId getZoneId() const {
        return zoneId_;
    }

    MapCode getMapCode() const {
        return worldMapCode_;
    }

    ObjectId getMapId() const {
        return mapId_;
    }

private:
    bool isEntered_i(AccountId accountId) const {
        const CommunityUserSet::const_iterator pos = communityUserSet_.find(accountId);
        return pos != communityUserSet_.end();
    }

    bool isShardChannel() const {
        return ! isValidChannelId(channelId_);
    }

private:
    const ChannelId channelId_;
    const ZoneId zoneId_;
    const MapCode worldMapCode_;
    const ObjectId mapId_;

    CommunityUserSet communityUserSet_;

    mutable LockType lock_;
};

}} // namespace gideon { namespace communityserver {
