#pragma once

#include <gideon/cs/shared/data/TradeInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class CombatState
 **/
class CombatState
{
public:
    /// 전투 중?
    virtual bool isCombating() const = 0;

    /// 전투 회피(스폰 위치로 복귀) 중?
    virtual bool isEvading() const = 0;

    /// 겁 먹고 도망 중?
    virtual bool isFleeing() const = 0;

public:
    bool shouldRun() const {
        return isCombating() || isEvading() || isFleeing();
    }

    bool canPerceiveEntity() const {
        return ! shouldRun();
    }
};

}} // namespace gideon { namespace zoneserver {
