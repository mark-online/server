#include "ZoneServerPCH.h"
#include "AggroSendableImpl.h"
#include "../../Entity.h"
#include "../Thinkable.h"
#include "../Knowable.h"
#include "../../../../ai/aggro/AggroList.h"
#include "../../../../ai/Brain.h"

namespace gideon { namespace zoneserver { namespace go {

AggroSendableImpl::AggroSendableImpl(Entity& owner) :
    owner_(owner),
    additionThreatPer_(0)
{
}


void AggroSendableImpl::setAggroTarget(const GameObjectInfo& targetInfo, go::Entity& entity)
{
    std::lock_guard<LockType> lock(lockAggroble_);

    targetMap_.emplace(targetInfo, &entity);
}


void AggroSendableImpl::removeAggroTarget(const GameObjectInfo& targetInfo)
{
    std::lock_guard<LockType> lock(lockAggroble_);

    targetMap_.erase(targetInfo);
}


void AggroSendableImpl::updateTagetThreat(const GameObjectInfo& senderInfo, uint32_t threatValue, bool isDivision, bool isRemove)
{
    go::Entity* sender = owner_.queryKnowable()->getEntity(senderInfo);
    if (! sender) {
        return;
    }
    go::AggroSendable* sendalbe = sender->queryAggroSendable();
    if (sendalbe) {
        threatValue = sendalbe->getApplyThreat(threatValue);
    }

    {
        std::lock_guard<LockType> lock(lockAggroble_);

        if (isDivision) {
            if (targetMap_.size() > 0) {
                threatValue = static_cast<uint32_t>(threatValue / targetMap_.size());
            }
        }

        if (threatValue == 0) {
            return;
        }

        for (EntityMap::value_type& value : targetMap_) {
            go::Entity* target = value.second;
            go::Thinkable* thinkable = target->queryThinkable();
            if (thinkable != nullptr) {
                ai::AggroList* aggroList = thinkable->getBrain().getAggroList();
                if (aggroList != nullptr) {
                    if (isRemove) {
                        aggroList->downThreat(*sender, threatValue);
                    }
                    else {
                        aggroList->addThreat(*sender, getApplyThreat(threatValue), threatValue);
                    }
                }
            }
        }
    }
}


void AggroSendableImpl::updateAdditionThreat(int32_t threatPer, bool isRemove)
{
    std::lock_guard<LockType> lock(lockAggroble_);

    if (isRemove) {
        additionThreatPer_ -= threatPer;
    }
    else {
        additionThreatPer_ += threatPer;
    }
}


uint32_t AggroSendableImpl::getApplyThreat(uint32_t threat)
{
    std::lock_guard<LockType> lock(lockAggroble_);

    int additionThreat = threat * additionThreatPer_ / 1000;
    return threat += additionThreat;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
