#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/SkillCastableEquipItemTypeInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver {
class Skill;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class SkillCastable
 * 스킬을 시전할 수 있다
 */
class SkillCastable
{
public:
    virtual ~SkillCastable() {}

public:
    /// 특정 대상에게 스킬을 시전한다
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, SkillCode skillCode) = 0;

    /// 특정 에어리어에 스킬을 시전한다
    virtual ErrorCode castAt(const Position& targetPosition, SkillCode skillCode) = 0;

    /// 최근 시전 중인 스킬을 취소한다
    virtual void cancel(SkillCode skillCode) = 0;

    virtual void cancelConcentrationSkill(SkillCode skillCode) = 0;

    /// 시전 중인 모든 스킬을 취소한다
    virtual void cancelAll() = 0;

    /// 스킬 시전에 필요한 스탯을 소비시킨다
    virtual void consumePoints(const Points& points) = 0;
	
	/// 스킬 시전에 필요한 스탯을 소비시킨다
	virtual void consumeMaterialItem(const BaseItemInfo& itemInfo) = 0;

    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const GameObjectInfo& targetInfo) const = 0;

    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const Position& targetPosition) const = 0;

public:

    /// 가장 긴 스킬 사거리(max_distance)를 얻는다
    virtual float32_t getLongestSkillDistance() const = 0;

    /// 스킬을 사용 중인가?
    virtual bool isUsing(SkillCode skillCode) const = 0;

    /// 스킬을 시전할 수 있는가?
    virtual bool canCast(SkillCode skillCode) const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
