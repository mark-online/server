#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/itemInfo.h>
#include <sne/core/utility/Singleton.h>

namespace gideon { namespace zoneserver {

class Inventory;

/**
 * @class OrderInvenItemMoveService
 *
 * 은행 서비스
 */
class ZoneServer_Export OrderInvenItemMoveService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(OrderInvenItemMoveService);

public:
	OrderInvenItemMoveService() {}
	~OrderInvenItemMoveService() {}

public:
	ErrorCode playerInvenToOutsideInven(Inventory& playerInven, Inventory& buildingInven, 
        ObjectId playerItemId, SlotId buildingSlotId);
	ErrorCode outsideInvenToPlayerInven(Inventory& playerInven, Inventory& buildingInven,
        ObjectId buildingItemId, SlotId playerSlotId);
	ErrorCode switchStaticObjectAndPlayerInven(Inventory& playerInven, Inventory& buildingInven,
        ObjectId invenItemId, ObjectId buildingItemId);
	/*ErrorCode depositMoney(go::Player& owner, GameMoney money);
	ErrorCode withdrawMoney(go::Player& owner, GameMoney money);*/

};

}} // namespace gideon { namespace zoneserver {

#define ORDER_INVEN_ITEM_MOVE_SERVICE gideon::zoneserver::OrderInvenItemMoveService::instance()
