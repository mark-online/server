
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
 * @class EffectCallback
 */
class EffectCallback
{
public:
    virtual ~EffectCallback() {}
public:
    /// 스킬 영향을 적용한다
    virtual void applyEffect(go::Entity& from,
        const SkillEffectResult& skillEffectResult) = 0;
    virtual void hitEffect(go::Entity& from, DataCode dataCode) = 0;
    virtual void cancelEffect(EffectStackCategory category) = 0;

public:
    /// entity가 스킬의 영향을 받았다(to, from만 받음)
    virtual void effectApplied(go::Entity& to, go::Entity& from,
        const SkillEffectResult& skillEffectResult) = 0;
    virtual void effectHit(go::Entity& to, go::Entity& from, DataCode dataCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
