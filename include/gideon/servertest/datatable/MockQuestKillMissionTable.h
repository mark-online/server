#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/QuestKillMissionTable.h>

namespace gideon { namespace servertest {


/**
* @class MockQuestKillMissionTable
*/
class GIDEON_SERVER_API MockQuestKillMissionTable : public gideon::datatable::QuestKillMissionTable
{
public:
	MockQuestKillMissionTable();	


	virtual const datatable::QuestKillMissionTemplate* getQuestKillMissionTemplate(QuestMissionCode code) const;


private:
	void fillQuestKillMission();
	
	virtual const QuestKillMissionMap& getQuestKillMissionMap() const  {
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
	QuestKillMissionMap missionMap_;
};

}} // namespace gideon { namespace servertest {