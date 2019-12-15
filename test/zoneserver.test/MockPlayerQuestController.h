#pragma once

#include "ZoneServer/controller/player/PlayerQuestController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerQuestController
 *
 * 테스트 용 mock MockPlayerQuestController
 */
class MockPlayerQuestController :
    public zoneserver::gc::PlayerQuestController,
    public sne::test::CallCounter
{
public:
    MockPlayerQuestController(zoneserver::go::Entity* owner);

    // = QuestRpc overriding
    OVERRIDE_SRPC_METHOD_2(onAcceptQuest,
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_2(onCancelQuest,
        ErrorCode, errorCode, QuestCode, questCode);
    OVERRIDE_SRPC_METHOD_3(onCompleteTransportMission,
        ErrorCode, errorCode, QuestCode, questCode, QuestMissionCode, questMissionCode);
    OVERRIDE_SRPC_METHOD_3(onCompleteQuest,
        ErrorCode, errorCode, QuestCode, questCode, DataCode, selectItemCode);


    OVERRIDE_SRPC_METHOD_3(evQuestMissionUpdated,
        QuestCode, questCode, QuestMissionCode, missionCode,
        QuestGoalInfo, questGoalInfo);


public:
    ErrorCode lastErrorCode_;
};
