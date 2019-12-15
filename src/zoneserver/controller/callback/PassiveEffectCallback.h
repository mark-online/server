#pragma once

#include <gideon/cs/shared/data/SkillEffectInfo.h>


namespace gideon { namespace zoneserver {
class Skill;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
class GraveStone;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PassiveEffectCallback
 * 캐스터만이 영향을 받는다
 */
class PassiveEffectCallback
{
public:
    virtual ~PassiveEffectCallback() {}
public:
    /// 스킬 영향을 적용한다
    virtual void addCasterEffect(const SkillEffectResult& skillEffectResult) = 0;
    virtual void removeCasterEffect(SkillCode skillCode) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
