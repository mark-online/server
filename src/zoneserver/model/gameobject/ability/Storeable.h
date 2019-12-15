#pragma once

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class Storeable
 * 상점을 이용할수 있다.
 */
class Storeable
{
public:
    virtual ~Storeable() {}
	
public:
	virtual void sellItem(uint64_t& currentValue, uint32_t& buyBackIndex, BuyBackItemInfo& buyBackInfo, ObjectId itemId, uint8_t itemCount,
		CostType ct, uint32_t totalValue) = 0;
    virtual ErrorCode buyBackItem(CostType& ct, uint64_t& currentValue, uint32_t index) = 0;
	virtual ErrorCode buyItem(const BaseItemInfo& itemInfo, uint64_t& currentValue,
		CostType ct, uint32_t totalValue) = 0;
	virtual ErrorCode repairGlider(ObjectId gliderId) = 0;
    virtual const BuyBackItemInfoMap queryBuyBackItemInfoMap() const = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
