#include "ZoneServerTestPCH.h"
#include "MockPlayerMailController.h"

using namespace gideon::zoneserver;

MockPlayerMailController::MockPlayerMailController(zoneserver::go::Entity* owner) :
    PlayerMailController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}


DEFINE_SRPC_METHOD_3(MockPlayerMailController, onSendMail,
    ErrorCode, errorCode, SendMailInfo, mailInfo, GameMoney, gameMoney)
{
    addCallCount("onSendMail");
    lastErrorCode_ = errorCode;
    mailInfo, gameMoney;
}


DEFINE_SRPC_METHOD_1(MockPlayerMailController, onSynchronizeMail,
    ErrorCode, errorCode)
{
    addCallCount("onSynchronizeMail");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_2(MockPlayerMailController, onMailTitle,
    ErrorCode, errorCode, MailTitleInfo, mailTitleInfo)
{
    addCallCount("onMailTitle");
    lastErrorCode_ = errorCode;
    mailTitleInfo;
}


DEFINE_SRPC_METHOD_2(MockPlayerMailController, onMailContent,
    ErrorCode, errorCode, MailContentInfo, mailContentInfo)
{
    addCallCount("onMailContent");
    lastErrorCode_ = errorCode;
    mailContentInfo;
}


DEFINE_SRPC_METHOD_2(MockPlayerMailController, onAquireItemInMail,
    ErrorCode, errorCode, ObjectId, mailId)
{
    addCallCount("onAquireItemInMail");
    lastErrorCode_ = errorCode;
    mailId;
}


DEFINE_SRPC_METHOD_2(MockPlayerMailController, onDeleteMail,
    ErrorCode, errorCode, ObjectId, mailId)
{
    addCallCount("onDeleteMail");
    lastErrorCode_ = errorCode;
    mailId;
}


DEFINE_SRPC_METHOD_1(MockPlayerMailController, evMailSynchronized,
    MailTitleInfos, mailTitleInfos)
{
    addCallCount("evMailSynchronized");
    mailTitleInfos;
}


DEFINE_SRPC_METHOD_1(MockPlayerMailController, evMailReceived,
    ObjectId, mailId)
{
    addCallCount("evMailReceived");
    mailId;
}
