#pragma once

#include <gideon/cs/shared/data/TradeInfo.h>
#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/Money.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class TradeCallback
 */
class TradeCallback
{
public:
    virtual ~TradeCallback() {}

    virtual void tradeGotRequest(ObjectId playerId) = 0;
	virtual void tradeGotRespond(TradeId tradeId, ObjectId playerId, bool isAnswer) = 0;
	virtual void tradeCancelled(TradeId tradeId, ObjectId playerId) = 0;
	virtual void tradeReadyToggled(bool isReady, GameMoney gameMoney) = 0;
	virtual void tradeConformToggled(bool isReady) = 0;
	virtual void tradeItemAdded(const ItemInfo& itemInfo) = 0;
	virtual void tradeItemRemoved(ObjectId itemId) = 0;
	virtual void tradeCompleted(TradeId tradeId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
