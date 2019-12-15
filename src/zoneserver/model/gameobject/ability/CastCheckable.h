#pragma once

#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/SkillCastableEquipItemTypeInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver {
class Skill;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class CastCheckable
 * 시전을 체크 수 있다
 */
class CastCheckable
{
public:
    virtual ~CastCheckable() {}

public:
    virtual bool checkCastableNeedSkill(SkillCode needSkillCode) const = 0;
    virtual bool checkCastableUsableState(SkillUseableState needUsableState) const = 0;
    virtual bool checkCastableEquip(EquipPart checekEquipPart, SkillCastableEquipType checkAllowedType) const = 0;
    virtual bool checkCastableNeedItem(const BaseItemInfo& needItem) const = 0;
    virtual bool checkCastableEffectCategory(EffectStackCategory category) const = 0;
    virtual ErrorCode checkCastablePoints(PointType pt, bool isPercent,
        bool isCheckUp, uint32_t checkValue) const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {










