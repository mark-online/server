#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/TreasureTable.h>

namespace gideon { namespace servertest {

/**
* @class MockTreasureTable
*/
class GIDEON_SERVER_API MockTreasureTable : public gideon::datatable::TreasureTable
{
public:
	MockTreasureTable();
	virtual ~MockTreasureTable() {
		destroyTreasures();
	}
	virtual const gdt::treasure_t* getTreasure(TreasureCode code) const;

private:
	void fillTreasure1();
    void fillTreasure2();
	void destroyTreasures();

	virtual const TreasureMap& getTreasureMap() const {
		return treasureMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	TreasureMap treasureMap_;
};

}} // namespace gideon { namespace servertest {