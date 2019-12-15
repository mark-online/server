#pragma once

#include "../EntityEvent.h"
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;
class EntityMap;

/**
 * @class Knowable
 * 관심 영역 내의 Entity를 알 수 있다
 */
class Knowable
{
public:
    /**
     * @class QueryFilter
     */
    class QueryFilter
    {
    public:
        virtual ~QueryFilter() {}

        virtual bool isOk(const Entity& target, const Position& dirTarget) const = 0;
    };

    /**
     * @class EnumerateCommand
     */
    class EnumerateCommand
    {
    public:
        virtual ~EnumerateCommand() {}

        /// @return true를 리턴하면 enumerating을 중지한다
        virtual bool execute(Entity& entity) = 0;
    };

public:
    virtual ~Knowable() {}

public:
    virtual void know(const EntityMap& entities) = 0;
    virtual void know(Entity& entity) = 0;

    virtual void forget(const EntityMap& entities) = 0;
    virtual void forget(Entity& entity) = 0;
    virtual void forget(const GameObjectInfo& info) = 0;
    virtual void forgetAll() = 0;

    virtual void broadcast(EntityEvent::Ref event, bool exceptSelf = false) = 0;

    /// @return command.execute()의 결과값을 리턴한다
    virtual bool enumerate(EnumerateCommand& command) = 0;

    /// @return command.execute()의 결과값을 리턴한다
    virtual bool enumeratePlayers(EnumerateCommand& command) = 0;

public:
    virtual bool doesKnow(const Entity& entity) const = 0;

    virtual bool doesKnow(const GameObjectInfo& objectInfo) const = 0;

    virtual EntityMap getKnownEntities() const = 0;

    virtual GameObjects getKnownEntitiesInArea(const Position& center,
        float32_t radius, const QueryFilter* filter) const = 0;

    virtual size_t getKnownPlayerCount() const = 0;

    /// Entity가 distanceSq 이내에 있는가?
    virtual bool isAnyPlayerIn2dDistance(float32_t distanceSq) const = 0;

    virtual Entity* getEntity(const GameObjectInfo& id) = 0;

    /// entityCode에 해당하는 가장 가까운 entity를 찾는다
    virtual Entity* getNearestEntityBy(DataCode entityCode) = 0;

    /// 가장 가까운 거리에 있는 NPC를 찾는다
    virtual Entity* getNearestNpc() = 0;

    /// 가장 가까운 거리에 있는 Monster를 찾는다
    virtual Entity* getNearestMonster() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
