#pragma once

#include "../SkillCastable.h"
#include "../Knowable.h"
#include "../../Entity.h"
#include "../../../../service/skill/SkillService.h"

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class SkillCastableAbility
 * 스킬을 시전할 수 있는 능력
 */
class SkillCastableAbility : public SkillCastable
{
    typedef std::shared_ptr<Skill> SkillPtr;
    typedef sne::core::HashMap<SkillCode, SkillPtr> SkillMap;

public:
    SkillCastableAbility(Entity& owner) :
        owner_(owner) {}
        
    void finalize() {
        for (SkillMap::value_type& value : skills_) {
            Skill* skill = (value.second).get();
            skill->cancel();
        }
    }

    void learn(SkillCode skillCode) {
        assert(! isLearned(skillCode));
        if (isLearned(skillCode)) {
            return;
        }

        auto skill = SKILL_SERVICE->createSkillFor(owner_, skillCode);

        {
            std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

            skills_.emplace(skillCode, std::move(skill));
        }
    }

    void unlearn(SkillCode skillCode) {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        skills_.erase(skillCode);
    }

    bool isLearned(SkillCode skillCode) const {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        return skills_.find(skillCode) != skills_.end();
    }

public:
    // = SkillCastable overriding
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, SkillCode skillCode) {
        SkillPtr skill = getSkill(skillCode);
        if (! skill.get()) {
            assert(false);
            return ecSkillNotLearned;
        }

        return skill->castTo(targetInfo);
    }

    virtual ErrorCode castAt(const Position& targetPosition, SkillCode skillCode) {
        SkillPtr skill = getSkill(skillCode);
        if (! skill.get()) {
            assert(false);
            return ecSkillNotLearned;
        }

        return skill->castAt(targetPosition);
    }

    virtual void cancel(SkillCode skillCode) {
        SkillPtr skill = getSkill(skillCode);
        if (! skill.get()) {
            return;
        }

        skill->cancel();
    }

    virtual void cancelConcentrationSkill(SkillCode skillCode) {
        SkillPtr skill = getSkill(skillCode);
        if (! skill.get()) {
            return;
        }

        skill->cancelConcentrationSkill();
    }

    virtual void cancelAll() {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        for (SkillMap::value_type& value : skills_) {
            Skill* skill = (value.second).get();
            if (skill->isUsing()) {
                skill->cancel();
            }
        }
    }

    virtual void consumePoints(const Points& points) {
        points;
        assert(false && "DON'T CALL ME!");
    }

    virtual void consumeMaterialItem(const BaseItemInfo& itemInfo) {
        itemInfo;
        assert(false && "DON'T CALL ME!");
    }

    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const GameObjectInfo& targetInfo) const {
        Knowable* knowable = owner_.queryKnowable();
        if (! knowable) {
            assert(false);
            return ecSkillTargetNotFound;
        }

        {
            std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

            if (! isLearned(skillCode)) {
                return ecSkillNotLearned;
            }

            if (! knowable->doesKnow(targetInfo)) {
                if (! owner_.isSame(targetInfo)) {
                    return ecSkillTargetNotFound;
                }
            }
        }
        return ecOk;
    }

    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const Position& targetPosition) const {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        if (! isLearned(skillCode)) {
            return ecSkillNotLearned;
        }

        // TODO: targetPosition 검사할게 있나?
        targetPosition;
        return ecOk;
    }

    virtual float32_t getLongestSkillDistance() const {
        const float32_t defaultDistance = 1.0f;

        float32_t maxDistance = defaultDistance;

        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        for (const SkillMap::value_type& value : skills_) {
            const Skill* skill = (value.second).get();

            const float32_t distance = skill->getMaxDistance();
            if (distance > maxDistance) {
                maxDistance = distance;
            }
        }
        return maxDistance;
    }

    virtual bool isUsing(SkillCode skillCode) const {
        const SkillPtr skill =
            const_cast<SkillCastableAbility*>(this)->getSkill(skillCode);
        if (! skill.get()) {
            return false;
        }
        return skill->isUsing();
    }

    virtual bool canCast(SkillCode skillCode) const {
        const SkillPtr skill =
            const_cast<SkillCastableAbility*>(this)->getSkill(skillCode);
        if (! skill.get()) {
            return false;
        }
        return skill->canCast();
    }

private:    
    SkillPtr getSkill(SkillCode skillCode) {
        std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

        const SkillMap::iterator pos = skills_.find(skillCode);
        if (pos == skills_.end()) {
            return SkillPtr();
        }
        return (*pos).second;
    }

private:
    Entity& owner_;

    SkillMap skills_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
