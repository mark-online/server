#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockQuestObtainMissionTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockQuestObtainMissionTable::MockQuestObtainMissionTable()
{
	fillQuestObtainMission();
}

const datatable::QuestObtainMissionTemplate* MockQuestObtainMissionTable::getQuestObtainMissionTemplate(QuestMissionCode code) const
{
	const QuestObtainMissionMap::const_iterator pos = missionMap_.find(code);
	if (pos != missionMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}


void MockQuestObtainMissionTable::fillQuestObtainMission()
{
	gdt::quest_obtain_mission_t mission;
	mission.mission_code(scorpionObtainMissionCode);
	mission.goal(qgtObtainItem);
	mission.goal_info(normalNpcElementCode1);
	mission.goal_value(1);
	missionMap_.insert(QuestObtainMissionMap::value_type(scorpionObtainMissionCode,
		datatable::QuestObtainMissionTemplate(mission)));
}

}} //namespace gideon { namespace servertest {
