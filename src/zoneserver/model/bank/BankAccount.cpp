#include "ZoneServerPCH.h"
#include "BankAccount.h"
#include "../gameobject/Entity.h"
#include "../gameobject/ability/Achievementable.h"
#include "esut/Random.h"

namespace gideon { namespace zoneserver {

namespace {

Inventory::LostType getLostType()
{
	uint32_t value = esut::random(0, 1000);
	if (value > 950) {
		return Inventory::ltDelete;
	}
	else if (value > 850) {
		return Inventory::ltDrop;
	}
	return Inventory::ltProtect;
}

} // namespace {

BankAccount::BankAccount(go::Entity& owner, BankAccountInfo& bankAccountInfo) :
    Inventory(owner, bankAccountInfo, itBank),
	owner_(owner),
	bankAccountInfo_(bankAccountInfo)
{
}


void BankAccount::depositMoney(GameMoney gameMoney)
{
    {
	    std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

	    bankAccountInfo_.gameMoney_ += gameMoney;
    }
    owner_.queryAchievementable()->updateAchievement(acmtBankGameMoney, nullptr);
}


void BankAccount::withdrawMoney(GameMoney gameMoney)
{
	std::lock_guard<go::Entity::LockType> lock(owner_.getLock());

	if (bankAccountInfo_.gameMoney_ > gameMoney) {
		bankAccountInfo_.gameMoney_ -= gameMoney;
	}
	else {
		bankAccountInfo_.gameMoney_ = gameMoneyMin;
	}
}

}} // namespace gideon { namespace zoneserver {
