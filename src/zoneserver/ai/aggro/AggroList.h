#pragma once

#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/ItemEffectInfo.h>
#include <gideon/cs/shared/data/Rate.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace go {
class Npc;
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class AggroInfo
 */
class AggroInfo
{
public:
    AggroInfo(const GameObjectInfo& target, uint32_t threat) :
        target_(target),
        threat_(threat) {}

    void increaseThreat(uint32_t threat) {
        threat_ += threat;
    }

    void decreaseThreat(uint32_t threat) {
        if (threat_ < threat) {
            threat_ = 0;
        }
        else {
            threat_ -= threat;
        }        
    }

    void setThreat(uint32_t threat) {
        threat_ = threat;
    }


public:
    bool operator>(const AggroInfo& rhs) const {
        return threat_ > rhs.threat_;
    }

    const GameObjectInfo& getTarget() const {
        return target_;
    }

    uint32_t getThreat() const {
        return threat_;
    }

private:
    GameObjectInfo target_;
    uint32_t threat_;
};


/**
 * @struct AttackDamageScore
 */
struct AttackDamageScore
{
    GameObjectInfo playerInfo_;
    uint32_t score_;

    AttackDamageScore(const GameObjectInfo& playerInfo, uint32_t score) :
        playerInfo_(playerInfo),
        score_(score) {}

    bool operator>(const AttackDamageScore& rhs) const {
        return score_ > rhs.score_;
    }
};

/**
 * @struct AttackDamageScore
 */
struct ThreadAvoidInfo
{
    GameObjectInfo target_;
    uint32_t avoidRate_;

    ThreadAvoidInfo(const GameObjectInfo& target, uint32_t avoidRate) :
        target_(target),
        avoidRate_(avoidRate) {}

    void upRate(uint32_t rate)
    {
        avoidRate_+= rate;
    }

    void downRate(uint32_t rate) {
        if (avoidRate_ < rate) {
            avoidRate_ = 0;
        }
        else {
            avoidRate_ -= rate;
        }
    }

    bool shouldRemove() const {
        return avoidRate_ == 0;
    }

    bool shouldAvoid() const {
        return isSuccessRate(avoidRate_);
    }
};
 


/**
 * @class AggroList
 * 어그로 목록
 * - Lock order: AggroList -> Entity
 */
class AggroList : public boost::noncopyable
{
    typedef sne::core::Set<AggroInfo, std::greater<AggroInfo> > AggroSet;
	typedef sne::core::Set<AttackDamageScore, std::greater<AttackDamageScore> > AttackDamageScoreSet;
    typedef sne::core::Vector<ThreadAvoidInfo> ThreadAvoidInfos;

    typedef std::mutex LockType;

public:
    AggroList(go::Entity& owner) :
        owner_(owner) {}
    virtual ~AggroList() {}

public:
    void update(const Position& attackedPosition, float32_t maxMoveDistanceSq);

    go::Entity* selectVictim(go::Npc& attacker, go::Entity* currentVictim = nullptr);

    go::Entity* selectRandomVictim();

public:    
    bool addThreat(const go::Entity& enemy, uint32_t threat, uint32_t score);
    bool addThreatByTopThreat(const go::Entity& enemy, permil_t threatPerValue);
    bool addThreatByEvent(const go::Entity& enemy, int eventId);
    bool addThreat(const go::Entity& enemy, int eventId, int rate, bool isPercent,
        bool ignoreFaction = false);
    

    bool downThreat(const go::Entity& enemy, uint32_t threat);
    void removeThreat(const GameObjectInfo& enemyInfo);
    void setThreat(const go::Entity& enemy, uint32_t threat);
    void upThreatApplyAvoidTarget(const GameObjectInfo& enemy, uint32_t threatApplyRate);
    void downThreatApplyAvoidTarget(const GameObjectInfo& enemy, uint32_t threatApplyRate);

    void setForceVictim(const GameObjectInfo& enemy);
    void releaseForceVictim(const GameObjectInfo& enemy);
    void clear();

public:
    go::Entity* getMostHated() const;
	go::Entity* getTopScorePlayer() const;

    uint32_t getThreat(const GameObjectInfo& info) const;

    bool isHateTo(go::Entity& entity) const;

    bool isThreated() const;

private:
    void increaseThreat(const GameObjectInfo& target, uint32_t threat);
    void decreaseThreat(const GameObjectInfo& target, uint32_t threat);
	void increaseEntityScore(const GameObjectInfo& target, uint32_t score);
    void removeThreatApplyAvoidTarget(const GameObjectInfo& target);

    bool canHate(const go::Entity& target) const;

    const AggroInfo* getAggroInfo(const GameObjectInfo& entityInfo) const;
    ThreadAvoidInfo* getThreadAvoidInfo(const GameObjectInfo& entityInfo);
    const ThreadAvoidInfo* getThreadAvoidInfo(const GameObjectInfo& entityInfo) const;

private:
    go::Entity& owner_;

    AggroSet aggroSet_;
	AttackDamageScoreSet attackDamageScoreSet_;
    ThreadAvoidInfos threadAvoidInfos_;
      
    GameObjectInfo forceVictim_;
    mutable LockType lock_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
