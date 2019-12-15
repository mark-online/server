#pragma once

#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/SkillCastableEquipItemTypeInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/datatable/SkillTemplate.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon {

/**
 * @struct ServerEffectInfo
 */
struct ServerEffectInfo :
    sne::core::ThreadSafeMemoryPoolMixin<ServerEffectInfo>
{   
    struct Effect {
        EffectScriptType script_;
        int32_t value_;
        Position position_;
        int32_t accumulationValue_;
        GameTime activateTime_;
        GameTime stopTime_;
        GameTime intervalTime_;
        GameTime nextIntervalTime_;
        uint32_t callCount_; 
        bool isRemove_;
        bool shouldExcute_;

        Effect() :
            script_(estUnknown),
            value_(0),
            accumulationValue_(0),
            activateTime_(0),
            stopTime_(0),
            intervalTime_(0),
            nextIntervalTime_(0),
            callCount_(0),
            isRemove_(false),
            shouldExcute_(false)
        {
        }
    };

    typedef std::array<Effect, eanCount> Effects; 
         
    DataCode dataCode_;
    bool isCaster_;

    EffectStackCategory category_;

    GameObjectInfo gameObjectInfo_;
    EffectLevel effectLevel_;    
    Effects effects_;
    sec_t startTime_;

    ServerEffectInfo(DataCode dataCode = invalidDataCode,
        bool isCaster = true, EffectStackCategory category = escNone,
        const GameObjectInfo& gameObjectInfo = GameObjectInfo(), sec_t startTime = 0) :
        dataCode_(dataCode),
        isCaster_(isCaster),
		category_(category),
        gameObjectInfo_(gameObjectInfo),
        startTime_(startTime) {}

    bool isValid() const {
        if (startTime_ == 0) {
            return false;
        }
        for (int i = 0; i < eanCount; ++i) {
            if (effects_[i].script_ != estUnknown) {
                return true;
            }
        }
        return false;
    }

    bool shouldRemove() const {
        for (int i = 0; i < eanCount; ++i) {
            if (! effects_[i].isRemove_) {
                return false;
            }
        }
        return true;
    }

    bool hasEffect(EffectScriptType type) const {
        for (int i = 0; i < eanCount; ++i) {
            if (effects_[i].script_ == type) {
                return true;
            }
        }
        return false;
    }

    bool hasTimeEffect() const {
        for (int i = 0; i < eanCount; ++i) {
            if (effects_[i].activateTime_ > 0 && effects_[i].script_ != estUnknown) {
                return true;
            }
        }
        return false;
    }

    bool hasInstantEffect() const {
        for (int i = 0; i < eanCount; ++i) {
            if (effects_[i].intervalTime_ == 0 && effects_[i].script_ != estUnknown) {
                return true;
            }
        }
        return false;
    }


    bool hasOnceEffect() const {
        for (int i = 0; i < eanCount; ++i) {
            if (effects_[i].intervalTime_ == 0 && effects_[i].activateTime_ == 0 && effects_[i].script_ != estUnknown) {
                return true;
            }
        }
        return false;
    }

    bool isTimeEffect(EffectApplyNum num) const {
        return effects_[num].activateTime_ > 0;
    }

    bool isInstantEffect(EffectApplyNum num) const {
        return effects_[num].intervalTime_ == 0;
    }
};


typedef std::shared_ptr<ServerEffectInfo> ServerEffectPtr;
typedef sne::core::HashMap<EffectStackCategory, ServerEffectPtr> ServerEffectMap;
typedef sne::core::Vector<ServerEffectPtr> ServerEffects;
typedef sne::core::Set<ServerEffectPtr> ServerEffectSet;


/**
 * @enum ServerEffectType
 */
enum ServerEffectType
{
    ssetUnknown = 0,
    ssetLifeStats,
    ssetLifeCapacityStatus,
    ssetAttributeRateStatus,
    ssetAttackPowerStatus,
    ssetAttackStatus,
    ssetDefenceStatus,
    ssetExecution,
    ssetStun,
    ssetSleep,
    ssetTransformation,
    ssetMutation,
    ssetHide,
    ssetFrenzy,
    ssetParalyze,
    ssetMoveSpeedUp,
    ssetMoveSpeedDown,
    ssetCastingUp,
    ssetCastingDown,
    ssetDisableMagic,
    ssetShackles,
    ssetKnockback,
    ssetSpawnProtection,
    ssetInvincible,
    ssetBindRecall,
	ssetPlayerRecall,
    ssetDash,
    ssetPanic,
    ssetRevive,
    ssetRelease,
    ssetDirectThreat,
    ssetThreat,
    ssetThreatApplyAvoid,
    ssetForceVictim,
    ssetBackStep,
};

