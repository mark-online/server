#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockQuestTransportMissionTable.h>
#include <gideon/servertest/datatable/DataCodes.h>


namespace gideon { namespace servertest {


MockQuestTransportMissionTable::MockQuestTransportMissionTable()
{
	fillQuestTransportMission();
}

const datatable::QuestTransportMissionTemplate* MockQuestTransportMissionTable::getQuestTransportMissionTemplate(QuestMissionCode code) const
{
	const QuestTransportMissionMap::const_iterator pos = missionMap_.find(code);
	if (pos != missionMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}


void MockQuestTransportMissionTable::fillQuestTransportMission()
{
	gdt::quest_transport_mission_t mission;
	mission.mission_code(questItemTransportMissionCode);
    mission.goal(qgtTransportNpc);
    mission.goal_info(normalNpcCode);

    mission.issue_item_code(transportQuestItemCode);
	mission.map_code(::gdt::map_code_t(0));
	missionMap_.emplace(questItemTransportMissionCode,
		datatable::QuestTransportMissionTemplate(mission));
}


}} //namespace gideon { namespace servertest {
