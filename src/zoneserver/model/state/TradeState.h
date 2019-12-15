#pragma once

#include <gideon/cs/shared/data/TradeInfo.h>

namespace gideon { namespace zoneserver {

/**
 * @class TradeState
 **/
class TradeState
{
public:
    virtual bool canRequestTrade() const = 0;
	virtual bool canStartTrade() const = 0;
    virtual bool isTradeRequstState() const = 0;

public:
	virtual TradeId getTradeId() const = 0;

public:
    virtual void tradeRequest() = 0;
	virtual void tradeAccepted(TradeId tradeId) = 0;
	virtual void tradeCancelled(TradeId tradeId) = 0;
	virtual void tradeCompleted(TradeId tradeId) = 0;

};


}} // namespace gideon { namespace zoneserver {
