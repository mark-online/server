#pragma once

#include "../zoneserver_export.h"
#include "Controller.h"
#include "callback/EffectCallback.h"
#include "callback/CreatureEffectCallback.h"
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon {
struct Position;
} // namespace gideon 


namespace gideon { namespace zoneserver {
class SkillCasterState;
class CreatureState;
}} // namespace gideon { namespace zoneserver {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class SkillController
 * 스킬 담당
 */
class ZoneServer_Export CreatureEffectController : public Controller,
    public EffectCallback,
    public CreatureEffectCallback
{
public:
    CreatureEffectController(go::Entity* owner);
    virtual ~CreatureEffectController() {}
    
    virtual void initialize() {}

private:
    // = EffectCallback overriding
    virtual void applyEffect(go::Entity& from,
        const SkillEffectResult& skillEffectResult);
    virtual void hitEffect(go::Entity& from, DataCode dataCode);
    virtual void cancelEffect(EffectStackCategory category);

    virtual void addEffect(const DebuffBuffEffectInfo& info);
    virtual void removeEffect(DataCode dataCode, bool isCaster);

    // = CreatureEffectCallback overriding
    virtual void effectiveMesmerization(MesmerizationType mezt, bool isActivate);
    virtual void mutate(NpcCode npcCode, sec_t duration, bool isActivate);
    virtual void transform(NpcCode npcCode, bool isActivate);
    virtual void panic(sec_t duration, bool isActivate);
    virtual void changeMoveSpeed(float32_t oldSpeed);
    virtual void knockback(const Position& position);
    virtual void knockbackRelease();
    virtual void dash(const Position& position);
    virtual void reviveAtOnce(permil_t perRefillHp);

private:
    virtual bool mesmerizationEffected(MesmerizationType mezt, bool isActivate);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
