#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/QuestItemTable.h>

namespace gideon { namespace servertest {

/**
* @class MockQuestItemTable
*/
class GIDEON_SERVER_API MockQuestItemTable : public gideon::datatable::QuestItemTable
{
public:
	MockQuestItemTable();
	virtual ~MockQuestItemTable() {
		destroyQuestItem();
	}
	virtual const gideon::datatable::QuestItemTemplate* getQuestItemTemplate(QuestItemCode code) const;

private:
	void fillQuestItem();
	void destroyQuestItem();

	virtual const QuestItemMap& getQuestItemMap() const {
		return questItemMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	QuestItemMap questItemMap_;
};

}} // namespace gideon { namespace servertest {