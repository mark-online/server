#pragma once

#include "EffectScriptCommand.h"
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace go {

class Creature;

/**
 * @class CreatureEffectScriptCommand
 */
class CreatureEffectScriptCommand : public EffectScriptCommand
{
public:
    CreatureEffectScriptCommand(Creature& owner) :
        owner_(owner) {}

    virtual ~CreatureEffectScriptCommand() {}

    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove) = 0;

protected:
    void updateThreatToMyTarget(const GameObjectInfo& senderInfo, uint32_t aggroValue, 
        bool isDivision, bool isRemove);
    void updateThreatToMe(const GameObjectInfo& senderInfo, uint32_t aggroValue, 
        bool isApplyScore, bool isRemove);
    void removeThreatToTarget(const GameObjectInfo& enemyInfo);
    void setOneThreatToTarget(const GameObjectInfo& enemyInfo);

protected:
    Creature& owner_;
};


/**
 * @class SkillLifePointsStatsEffectCommand
 */
class SkillLifePointsStatsEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillLifePointsStatsEffectCommand>
{
public:
    SkillLifePointsStatsEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);    

    void absorb(ServerEffectInfo::Effect& skillEffect, const GameObjectInfo& casterInfo);
    void give(ServerEffectInfo::Effect& skillEffect, const GameObjectInfo& casterInfo);
};


/**
 * @class SkillLifeCapacityStatsEffectCommand
 */
class SkillLifeCapacityStatsEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillLifeCapacityStatsEffectCommand>
{
public:
    SkillLifeCapacityStatsEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);

private:
    void upLifeStatsCapacity(ServerEffectInfo::Effect& skillEffect, PointType pointType, bool isRemove);
    void downLifeStatsCapacity(ServerEffectInfo::Effect& skillEffect, PointType pointType, bool isRemove);
};


/**
 * @class SkillAttackPowerEffectCommand
 */
class SkillAttackPowerEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillAttackPowerEffectCommand>
{
public:
    SkillAttackPowerEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillAttackStatusEffectCommand
 */
class SkillAttackStatusEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillAttackStatusEffectCommand>
{
public:
    SkillAttackStatusEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillDefenceStatusEffectCommand
 */
class SkillDefenceStatusEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillDefenceStatusEffectCommand>
{
public:
    SkillDefenceStatusEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillAttributeEffectCommand
 */
class SkillAttributeEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillAttributeEffectCommand>
{
public:
    SkillAttributeEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillAttributeResistEffectCommand
 */
class SkillAttributeResistEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillAttributeResistEffectCommand>
{
public:
    SkillAttributeResistEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};

/**
 * @class SkillExecutionEffectCommand
 */
class SkillExecutionEffectCommand  : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillExecutionEffectCommand>
{
public:
    SkillExecutionEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
    void rewardExp(const GameObjectInfo& skillCasterInfo);
};


/**
 * @class SkillStunEffectCommand
 */
class SkillStunEffectCommand  : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillStunEffectCommand>
{
public:
    SkillStunEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillSleepEffectCommand
 */
class SkillSleepEffectCommand  : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillSleepEffectCommand>
{
public:
    SkillSleepEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};



/**
 * @class SkillPanicEffectCommand
 */
class SkillPanicEffectCommand  : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillPanicEffectCommand>
{
public:
    SkillPanicEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillMutationEffectCommand
 */
class SkillMutationEffectCommand  : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillMutationEffectCommand>
{
public:
    SkillMutationEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillTransformationEffectCommand 
 */
class SkillTransformationEffectCommand  : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillTransformationEffectCommand>
{
public:
    SkillTransformationEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillShacklesEffectCommand 
 */
class SkillShacklesEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillShacklesEffectCommand>
{
public:
    SkillShacklesEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillHideEffectCommand 
 */
class SkillHideEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillHideEffectCommand>
{
public:
    SkillHideEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillMoveUpEffectCommand 
 */
class SkillMoveUpEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillMoveUpEffectCommand>
{
public:
    SkillMoveUpEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner)
        {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillMoveDownEffectCommand 
 */
class SkillMoveDownEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillMoveDownEffectCommand>
{
public:
    SkillMoveDownEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner)
        {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillFrenzyEffectCommand 
 */
class SkillFrenzyEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillFrenzyEffectCommand>
{
public:
    SkillFrenzyEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillParalyzeEffectCommand 
 */
class SkillParalyzeEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillParalyzeEffectCommand>
{
public:
    SkillParalyzeEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillCastingDelayUpEffectCommand 
 */
class SkillCastingDelayUpEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillCastingDelayUpEffectCommand>
{
public:
    SkillCastingDelayUpEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillCastingDelayDownEffectCommand 
 */
class SkillCastingDelayDownEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillCastingDelayDownEffectCommand>
{
public:
    SkillCastingDelayDownEffectCommand (Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillKnockbackEffectCommand
 */
class SkillKnockbackEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillKnockbackEffectCommand >
{
public:
    SkillKnockbackEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SpawnProtectionEffectCommand
 */
class SpawnProtectionEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SpawnProtectionEffectCommand >
{
public:
    SpawnProtectionEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class InvincibleEffectCommand
 */
class InvincibleEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<InvincibleEffectCommand>
{
public:
    InvincibleEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class BindRecallEffectCommand
 */
class BindRecallEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<BindRecallEffectCommand>
{
public:
    BindRecallEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillReleaseScriptCommand
 */
class SkillReleaseScriptCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillReleaseScriptCommand>
{
public:
    SkillReleaseScriptCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class PlayerRecallCommand
 */
class PlayerRecallCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<PlayerRecallCommand>
{
public:
    PlayerRecallCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class DisableMagicCommand
 */
class DisableMagicCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<DisableMagicCommand>
{
public:
    DisableMagicCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillDashEffectCommand
 */
class SkillDashEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillDashEffectCommand>
{
public:
    SkillDashEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class SkillReviveEffectCommand
 */
class SkillReviveEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<SkillReviveEffectCommand>
{
public:
    SkillReviveEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class DirectThreatEffectCommand
 */
class DirectThreatEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<DirectThreatEffectCommand>
{
public:
    DirectThreatEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class ThreatEffectCommand
 */
class ThreatEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<ThreatEffectCommand>
{
public:
    ThreatEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class ThreatApplyAvoidEffectCommand
 */
class ThreatApplyAvoidEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<ThreatApplyAvoidEffectCommand>
{
public:
    ThreatApplyAvoidEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class ForceVictimEffectCommand
 */
class ForceVictimEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<ForceVictimEffectCommand>
{
public:
    ForceVictimEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};


/**
 * @class BackStepEffectCommand
 */
class BackStepEffectCommand : public CreatureEffectScriptCommand,
    public sne::core::ThreadSafeMemoryPoolMixin<BackStepEffectCommand>
{
public:
    BackStepEffectCommand(Creature& owner) :
        CreatureEffectScriptCommand(owner) {}

private:
    virtual void execute(ServerEffectPtr effectPtr, EffectApplyNum num, bool isRemove);
};

/**
 * @class EffectInfoCommandHelper
 */
class EffectInfoCommandHelper
{
public:
    virtual CreatureEffectScriptCommand* getEffectInfoCommand(EffectScriptType type) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {