#include "ZoneServerTestPCH.h"
#include "MockPlayerQuestController.h"

using namespace gideon::zoneserver;

MockPlayerQuestController::MockPlayerQuestController(zoneserver::go::Entity* owner) :
    PlayerQuestController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}



// = QuestRpc overriding

DEFINE_SRPC_METHOD_2(MockPlayerQuestController, onAcceptQuest,
    ErrorCode, errorCode, QuestCode, questCode)
{
    addCallCount("onAcceptQuest");

    lastErrorCode_ = errorCode;
    questCode;
}


DEFINE_SRPC_METHOD_2(MockPlayerQuestController, onCancelQuest,
    ErrorCode, errorCode, QuestCode, questCode)
{
    addCallCount("onCancelQuest");

    lastErrorCode_ = errorCode;
    questCode;
}


DEFINE_SRPC_METHOD_3(MockPlayerQuestController, onCompleteQuest,
    ErrorCode, errorCode, QuestCode, questCode, DataCode, selectItemCode)
{
    addCallCount("onCompleteQuest");

    lastErrorCode_ = errorCode;
    questCode, selectItemCode;
}


DEFINE_SRPC_METHOD_3(MockPlayerQuestController, onCompleteTransportMission,
    ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode)
{
    addCallCount("onCompleteTransportMission");

    lastErrorCode_ = errorCode;
    questCode, questMissionCode;
}


DEFINE_SRPC_METHOD_3(MockPlayerQuestController, evQuestMissionUpdated,
    QuestCode, questCode, QuestMissionCode, missionCode,
    QuestGoalInfo, questGoalInfo)
{
    addCallCount("evQuestMissionUpdated");
    questCode, missionCode, questGoalInfo;
}
