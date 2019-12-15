#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/StaticObjectSkillInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/Code.h>

namespace gideon { namespace zoneserver {
class Skill;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class StaticObjectCastable
 * 장치가 시전할 수 있다
 */
class StaticObjectCastable
{
public:
    /// 특정 대상에게 시전한다
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, StaticObjectSkillCode skillCode) = 0;

    /// 특정 에어리어에 시전한다
    virtual ErrorCode castAt(const Position& targetPosition, StaticObjectSkillCode skillCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {