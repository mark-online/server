#pragma once

#include "StaticObject.h"
#include "ability/SkillCastable.h"
#include "ability/Tickable.h"
#include <gideon/cs/shared/data/DeviceInfo.h>
#include <gideon/cs/shared/data/StaticObjectSkillInfo.h>
#include <gideon/cs/datatable/DeviceTable.h>

namespace gideon { namespace datatable {
class DeviceTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace ai { namespace evt {
class EventTrigger;
}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {

namespace gideon { namespace zoneserver { namespace go {

class Entity;
class Knowable;
class SkillCastableAbility;

/**
 * @class Device
 */
class Device : public StaticObject,
    private SkillCastable,
    private Tickable
{
    typedef StaticObject Parent;

public:
	Device(std::unique_ptr<gc::EntityController> controller);
    virtual ~Device();
    
public:
    bool initialize(ObjectId deviceId, const datatable::DeviceTemplate& deviceTemplate);

    ErrorCode activate(Entity& activator);

    void deactivate();

public:
    /// @internal
    ErrorCode activated(Entity& activator);

private:
    void afterActivation(Entity& activator);

    void setActivateState(bool isActivated);

    void reserveDeactivation();

private:
    ErrorCode checkActivatable(Entity& activator) const;

private:
    virtual void finalize();

    virtual ErrorCode respawn(WorldMap& worldMap);

    virtual DataCode getEntityCode() const {
        return deviceCode_;
    }

private:
    // = StaticObject overriding
    virtual std::unique_ptr<EffectScriptApplier> createEffectScriptApplier();

public:
    // = EntityAbility overriding
    virtual Knowable* queryKnowable() { return knowable_.get(); }
    virtual SkillCastable* querySkillCastable() {return this;}
    virtual Tickable* queryTickable() { return this; }

protected:
    // = Positionable overriding
    virtual WorldPosition getWorldPosition() const;

private:
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
    // = Tickable overriding
    virtual void tick(GameTime diff);

private:
    ErrorCode checkItemCondition(Entity& activator,
        const datatable::DeviceCondition& condition) const;

private:
    const datatable::DeviceTemplate* deviceTemplate_;
    DeviceCode deviceCode_;

    std::unique_ptr<Knowable> knowable_;
    std::unique_ptr<SkillCastableAbility> skillCastableAbility_;

    std::unique_ptr<ai::evt::EventTrigger> eventTrigger_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
