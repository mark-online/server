#pragma once

#include "ZoneServer/controller/player/PlayerMailController.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockPlayerMailController
 *
 * 테스트 용 mock MockPlayerMailController
 */
class MockPlayerMailController :
    public zoneserver::gc::PlayerMailController,
    public sne::test::CallCounter
{
public:
    MockPlayerMailController(zoneserver::go::Entity* owner);

    OVERRIDE_SRPC_METHOD_3(onSendMail,
        ErrorCode, errorCode, SendMailInfo, mailInfo, GameMoney, gameMoney);
    OVERRIDE_SRPC_METHOD_1(onSynchronizeMail,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onMailTitle,
        ErrorCode, errorCode, MailTitleInfo, mailTitleInfo);    
    OVERRIDE_SRPC_METHOD_2(onMailContent,
        ErrorCode, errorCode, MailContentInfo, mailContentInfo);    
    OVERRIDE_SRPC_METHOD_2(onAquireItemInMail,
        ErrorCode, errorCode, ObjectId, mailId);
    OVERRIDE_SRPC_METHOD_2(onDeleteMail,
        ErrorCode, errorCode, ObjectId, mailId);

    OVERRIDE_SRPC_METHOD_1(evMailSynchronized,
        MailTitleInfos, mailTitleInfos);
    OVERRIDE_SRPC_METHOD_1(evMailReceived,
        ObjectId, mailId);

public:
    ErrorCode lastErrorCode_;
    ObjectId lastAnchorId_;
    ObjectId lastOwnerId_;
};
