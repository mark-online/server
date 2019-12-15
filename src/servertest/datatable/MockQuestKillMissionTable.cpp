#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockQuestKillMissionTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockQuestKillMissionTable::MockQuestKillMissionTable()
{
	fillQuestKillMission();
}

const datatable::QuestKillMissionTemplate* MockQuestKillMissionTable::getQuestKillMissionTemplate(QuestMissionCode code) const
{
	const QuestKillMissionMap::const_iterator pos = missionMap_.find(code);
	if (pos != missionMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}


void MockQuestKillMissionTable::fillQuestKillMission()
{
	gdt::quest_kill_mission_t mission;
	mission.mission_code(scorpionKillMissionCode);
	mission.goal(qgtKillNpc);
	mission.goal_info(normalNpcCode);
	mission.goal_value(1);
	missionMap_.insert(QuestKillMissionMap::value_type(scorpionKillMissionCode,
		datatable::QuestKillMissionTemplate(mission)));
}

}} //namespace gideon { namespace servertest {