// TODO skill
inline ServerEffectType toServerEffectType(EffectScriptType type)
{
    switch (type) {
    case estHpUp:
    case estHpDown:
    case estMpUp:
    case estMpDown:
    case estHpGive:
    case estMpGive:
    case estHpMpUp:
    case estHpAbsorb:
    case estMpAbsorb:
        return ssetLifeStats;
    case estDodgeRateUp:
    case estDodgeRateDown:
    case estBlockRateUp:
    case estBlockRateDown:
    case estDefenceUp:
    case estDefenceDown:
        return ssetDefenceStatus;
    case estHpCapacityUp:
    case estHpCapacityDown:
    case estMpCapacityUp:
    case estMpCapacityDown:
    case estHpShield:
    case estMpShield:
        return ssetLifeCapacityStatus;        
    case estExecution:
        return ssetExecution;
    case estPhysicalUp:
    case estMagicUp:
    case estPhysicalDown:
    case estMagicDown:
        return ssetAttackPowerStatus;
    case estHitRateUp:
    case estPhysicalCriticalRateUp:
    case estMagicCriticalRateUp:
    case estHitRateDown:
    case estPhysicalCriticalRateDown:
    case estMagicCriticalRateDown:
        return ssetAttackStatus;
    case estSleep:
        return ssetSleep;
    case estLightUp:
    case estDarkUp:
    case estFireUp:
    case estIceUp:
    case estAllAttributeUp:
    case estLightResistUp:
    case estDarkResistUp:
    case estFireResistUp:
    case estIceResistUp:
    case estAllAttributeResistUp:
    case estLightDown:
    case estDarkDown:
    case estFireDown:
    case estIceDown:
    case estAllAttributeDown:
    case estLightResistDown:
    case estDarkResistDown:
    case estFireResistDown:
    case estIceResistDown:
    case estAllAttributeResistDown:
        return ssetAttributeRateStatus;
    case estTransformation:
        return ssetTransformation;
    case estMutation:
        return ssetMutation;
    case estHide:
        return ssetHide;
    case estFrenzy:
        return ssetFrenzy;
    case estParalyze:
        return ssetParalyze;
    case estCastTimeUp:
        return ssetCastingUp;
    case estCastTimeDown:
        return ssetCastingDown;
    case estMoveSpeedUp:
        return ssetMoveSpeedUp;
    case estMoveSpeedDown:
        return ssetMoveSpeedDown;
    case estShackles:
        return ssetShackles;
    case estStun:
        return ssetStun;
    case estKnockback:
        return ssetKnockback;
    case estCombatChange:
    case estPeaceChange:
        return ssetUnknown;
    case estSpawnProtection:        
        return ssetSpawnProtection;
    case estInvincible:
        return ssetInvincible;
    case estBindRecall:
        return ssetBindRecall;
	case estPlayerRecall:
	    return ssetPlayerRecall;
    case estDisableMagic:
        return ssetDisableMagic;
    case estDash:
        return ssetDash;
    case estPanic:
        return ssetPanic;
    case estRevive:
        return ssetRevive;
    case estRelease:
        return ssetRelease;
    case estDirectThreatUp:
    case estDirectThreatDown:
        return ssetDirectThreat;
    case estThreatUp:
    case estThreatDown:
        return ssetThreat;
    case estThreatApplyAvoid:
        return ssetThreatApplyAvoid;
    case estForceVictim:
        return ssetForceVictim;
    case estBackStep:
        return ssetBackStep;
    }
    assert(false);
    return ssetUnknown;
}


/**
 * @enum PassiveCheckCondition
 */
enum PassiveCheckCondition
{
    pccPoints,
    pccUseableState,
    pccEquip,
    pccLearnSkill,
    pccTimer,
    pccCount,
};


inline void fillNextEffectTime(ServerEffects& effects, ServerEffectPtr effectPtr, EffectApplyNum num)
{
    if (1 < effectPtr->effects_[num].callCount_) {
        effectPtr->effects_[num].nextIntervalTime_ = effectPtr->effects_[num].nextIntervalTime_ + effectPtr->effects_[num].intervalTime_;       
    }
    --effectPtr->effects_[num].callCount_;
    effectPtr->effects_[num].shouldExcute_ = true;
    
    effects.push_back(effectPtr);

}

inline bool isExcuteEffect(const ServerEffectPtr effectPtr, GameTime now, EffectApplyNum num)
{
    if (effectPtr->effects_[num].isRemove_) {
        return false;
    }
    if ((effectPtr->effects_[num].nextIntervalTime_ <= now) &&
        (effectPtr->effects_[num].nextIntervalTime_ > 0) && (effectPtr->effects_[num].callCount_ > 0)) {
        return true;        
    }
    return false;
}


inline bool isTimeoutEffect(const ServerEffectPtr effectPtr, GameTime now, EffectApplyNum num)
{
    if (effectPtr->effects_[num].intervalTime_ > 0) {
        if (effectPtr->effects_[num].callCount_ > 0) {
            return false;
        }
    }
    else {
        if (effectPtr->effects_[num].stopTime_ > now) {
            return false;
        }
    }
    return true;
}



} // namespace gideon {
