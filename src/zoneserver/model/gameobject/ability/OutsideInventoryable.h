#pragma once

#include <gideon/cs/shared/data/InventoryInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {
class Inventory;
}} // namespace gideon { namespace zoneserver

namespace gideon { namespace zoneserver {  namespace go {

class Player;

/**
 * @class OutsideInventoryable
 * 장치에 인벤을 사용할수 있다
 */
class OutsideInventoryable
{
public:
    virtual ~OutsideInventoryable() {}
	
public:
    virtual Inventory* getInventory(InvenType invenType) = 0;
    virtual const Inventory* getInventory(InvenType invenType) const = 0;

    virtual ErrorCode checkInventoryUseable(InvenType invenType, go::Entity& player) const = 0;
    virtual ErrorCode canAddItem(InvenType invenType, DataCode itemCode) const = 0;
    virtual ErrorCode canOutItem(InvenType invenType, go::Entity& player) const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
