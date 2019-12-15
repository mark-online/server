#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Code.h>

namespace gideon { namespace zoneserver {
class Skill;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class ItemCastable
 * 아이템을 시전할 수 있다
 */
class ItemCastable
{
public:
    virtual ~ItemCastable() {}

    /// 특정 대상에게 시전한다
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, ObjectId objectId) = 0;

    /// 특정 에어리어에 시전한다
    virtual ErrorCode castAt(const Position& targetPosition, ObjectId objectId) = 0;

    /// 시전을 취소 할수있다
    virtual void cancel(ObjectId objectId) = 0;


    virtual void insertCastItem(ObjectId itemId, DataCode dataCode) = 0;

    virtual void removeCastItem(ObjectId itemId) = 0;

    virtual ErrorCode checkItemCasting(ObjectId itemId,
        const GameObjectInfo& targetInfo) const = 0;

    virtual ErrorCode checkItemCasting(ObjectId itemId,
        const Position& targetPosition) const = 0;

    virtual bool checkQuestItem(ObjectId objectId) const = 0;

    virtual void useQuestItem(ObjectId objectId) = 0;

    virtual bool checkElementItem(ObjectId objectId) const = 0;

    virtual void useElementItem(ObjectId objectId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {