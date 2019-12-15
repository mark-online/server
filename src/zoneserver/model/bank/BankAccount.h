#pragma once

#include "../item/Inventory.h"
#include <gideon/cs/shared/data/BankAccountInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}}

namespace gideon { namespace zoneserver {

/***
 * @class BankAccount
 ***/
class BankAccount : public Inventory
{
public:
	BankAccount(go::Entity& owner, BankAccountInfo& bankAccountInfo) ;
	
	void depositMoney(GameMoney gameMoney);
	void withdrawMoney(GameMoney gameMoney);
	
	GameMoney getGameMoney() const {
		return bankAccountInfo_.gameMoney_;
	}

private:
	go::Entity& owner_;
	BankAccountInfo& bankAccountInfo_;
};


}} // namespace gideon { namespace zoneserver {