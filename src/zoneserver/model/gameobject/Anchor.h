#pragma once

#include "StaticObject.h"
#include "ability/SkillCastable.h"
#include "ability/CastGameTimeable.h"
#include "ability/SelectRecipeProductionable.h"
#include "ability/CastNotificationable.h"
#include "ability/Tickable.h"
#include "../../service/time/GameTimer.h"
#include <gideon/cs/shared/data/AnchorInfo.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <sne/base/concurrent/Future.h>

namespace gideon { namespace zoneserver {
class GlobalCoolDownTimer;
class CoolDownTimer;
}} //namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

class SkillCastableAbility;
class Entity;
class Knowable;

/**
 * @class Anchor
 * 설치물
 */
class Anchor : public StaticObject,
    public SkillCastable,
    public CastGameTimeable,
    private Tickable
{
public:
	Anchor(std::unique_ptr<gc::EntityController> controller);
    virtual ~Anchor();

    bool initialize(DataCode dataCode, ObjectId objectId,
        const ObjectPosition& position, go::Entity* player);

    const AnchorOwnerInfo& getOwnerInfo() const {
        return anchorInfo_.ownerInfo_;
    } 

private:
    virtual void finalize();
    
protected: // = Entity overriding
    // virtual ErrorCode despawn() override;
    virtual std::unique_ptr<EffectScriptApplier> createEffectScriptApplier() override;
    virtual std::unique_ptr<EffectHelper> createEffectHelper() override;

public: // = EntityAbility overriding
    virtual Knowable* queryKnowable() override { return knowable_.get(); }
    virtual CastGameTimeable* queryCastGameTimeable() override { return this; }
    virtual SkillCastable* querySkillCastable() override {return this;}
    virtual CastNotificationable* queryCastNotificationable() override {
        return castNotificationable_.get();
    }
    virtual Tickable* queryTickable() override {return this;}

private:
    virtual void tick(GameTime diff);
    virtual void handleExpiredTasks();

private:
    virtual void setCooldown(DataCode dataCode, GameTime coolTime,
        uint32_t index, GameTime globalCoolDownTime);
    virtual void cancelCooldown(DataCode dataCode, uint32_t index);
    virtual void cancelPreCooldown();

    virtual bool isGlobalCooldown(uint32_t index) const;
    virtual bool isLocalCooldown(DataCode dataCode) const;

private:
    // = Positionable
    virtual WorldPosition getWorldPosition() const;

protected:
    // = SkillCastable overriding
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, SkillCode skillCode);
    virtual ErrorCode castAt(const Position& targetPosition, SkillCode skillCode);

    virtual void cancel(SkillCode skillCode);
    virtual void cancelConcentrationSkill(SkillCode skillCode);
    virtual void cancelAll();
    virtual void consumePoints(const Points& points);
    virtual void consumeMaterialItem(const BaseItemInfo& itemInfo);

    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const GameObjectInfo& targetInfo) const;
    virtual ErrorCode checkSkillCasting(SkillCode skillCode,
        const Position& targetPosition) const;

    virtual float32_t getLongestSkillDistance() const;
    virtual bool isUsing(SkillCode skillCode) const;
    virtual bool canCast(SkillCode skillCode) const;

private:
    virtual DataCode getEntityCode() const {
        return anchorInfo_.anchorCode_;
    }
 
    virtual float32_t getModelingRadiusSize() const {
        return anchorTemlate_->getModelingRadius();
    }
	
private:
    const datatable::AnchorTemplate* anchorTemlate_;
    std::unique_ptr<Knowable> knowable_;
    std::unique_ptr<SkillCastableAbility> skillCastableAbility_;
    std::unique_ptr<GlobalCoolDownTimer> globalCoolTime_;
    std::unique_ptr<CoolDownTimer> coolTime_;
    std::unique_ptr<CastNotificationable> castNotificationable_;
    
    GameTimeTracker oneSecondTracker_;
    AnchorInfo anchorInfo_;   
    GameTime expireTime_;
    SkillCode skillCode_;
    GameTime nextActivateTime_;
    GameTime interval_;
};


}}} // namespace gideon { namespace zoneserver { namespace go {
