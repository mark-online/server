#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/LevelInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class GrowthCallback
 */
class GrowthCallback
{
public:
    virtual ~GrowthCallback() {}

public:
    // 케릭터 레벨이 업됐다
    virtual void playerLeveledUp(const GameObjectInfo& creatureInfo,
        bool isMajorLevelup) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
