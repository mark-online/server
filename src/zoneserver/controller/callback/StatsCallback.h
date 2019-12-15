#pragma once

#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/EffectStatusInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class StatsCallback
 */
class StatsCallback
{
public:
    virtual ~StatsCallback() {}

    virtual void pointChanged(const GameObjectInfo& entityInfo, 
        PointType pointType, uint32_t currentPoint) = 0;

    virtual void maxPointChanged(const GameObjectInfo& entityInfo,
        PointType pointType, uint32_t currentPoint, uint32_t maxPoint) = 0;
    
    virtual void creatureStatusChanged(EffectStatusType effectStatusType,
        int32_t currentValue) = 0;

    virtual void allAttributeChanged(const AttributeRates& attribute, bool isResist) = 0;
 
    virtual void pointsRestored(const GameObjectInfo& entityInfo) = 0;

    virtual void fullCreatureStatusChanged() = 0;

    virtual void creatureLifeStatsChanged(const GameObjectInfo& entityInfo,
        const LifeStats& lifeStats) = 0;

    virtual void chaoticUpdated(Chaotic chaotic) = 0;

    virtual void shieldCreated(const GameObjectInfo& entityInfo, PointType pointType, uint32_t value) = 0;
    virtual void shieldDestroyed(const GameObjectInfo& entityInfo, PointType pointType) = 0;
    virtual void shieldPointChanged(const GameObjectInfo& entityInfo, 
        PointType pointType, uint32_t currentPoint) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
