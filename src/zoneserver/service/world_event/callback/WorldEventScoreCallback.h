#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/LevelInfo.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}}

namespace gideon { namespace zoneserver {

/***
 * @struct PlayerScoreInfo
***/
struct PlayerScoreInfo
{
    go::Entity* player_;
    uint32_t score_;
    Nickname nickname_;

    PlayerScoreInfo(go::Entity* player, uint32_t score, const Nickname& nickname) :
        player_(player),
        score_(score),
        nickname_(nickname)
    {
    }
};

typedef sne::core::HashMap<ObjectId, PlayerScoreInfo> PlayerScoreInfoMap;

/***
 * @class WorldEventScoreCallback
 ***/
class WorldEventScoreCallback
{
public:
    WorldEventScoreCallback() {}
    virtual ~WorldEventScoreCallback() {}

    virtual void addScore(go::Entity& killer, CreatureLevel level) = 0;
    virtual void resetScore() = 0;
    virtual void reward(bool isCompleteEvent) = 0;
};

}} // namespace gideon { namespace zoneserver {
