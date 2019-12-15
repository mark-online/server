#pragma once

#include "CreatureSkill.h"
#include "helper/NpcSkillChecker.h"

namespace gideon { namespace zoneserver {

/**
 * @class NpcSkill
 *
 * 스킬
 */
class NpcSkill :
    public sne::core::ThreadSafeMemoryPoolMixin<NpcSkill>,
    public CreatureSkill
{
public:
    NpcSkill(go::Entity& caster, bool shouldCheckSafeRegion, 
        const datatable::SkillEffectTemplate& skillEffectTemplate,
        const datatable::NpcActiveSkillTemplate& skillTemplate);
    virtual ~NpcSkill();

    virtual const SkillChecker& getSkillChecker() const {
        return npcSkillChecker_;
    }

    virtual const datatable::ActiveSkillTemplate& getActiveSkillTemplate() const;
    virtual const datatable::SkillCastCheckPointInfo& getSkillCastCheckPointInfo() const;

public:
    const datatable::NpcActiveSkillTemplate& skillTemplate_;
    NpcSkillChecker npcSkillChecker_;
};

}} // namespace gideon { namespace zoneserver {
