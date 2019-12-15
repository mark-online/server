#pragma once

#include "StaticObject.h"
#include "ability/SkillCastable.h"
#include "ability/CastGameTimeable.h"
#include "ability/StaticObjectOwerable.h"
#include "ability/SelectRecipeProductionable.h"
#include "ability/Tickable.h"
#include "../../service/time/GameTimer.h"
#include <gideon/cs/shared/data/StaticObjectSkillInfo.h>
#include <gideon/cs/datatable/AnchorTemplate.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <sne/base/concurrent/Future.h>


namespace gideon { namespace zoneserver { 
class GlobalCoolDownTimer;
class CoolDownTimer;
}} // namespace gideon { namespace zoneserver { 

namespace gideon { namespace zoneserver { namespace go {

class Entity;
class Knowable;
class SkillCastableAbility;
class AbstractAnchor;
class NpcHireable;

/**
 * @class AbstractAnchor
 * 설치물
 */
class AbstractAnchor : public StaticObject,
    public SkillCastable,
    public CastGameTimeable,
    public StaticObjectOwnerable,
	private Tickable
{
    typedef StaticObject Parent;

public:
    AbstractAnchor(std::unique_ptr<gc::EntityController> controller);
    virtual ~AbstractAnchor();
    
    virtual void finalize();   

    virtual float32_t getModelingRadiusSize() const = 0;

    virtual const BuildingOwnerInfo& getBuildingOwnerInfo() const = 0;

    virtual DataCode getOwnerCode() const = 0;

    virtual void fillBuildingInfo(sec_t& /*expireIncibleCooltime*/, 
		AnchorOwnerInfo& /*ownerInfo*/, BuildingItemTaskInfos& /*consumeTaskInfos*/,
		BuildingItemTaskInfos& /*resouceTaskInfos*/, BuildingItemTaskInfos& /*selectProductionTaskInfos*/) {}

    virtual sec_t getExpireTime() const = 0;
    virtual sec_t getInvincibleExpireTime() const { return 0; }
    virtual bool isCompleteState() const = 0;
    virtual bool isActiveAbillity() const { return true; }
	virtual bool canForgeFunction() const { return false; }

	virtual ErrorCode canTeleport(go::Entity& /*player*/) const  = 0;

	SkillCastableAbility& getSkillCastableAbility() {
		return *skillCastableAbility_.get();
    }

    virtual void fillOwnerInfo(BuildingOwnerInfo& ownerInfo, BuildingOwnerType ownerType, go::Entity& player);

protected:
    virtual const datatable::AnchorSkillInfo& getSkillInfo(DataCode anchorSkillCode) const = 0;
    virtual ObjectPosition& getAnchorPosition() = 0;
    virtual const ObjectPosition& getAnchorPosition() const = 0;

	virtual void handleExpiredTasks() = 0;

protected:

protected:
    // = Entity overriding
    virtual ErrorCode despawn();
    virtual std::unique_ptr<EffectScriptApplier> createEffectScriptApplier();
    virtual std::unique_ptr<EffectHelper> createEffectHelper();
public:
    // = EntityAbility overriding
    virtual Knowable* queryKnowable() { return knowable_.get(); }
    virtual CastGameTimeable* queryCastGameTimeable() { return this; }
    virtual StaticObjectOwnerable* queryStaticObjectOwnerable() { return this; }
    virtual const StaticObjectOwnerable* queryStaticObjectOwnerable() const { return this; }
    virtual SkillCastable* querySkillCastable() {return this;}
	virtual CastNotificationable* queryCastNotificationable() { return castNotificationable_.get(); }
	virtual Tickable* queryTickable() {return this;}
    virtual SelectRecipeProductionable* querySelectRecipeProductionable() { return nullptr; }
    virtual NpcHireable* queryNpcHireable() {return nullptr;}

private:
	virtual void tick(GameTime diff);

private:
    virtual void setCooldown(DataCode dataCode, GameTime coolTime,
        uint32_t index, GameTime globalCoolDownTime);
    virtual void cancelCooldown(DataCode dataCode, uint32_t index);
	virtual void cancelPreCooldown();

    virtual bool isGlobalCooldown(uint32_t index) const;
    virtual bool isLocalCooldown(DataCode dataCode) const;

public:
    virtual void setPosition(const ObjectPosition& position);
    virtual void setHeading(Heading heading);

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
    std::unique_ptr<Knowable> knowable_;
    std::unique_ptr<SkillCastableAbility> skillCastableAbility_;
    std::unique_ptr<GlobalCoolDownTimer> globalCoolTime_;
    std::unique_ptr<CoolDownTimer> coolTime_;
	std::unique_ptr<CastNotificationable> castNotificationable_;

	GameTimeTracker oneSecondTracker_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
