#pragma once


namespace gideon { namespace zoneserver {

/**
 * @class ItemManageState
 **/
class ItemManageState
{
public:
	virtual ~ItemManageState() {}

public:
	virtual bool canInventoryChangeState() const = 0;
	virtual bool canEquipOrUnEquipState() const = 0;
	virtual bool canItemUseableState() const = 0;

    virtual bool isInventoryLock() const = 0;

public:
    // 커뮤니티 서버랑 통신할때만 사용하도록...
    virtual void lockLnventory() = 0;
    virtual void unlockLnventory() = 0;
};


}} // namespace gideon { namespace zoneserver {
