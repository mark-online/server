#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockExpTable.h>

namespace gideon { namespace servertest {

MockExpTable::MockExpTable()
{
    LevelStepExpPoints invalidLevelStepExpPoints;
    invalidLevelStepExpPoints.fill(ceMin);
    expPoints_[clInvalidLevel] = invalidLevelStepExpPoints;

    // 120부터 10씩 증가
    for (int level = clMinLevel; level <= clMaxPlayerLevel; ++level) {
        LevelStepExpPoints& points = expPoints_[level];
        points[0] = ceMin;
        for(int levelStep = lsMinLevel; levelStep < lsCount; ++levelStep) {  
            points[levelStep] = toExpPoint((level * 120) + ((levelStep - 1) * 10));
        }
    }
}

}} // namespace gideon { namespace servertest {
