#pragma once

#include "../../zoneserver_export.h"
#include "../../model/gameobject/Npc.h"
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/NpcActiveSkillTable.h>
#include <sne/core/memory/MemoryPoolMixin.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

/**
 * @class NpcSkillList
 *
 * NPC 보유 스킬 목록
 */
class NpcSkillList : public boost::noncopyable,
    public sne::core::ThreadSafeMemoryPoolMixin<NpcSkillList>
{
    struct SpecialSkillInfo
    {
        const datatable::NpcActiveSkillTemplate* skillTemplate_;
        const datatable::SkillEffectTemplate* effectTemplate_;

        explicit SpecialSkillInfo(const datatable::NpcActiveSkillTemplate* skillTemplate = nullptr,
            const datatable::SkillEffectTemplate* effectTemplate = nullptr) :
            skillTemplate_(skillTemplate),
            effectTemplate_(effectTemplate) {}
    };

    typedef sne::core::Map<SkillCode, SpecialSkillInfo> SpecialSkillMap;

public:
    NpcSkillList(go::Npc& owner, const datatable::NpcTemplate& npcTemplate) :
        owner_(owner),
        npcTemplate_(npcTemplate),
        mainSkillTemplate_(nullptr),
        subSkillTemplate_(nullptr) {
        initSkillList();
    }

public:
    float32_t getMainSkillMinDistance() const {
        if (mainSkillTemplate_ != nullptr) {
            return mainSkillTemplate_->skillCastCheckDistanceInfo_.minDistance_;
        }
        return 0.0f;
    }

    float32_t getMainSkillMaxDistance() const {
        if (mainSkillTemplate_ != nullptr) {
            return mainSkillTemplate_->skillCastCheckDistanceInfo_.maxDistance_;
        }
        return 0.0f;
    }

    float32_t getMaxDistance(SkillCode skillCode) const {
        const datatable::NpcActiveSkillTemplate* skillTemplate = NPC_ACTIVE_SKILL_TABLE->getNpcSkill(skillCode);
        assert(skillTemplate);
        return skillTemplate->skillCastCheckDistanceInfo_.maxDistance_;
    }

    SkillCode getAttackableSkillCode(TargetingType& targetType, const go::Entity& target) const {
        const float32_t distanceToTargetSq = owner_.getSquaredLength(target.getPosition());
        const datatable::NpcSpecialSkills& skills = npcTemplate_.getNpcSpecialSkills();
        for (const datatable::NpcSpecialSkill& skill : skills) {
            if (skill.shouldActiveSkill()) {
                const SpecialSkillInfo& skillInfo =
                    const_cast<NpcSkillList*>(this)->specialSkillMap_[skill.skillCode_];
                if (skillInfo.skillTemplate_->isAttackable(distanceToTargetSq)) {
                    targetType = skillInfo.effectTemplate_->getTargetingType();
                    if (canCastSkill(skill.skillCode_)) {
                        return skill.skillCode_;
                    }                                       
                }
            }
        }

        if (mainSkillTemplate_ != nullptr) {
            if (mainSkillTemplate_->isAttackable(distanceToTargetSq)) {
                targetType = mainSkillEffectTemplate_->getTargetingType();
                const SkillCode skillCode = mainSkillTemplate_->getSkillCode();
                if (canCastSkill(skillCode)) {
                    return skillCode;
                }
            }
        }

        if (subSkillTemplate_ != nullptr) {
            if (subSkillTemplate_->isAttackable(distanceToTargetSq)) {
                targetType = subSkillEffectTemplate_->getTargetingType();
                const SkillCode skillCode = subSkillTemplate_->getSkillCode();
                if (canCastSkill(skillCode)) {
                    return skillCode;
                }
            }
        }
        return invalidSkillCode;
    }

    SkillCode getCastableBuffSkillCode() const {
        const datatable::NpcSpecialSkills& skills = npcTemplate_.getNpcSpecialSkills();
        for (const datatable::NpcSpecialSkill& skill : skills) {
            if (skill.shouldActiveSkill()) {
                const SpecialSkillInfo& skillInfo =
                    const_cast<NpcSkillList*>(this)->specialSkillMap_[skill.skillCode_];
                if (skillInfo.skillTemplate_->isBuffSkill()) {
                    if (canCastSkill(skill.skillCode_)) {
                        return skill.skillCode_;
                    }                                       
                }
            }
        }

        if (mainSkillTemplate_ != nullptr) {
            if (mainSkillTemplate_->isBuffSkill()) {
                const SkillCode skillCode = mainSkillTemplate_->getSkillCode();
                if (canCastSkill(skillCode)) {
                    return skillCode;
                }
            }
        }

        if (subSkillTemplate_ != nullptr) {
            if (subSkillTemplate_->isBuffSkill()) {
                const SkillCode skillCode = subSkillTemplate_->getSkillCode();
                if (canCastSkill(skillCode)) {
                    return skillCode;
                }
            }
        }
        return invalidSkillCode;
    }

    bool shouldStopDuringCasting(SkillCode skillCode) const {
        const datatable::NpcActiveSkillTemplate* skillTemplate = NPC_ACTIVE_SKILL_TABLE->getNpcSkill(skillCode);
        if (! skillTemplate) {
            return false;
        }
        return ! skillTemplate->isMovingSkill();
    }

    const CreatureSkills& getLearnedSkills() const {
        return skills_;
    }

    bool hasSkill() const {
        return ! skills_.empty();
    }

private:
    void initSkillList() {
        const SkillCode mainSkillCode = npcTemplate_.getInfo().main_skill_code();
        if (isValidSkillCode(mainSkillCode)) {
            mainSkillTemplate_ = NPC_ACTIVE_SKILL_TABLE->getNpcSkill(mainSkillCode);
            if (mainSkillTemplate_ != nullptr) {
                mainSkillEffectTemplate_ = SKILL_EFFECT_TABLE->getSkillEffectTemplate(mainSkillTemplate_->effectIndex_);
                assert(mainSkillEffectTemplate_);
                if (mainSkillEffectTemplate_) {
                    addSkill(*mainSkillTemplate_);
                }
            }
        }

        const SkillCode subSkillCode = npcTemplate_.getInfo().sub_skill_code();
        if (isValidSkillCode(subSkillCode)) {
            subSkillTemplate_ = NPC_ACTIVE_SKILL_TABLE->getNpcSkill(subSkillCode);
            if (subSkillTemplate_ != nullptr) {
                subSkillEffectTemplate_ = SKILL_EFFECT_TABLE->getSkillEffectTemplate(subSkillTemplate_->effectIndex_);
                assert(subSkillEffectTemplate_);
                if (subSkillEffectTemplate_) {
                    addSkill(*subSkillTemplate_);
                }
            }
        }
        const datatable::NpcSpecialSkills& skills = npcTemplate_.getNpcSpecialSkills();
        for (const datatable::NpcSpecialSkill& skill : skills) {
            const datatable::NpcActiveSkillTemplate* skillTemplate = NPC_ACTIVE_SKILL_TABLE->getNpcSkill(skill.skillCode_);
            const datatable::SkillEffectTemplate* effectTemplate = SKILL_EFFECT_TABLE->getSkillEffectTemplate(skillTemplate->effectIndex_);      
            assert(skillTemplate && effectTemplate);
            specialSkillMap_.emplace(skillTemplate->getSkillCode(), SpecialSkillInfo(skillTemplate, effectTemplate));
            addSkill(*skillTemplate);
        }
    }

    void addSkill(const datatable::NpcActiveSkillTemplate& skillTemplate) {
        skillTemplate;
        skills_.insert(skillTemplate.getSkillCode());
    }

private:
    bool canCastSkill(SkillCode skillCode) const {
        assert(skills_.find(skillCode) != skills_.end());
        return owner_.querySkillCastable()->canCast(skillCode);
    }

private:
    go::Npc& owner_;
    const datatable::NpcTemplate& npcTemplate_;

    CreatureSkills skills_;
    const datatable::NpcActiveSkillTemplate* mainSkillTemplate_;
    const datatable::SkillEffectTemplate* mainSkillEffectTemplate_;
    const datatable::NpcActiveSkillTemplate* subSkillTemplate_;
    const datatable::SkillEffectTemplate* subSkillEffectTemplate_;
    SpecialSkillMap specialSkillMap_;
};

}} // namespace gideon { namespace zoneserver {
