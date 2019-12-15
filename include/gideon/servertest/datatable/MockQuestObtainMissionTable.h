#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/QuestObtainMissionTable.h>

namespace gideon { namespace servertest {


/**
* @class MockQuestObtainMissionTable
*/
class GIDEON_SERVER_API MockQuestObtainMissionTable : public gideon::datatable::QuestObtainMissionTable
{
public:
	MockQuestObtainMissionTable();	


	virtual const datatable::QuestObtainMissionTemplate* getQuestObtainMissionTemplate(QuestMissionCode code) const;


private:
	void fillQuestObtainMission();
	
	virtual const QuestObtainMissionMap& getQuestObtainMissionMap() const  {
		return missionMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	QuestObtainMissionMap missionMap_;
};

}} // namespace gideon { namespace servertest {