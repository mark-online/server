#include "ZoneServerPCH.h"
#include "AggroList.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/ability/Liveable.h"
#include "../../model/gameobject/ability/Thinkable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Factionable.h"
#include "../../model/gameobject/ability/AggroSendable.h"
#include "../../model/gameobject/status/CreatureStatus.h"
#include "../../model/state/CreatureState.h"
#include "../../model/state/GliderState.h"
#include "../../model/state/CombatState.h"
#include "../../world/World.h"
#include "../../ai/event/npc/NpcBrainEventDef.h"
#include "../../ai/Brain.h"
#include "../../service/skill/SkillService.h"
#include "../../service/skill/NpcSkillList.h"
#include <gideon/3d/3d.h>

namespace gideon { namespace zoneserver { namespace ai {

void AggroList::update(const Position& attackedPosition, float32_t maxMoveDistanceSq)
{
    go::Knowable* knowable = owner_.queryKnowable();
    AggroSet eraseAggro;
    {
        std::unique_lock<LockType> lock(lock_);

        AggroSet::iterator pos = aggroSet_.begin();
        for (; pos != aggroSet_.end();) {
            const AggroInfo& aggro = *pos;

            go::Entity* target = knowable->getEntity(aggro.getTarget());
            if ((! target) || (! target->isValid())) {
                eraseAggro.insert(*pos);
                aggroSet_.erase(pos++);				
                continue;
            }

            go::Liveable* liveable = target->queryLiveable();
            if ((liveable != nullptr) && (liveable->getCreatureStatus().isDied())) {
                eraseAggro.insert(*pos);
                aggroSet_.erase(pos++);
                continue;
            }

            CreatureState* creatureState = target->queryCreatureState();
            if ((creatureState != nullptr) && creatureState->isHidden()) {
                eraseAggro.insert(*pos);
                aggroSet_.erase(pos++);
                continue;
            }

            GliderState* gliderState = target->queryGliderState();
            if ((gliderState != nullptr) && gliderState->isMonutGliding()) {
                eraseAggro.insert(*pos);
                aggroSet_.erase(pos++);
                continue;
            }

            if ((maxMoveDistanceSq > 0.0f) &&
                (target->getSquaredLength(attackedPosition) >= maxMoveDistanceSq)) {
                eraseAggro.insert(*pos);
                aggroSet_.erase(pos++);
                continue;
            }

            CombatState* combatState = target->queryCombatState();
            if (combatState && combatState->isEvading()) {
                eraseAggro.insert(*pos);
                aggroSet_.erase(pos++);
                continue;
            }

            ++pos;
        }
    }

    for (const AggroInfo& info : eraseAggro) {
        if (info.getTarget().isPlayer()) {
            go::Entity* player = WORLD->getPlayer(info.getTarget().objectId_);
            if (player) {
                player->queryAggroSendable()->removeAggroTarget(owner_.getGameObjectInfo());
            }
        }
    }
}


go::Entity* AggroList::selectVictim(go::Npc& attacker, go::Entity* currentVictim)
{
    go::Knowable* knowable = owner_.queryKnowable();

    const float32_t mainSkillMaxDistanceSq =
        square(attacker.getSkillList().getMainSkillMaxDistance());

    {
        std::unique_lock<LockType> lock(lock_);

        if (forceVictim_.isValid()) {
            go::Entity* target = knowable->getEntity(forceVictim_);
            if (target) {
                return target;
            }
        }

        const AggroInfo* currentVictimAggro =
            getAggroInfo((currentVictim != nullptr) ? currentVictim->getGameObjectInfo() : GameObjectInfo());
        const float32_t currentVictimThreat =
            (currentVictimAggro != nullptr) ? currentVictimAggro->getThreat() : 0.0f;

        for (const AggroInfo& aggro : aggroSet_) {
            go::Entity* target = knowable->getEntity(aggro.getTarget());
            if ((!target) || (!target->isValid())) {
                continue;
            }

            // TODO: 공격 가능 조건 검사 (지형 등)
            //if (! attacker.canAttack(*target)) {
            //    continue;
            //}

            if (!currentVictim) {
                return target;
            }

            if (target == currentVictim) {
                return currentVictim;
            }

            if (aggro.getThreat() <= (1.1f * currentVictimThreat)) {
                return currentVictim;
            }

            // TODO: http://www.wowwiki.com/Aggro#Drawing_aggro 적용

            // 130% rule for targets in ranged distances
            if (aggro.getThreat() > (1.3f * currentVictimThreat)) {
                return target;
            }

            //implement 110% threat rule for targets in main skill range
            const float32_t distanceToTargetSq = attacker.getSquaredLength(target->getPosition());
            if ((aggro.getThreat() > (1.1f * currentVictimThreat)) &&
                (distanceToTargetSq <= mainSkillMaxDistanceSq)) {
                return target;
            }
        }
    }
    return nullptr;
}


go::Entity* AggroList::selectRandomVictim()
{
    GameObjectInfo targetInfo;
    {
        std::unique_lock<LockType> lock(lock_);

        if (aggroSet_.empty()) {
            return nullptr;
        }

        const int targetIndex = esut::random() % aggroSet_.size();

        int i = 0;
        for (const AggroInfo& aggro : aggroSet_) {
            if (i == targetIndex) {
                targetInfo = aggro.getTarget();
                break;
            }
            ++i;
        }
    }

    if (! targetInfo.isValid()) {
        return nullptr;
    }

    return owner_.queryKnowable()->getEntity(targetInfo);
}


bool AggroList::addThreat(const go::Entity& enemy, uint32_t threat, uint32_t score)
{
    CreatureState* creatureState = owner_.queryCreatureState();
    if (creatureState && ! creatureState->canApplyThreat()) {
        return false;
    }

    if (threat == 0) {
        return false;
    }

    if (! canHate(enemy)) {
        return false;
    }

    const GameObjectInfo& enemyInfo = enemy.getGameObjectInfo();
    {
        std::unique_lock<LockType> lock(lock_);

        if (enemy.isPlayer() && score > 0) {
            increaseEntityScore(enemyInfo, score);
        }

        const ThreadAvoidInfo* info = getThreadAvoidInfo(enemy.getGameObjectInfo());
        if (info) {
            if (info->shouldAvoid()) {
                return false;
            }
        }
        increaseThreat(enemyInfo, threat);
    }

    ai::Brain& brain = owner_.queryThinkable()->getBrain();
    brain.attacked(enemy);
    return true;
}


bool AggroList::addThreatByTopThreat(const go::Entity& enemy, permil_t threatPerValue)
{
    if (threatPerValue <= 0) {
        return false;
    }

    if (! canHate(enemy)) {
        return false;
    }

    uint32_t applyThreat = 0;
    {
        std::unique_lock<LockType> lock(lock_);

        if (aggroSet_.empty()) {
            return false;
        }

        AggroSet::iterator pos = aggroSet_.begin();        
        const AggroInfo& aggro = *pos;
        applyThreat = (aggro.getThreat() * threatPerValue) / 1000;        
    }

    return addThreat(enemy, applyThreat, 0);

}

bool AggroList::addThreatByEvent(const go::Entity& enemy, int eventId)
{
    return addThreat(enemy, eventId, 1, false);
}


bool AggroList::addThreat(const go::Entity& enemy, int eventId, int rate, bool isPercent,
    bool ignoreFaction)
{
    if (! ignoreFaction) {
        if (! canHate(enemy)) {
            return false;
        }
    }

    const GameObjectInfo& enemyInfo = enemy.getGameObjectInfo();
    {
        std::unique_lock<LockType> lock(lock_);

        if (isPercent) {
            const AggroInfo* aggroInfo = getAggroInfo(enemyInfo);
            const uint32_t currentThreat = (aggroInfo != nullptr) ? aggroInfo->getThreat() : 0;
            const uint32_t finalRate = (currentThreat * std::abs(rate)) / 100;
            if (rate >= 0) {
                increaseThreat(enemyInfo, finalRate);
            }
            else {
                decreaseThreat(enemyInfo, finalRate);
            }
        }
        else {
            increaseThreat(enemyInfo, rate);
        }
    }

    ai::Brain& brain = owner_.queryThinkable()->getBrain();
    ai::Stateable* stateable = brain.queryStateable(); assert(stateable != nullptr);
    stateable->asyncHandleEvent(eventId);
    return true;
}


bool AggroList::downThreat(const go::Entity& enemy, uint32_t threat)
{
    if (! canHate(enemy)) {
        return false;
    }

    const GameObjectInfo& enemyInfo = enemy.getGameObjectInfo();
    {
        std::unique_lock<LockType> lock(lock_);

        decreaseThreat(enemyInfo, threat);
    }

    return true;
}


void AggroList::removeThreat(const GameObjectInfo& enemyInfo)
{
    std::unique_lock<LockType> lock(lock_);

    AggroSet::iterator pos = aggroSet_.begin();
    for (; pos != aggroSet_.end(); ++pos) {
        const AggroInfo& info = (*pos);
        if (info.getTarget() == enemyInfo) {
            aggroSet_.erase(pos);
            return;
        }
    }    
}


void AggroList::setThreat(const go::Entity& enemy, uint32_t threat)
{
    if (! canHate(enemy)) {
        return;
    }

    {
        std::unique_lock<LockType> lock(lock_);

        AggroSet::const_iterator pos = aggroSet_.begin();
        const AggroSet::const_iterator end = aggroSet_.end();
        for (; pos != end; ++pos) {
            const AggroInfo& aggro = *pos;
            if (aggro.getTarget() == enemy.getGameObjectInfo()) {
                AggroInfo updatedAggro = aggro;
                updatedAggro.setThreat(threat);
                aggroSet_.erase(pos);
                if (updatedAggro.getThreat() > 0) {
                    aggroSet_.insert(updatedAggro);
                }
                return;
            }
        }
    }
}


void AggroList::upThreatApplyAvoidTarget(const GameObjectInfo& enemy, uint32_t threatApplyRate)
{
    std::unique_lock<LockType> lock(lock_);

    ThreadAvoidInfo* info = getThreadAvoidInfo(enemy);
    if (info) {
        info->upRate(threatApplyRate);
    }
    else {
        threadAvoidInfos_.push_back(ThreadAvoidInfo(enemy, threatApplyRate));
    }
}


void AggroList::downThreatApplyAvoidTarget(const GameObjectInfo& enemy, uint32_t threatApplyRate)
{
    std::unique_lock<LockType> lock(lock_);

    ThreadAvoidInfo* info = getThreadAvoidInfo(enemy);
    if (info) {
        info->downRate(threatApplyRate);
        if (info->shouldAvoid()) {
            removeThreatApplyAvoidTarget(enemy);
        }
    }
}


void AggroList::setForceVictim(const GameObjectInfo& enemyInfo)
{
    go::Entity* enemy = owner_.queryKnowable()->getEntity(enemyInfo);
    if (! enemy) {
        return;
    }

    {
        std::unique_lock<LockType> lock(lock_);

        forceVictim_ = enemyInfo;
    }
    
    ai::Brain& brain = owner_.queryThinkable()->getBrain();
    brain.attacked(*enemy);
}


void AggroList::releaseForceVictim(const GameObjectInfo& enemy)
{    
    std::unique_lock<LockType> lock(lock_);

    if (forceVictim_ == enemy) {
        forceVictim_.reset();
    }       
}


void AggroList::clear()
{
    AggroSet aggroSet;
    {
        std::unique_lock<LockType> lock(lock_);

        aggroSet = aggroSet_;
        aggroSet_.clear();
        attackDamageScoreSet_.clear();
    }

    for (const AggroInfo& info : aggroSet) {
        if (info.getTarget().isPlayer()) {
            go::Entity* player = WORLD->getPlayer(info.getTarget().objectId_);
            if (player) {
                player->queryAggroSendable()->removeAggroTarget(owner_.getGameObjectInfo());
            }
        }
    }
}


go::Entity* AggroList::getMostHated() const
{
    go::Knowable* knowable = owner_.queryKnowable();

    std::unique_lock<LockType> lock(lock_);

    if (! aggroSet_.empty()) {
        const AggroInfo& aggro = *(aggroSet_.begin());
        return knowable->getEntity(aggro.getTarget());
    }

    return nullptr;
}


go::Entity* AggroList::getTopScorePlayer() const
{
    go::Knowable* knowable = owner_.queryKnowable();

    std::unique_lock<LockType> lock(lock_);

    if (! attackDamageScoreSet_.empty()) {
        const AttackDamageScore& score = *(attackDamageScoreSet_.begin());
        return knowable->getEntity(score.playerInfo_);
    }

    return nullptr;
}


uint32_t AggroList::getThreat(const GameObjectInfo& info) const
{
    for (const AggroInfo& aggro : aggroSet_) {
        if (aggro.getTarget() == info) {
            return static_cast<uint32_t>(aggro.getThreat());
        }
    }
    return 0;
}


bool AggroList::isHateTo(go::Entity& entity) const
{
    std::unique_lock<LockType> lock(lock_);

    for (const AggroInfo& aggro : aggroSet_) {
        if (aggro.getTarget() == entity.getGameObjectInfo()) {
            return true;
        }
    }
    return false;
}


bool AggroList::isThreated() const
{
    std::unique_lock<LockType> lock(lock_);

    return ! aggroSet_.empty();
}


bool AggroList::canHate(const go::Entity& target) const
{
    if (owner_.isSame(target)) {
        return false;
    }

    if (owner_.isNpcOrMonster()) {
        go::Npc& npc = static_cast<go::Npc&>(owner_);
        if (npc.getNpcTemplate().getReactiveType() == artNothing) {
            return false;
        }

        // 대상이 추적 가능 거리 내에 있는가?
        const float32_t maxChaseDistance = owner_.queryMoveable()->getMaxMoveDistance();
        if (maxChaseDistance > 0.0f) {
            if (owner_.getSquaredLength(target.getPosition()) >= square(maxChaseDistance)) {
                return false;
            }
        }
    }

    // FYI: StaticNpc는 어그로 적용 안됨!
    if (target.isNpc()) {
        if (static_cast<const go::Npc&>(target).isStaticNpc()) {
            return false;
        }
    }

    go::Thinkable* targetThinkable = const_cast<go::Entity&>(target).queryThinkable();
    if (targetThinkable != nullptr) {
        if (targetThinkable->getBrain().getAggroList()->isHateTo(owner_)) {
            return true;
        }
    }

    go::Factionable* factionable = owner_.queryFactionable();
    if (factionable != nullptr) {
        if (! factionable->isHostileTo(target)) {
            if (factionable->isFriendlyTo(target)) {
                return false;
            }
        }
    }

    return true;
}


void AggroList::increaseThreat(const GameObjectInfo& target, uint32_t threat)
{
    AggroSet::const_iterator pos = aggroSet_.begin();
    const AggroSet::const_iterator end = aggroSet_.end();
    for (; pos != end; ++pos) {
        const AggroInfo& aggro = *pos;
        if (aggro.getTarget() == target) {
            AggroInfo updatedAggro = aggro;
            updatedAggro.increaseThreat(threat);
            aggroSet_.erase(pos);
            aggroSet_.insert(updatedAggro);
            return;
        }
    }

    aggroSet_.insert(AggroInfo(target, threat));
}


void AggroList::decreaseThreat(const GameObjectInfo& target, uint32_t threat)
{
    AggroSet::const_iterator pos = aggroSet_.begin();
    const AggroSet::const_iterator end = aggroSet_.end();
    for (; pos != end; ++pos) {
        const AggroInfo& aggro = *pos;
        if (aggro.getTarget() == target) {
            AggroInfo updatedAggro = aggro;
            updatedAggro.decreaseThreat(threat);
            aggroSet_.erase(pos);
            if (updatedAggro.getThreat() > 0) {
                aggroSet_.insert(updatedAggro);
            }
            return;
        }
    }
}


void AggroList::increaseEntityScore(const GameObjectInfo& target, uint32_t score)
{
    AttackDamageScoreSet::const_iterator pos = attackDamageScoreSet_.begin();
    const AttackDamageScoreSet::const_iterator end = attackDamageScoreSet_.end();
    for (; pos != end; ++pos) {
        const AttackDamageScore& aggro = *pos;
        if (aggro.playerInfo_ == target) {
            AttackDamageScore updatedScore = aggro;
            updatedScore.score_ += score;
            attackDamageScoreSet_.erase(pos);
            attackDamageScoreSet_.insert(updatedScore);
            return;
        }
    }

    attackDamageScoreSet_.insert(AttackDamageScore(target, score));
}


void AggroList::removeThreatApplyAvoidTarget(const GameObjectInfo& target)
{
    ThreadAvoidInfos::iterator pos = threadAvoidInfos_.begin();
    ThreadAvoidInfos::iterator end = threadAvoidInfos_.end();
    for (; pos != end; ++pos) {
        ThreadAvoidInfo& info = (*pos);
        if (info.target_ == target) {
            threadAvoidInfos_.erase(pos);
            return;
        }
    } 
}


const AggroInfo* AggroList::getAggroInfo(const GameObjectInfo& entityInfo) const
{
    for (const AggroInfo& aggro : aggroSet_) {
        if (aggro.getTarget() == entityInfo) {
            return &aggro;
        }
    }
    return nullptr;
}


ThreadAvoidInfo* AggroList::getThreadAvoidInfo(const GameObjectInfo& entityInfo)
{
    for (ThreadAvoidInfo& avoidInfo : threadAvoidInfos_) {
        if (avoidInfo.target_ == entityInfo) {
            return &avoidInfo;
        }
    }
    return nullptr;
}


const ThreadAvoidInfo* AggroList::getThreadAvoidInfo(const GameObjectInfo& entityInfo) const
{
    for (const ThreadAvoidInfo& avoidInfo : threadAvoidInfos_) {
        if (avoidInfo.target_ == entityInfo) {
            return &avoidInfo;
        }
    }
    return nullptr;
}


}}} // namespace gideon { namespace zoneserver { namespace ai {
