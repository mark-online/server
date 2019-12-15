#pragma once

#include <gideon/cs/shared/data/Money.h>
#include <gideon/cs/shared/data/Time.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class OutsideInventoryCallback
 */
class OutsideInventoryCallback
{
public:
    virtual ~OutsideInventoryCallback() {}

    virtual void guildInventoryItemPushed(ErrorCode errorCode) = 0;
    virtual void guildInventoryItemPopped(ErrorCode errorCode) = 0;
    virtual void guildInventoryItemSwitched(ErrorCode errorCode) = 0;

    virtual void guildGameMoneyDepositted(ErrorCode errorCode, GameMoney gameMoney) = 0;
    virtual void guildGameMoneyWithdrawed(ErrorCode errorCode, sec_t resetDayWithdrawTime, GameMoney todayWithdraw, GameMoney gameMoney) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
