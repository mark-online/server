#pragma once

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Moneyable
 * 화폐을 사용할수 있다
 */
class Moneyable
{
public:
    virtual ~Moneyable() {}

public:
	virtual GameMoney getGameMoney() const = 0;
	virtual ForgeCoin getForgeCoin() const = 0;
	virtual EventCoin getEventCoin() const = 0;
    virtual GameMoney getBankGameMoney() const = 0;

	virtual void upGameMoney(GameMoney gameMoney) = 0;
	virtual void downGameMoney(GameMoney gameMoney) = 0;
	virtual void setGameMoney(GameMoney gameMoney) = 0;

	virtual void upForgeCoin(ForgeCoin forgeCoin) = 0;
	virtual void downForgeCoin(ForgeCoin forgeCoin) = 0;
	virtual void setForgeCoin(ForgeCoin forgeCoin) = 0;

	virtual void upEventCoin(EventCoin eventCoin) = 0;
	virtual void downEventCoin(EventCoin eventCoin) = 0;
	virtual void setEventCoin(EventCoin eventCoin) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
