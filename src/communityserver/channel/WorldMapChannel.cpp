#include "CommunityServerPCH.h"
#include "WorldMapChannel.h"
#include "../user/CommunityUserManager.h"
#include "../user/CommunityUser.h"

namespace gideon { namespace communityserver {


bool WorldMapChannel::enter(AccountId accountId)
{
    size_t userCount = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        if (isEntered_i(accountId)) {
            return false;
        }

        communityUserSet_.insert(accountId);
        userCount = communityUserSet_.size();
    }

    SNE_LOG_INFO("WorldMapChannel(C%u) user(A%" PRIu64 ") entered. total = %u.",
        channelId_, accountId, userCount);
    return true;
}


bool WorldMapChannel::leave(AccountId accountId)
{
    size_t userCount = 0;
    {
        std::lock_guard<LockType> lock(lock_);

        if (! isEntered_i(accountId)) {
            return false;
        }

        communityUserSet_.erase(accountId);
        userCount = communityUserSet_.size();
    }

    SNE_LOG_INFO("WorldMapChannel(C%u) user(A%" PRIu64 ") left. total = %u.",
        channelId_, accountId, userCount);
    return true;
}


void WorldMapChannel::say(const PlayerInfo& fromPlayerInfo, const ChatMessage& message)
{
    std::lock_guard<LockType> lock(lock_);

    if (! isEntered_i(fromPlayerInfo.accountId_)) {
        return;
    }

    CommunityUser* from = COMMUNITYUSER_MANAGER->getUser(fromPlayerInfo.accountId_);
    if (! from) {
        //assert(false);
        return;
    }

    for (const AccountId accountId : communityUserSet_) {
        CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
        if (! user) {
            continue;
        }

        if (! user->isBlocked(from->getPlayerId())) {
            if (isShardChannel()) {
                user->evShardSaid(fromPlayerInfo.playerId_, fromPlayerInfo.nickname_, message);
            }
            else {
                user->evWorldMapSaid(fromPlayerInfo.playerId_, fromPlayerInfo.nickname_, message);
            }
        }
    }
}


void WorldMapChannel::notice(const ChatMessage& message)
{
    std::lock_guard<LockType> lock(lock_);

    for (const AccountId accountId : communityUserSet_) {
        CommunityUser* user = COMMUNITYUSER_MANAGER->getUser(accountId);
        if (user != nullptr) {
            user->evShardNoticed(message);
        }
    }
}


bool WorldMapChannel::isEntered(AccountId accountId) const
{
    std::lock_guard<LockType> lock(lock_);

    return isEntered_i(accountId);
}

}} // namespace gideon { namespace communityserver {
