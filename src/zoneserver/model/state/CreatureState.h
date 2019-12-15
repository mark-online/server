#pragma once

#include <gideon/cs/shared/data/NpcInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class CreatureState
 **/
class CreatureState
{
public:
	virtual ~CreatureState() {}

public:
	virtual void died() = 0;
	virtual void revived() = 0;
	virtual bool stunned(bool isActivate) = 0;
    virtual bool slept(bool isActivate) = 0;
    virtual bool mutated(NpcCode npcCode, sec_t duration, bool isActivate) = 0;
    virtual bool transformed(NpcCode npcCode, bool isActivate) = 0;
    virtual bool panicked(sec_t duration, bool isActivate) = 0;
    virtual bool hidden(bool isActivate) = 0;
    virtual bool frenzied(bool isActivate) = 0;
    virtual bool paralysed(bool isActivate) = 0;
    virtual bool shackled(bool isActivate) = 0;
    virtual bool invincible(bool isActivate) = 0;
    virtual bool knockbacked(bool isActivate) = 0;
    virtual bool magicDisabled(bool isActivate) = 0;
    virtual bool castingBindRecalled(bool isActivate) = 0;
    virtual bool spawnProtection(bool isActivate) = 0;
public:
    virtual bool isDied() const =0 ;
    virtual bool isHidden() const = 0;
    virtual bool isSpawnProtection() const = 0;
    virtual bool isKnockback() const = 0;
    virtual bool isCastingBindRecall() const = 0;
    virtual bool isDisableMagic() const = 0;
    virtual bool isMutated() const = 0;
    virtual bool isSleeped() const = 0;
    virtual bool isInvincible() const = 0;
    virtual bool canApplyThreat() const = 0;
    virtual bool isCrowdControl() const = 0;

public:
    bool canDialog() const {
        return (! isDied()) && (! isHidden()) && (! isKnockback());
    }

    bool canSelectTarget() const {
        // TODO: 이동이 강제되는 상태에서는 대상을 선택할 수 없다
        return (! isDied()) && (! isKnockback());
    }
};


}} // namespace gideon { namespace zoneserver {
