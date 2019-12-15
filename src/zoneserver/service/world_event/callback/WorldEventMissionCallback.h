#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Code.h>
#include <gideon/cs/shared/data/LevelInfo.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {

typedef sne::core::Map<GameObjectInfo, DataCode> WorldEventEntityInfoMap;

/***
 * @class WorldEventMissionCallback
 ***/
class WorldEventMissionCallback
{
public:
    WorldEventMissionCallback() {}
    virtual ~WorldEventMissionCallback() {}

    virtual void killed(go::Entity* killer, CreatureLevel level, const GameObjectInfo& entityInfo) = 0;
    virtual void pathNodeGoalArrived(go::Entity& npc) = 0;

    /// 미션이 사라질 때 소환해야할 NPC 등록
    virtual void reserveRespawn(go::Entity& npc) = 0;

    virtual void joinMission(go::Entity& npc, go::Entity& player) = 0;
};

}} // namespace gideon { namespace zoneserver {
