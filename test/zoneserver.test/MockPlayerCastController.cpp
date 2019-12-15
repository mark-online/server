#include "ZoneServerTestPCH.h"
#include "MockPlayerCastController.h"

using namespace gideon::zoneserver;

MockPlayerCastController::MockPlayerCastController(zoneserver::go::Entity* owner) :
    PlayerCastController(owner),
    lastErrorCode_(ecWhatDidYouTest)
{
}


// = rpc::BankRpc overriding
DEFINE_SRPC_METHOD_2(MockPlayerCastController, onStartCasting,
    ErrorCode, errorCode, StartCastInfo, startInfo)
{
    addCallCount("onStartCasting");
    lastErrorCode_ = errorCode;
    startInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerCastController, evCasted,
    StartCastResultInfo, startInfo)
{
    addCallCount("evCasted");
    startInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerCastController, evCastCanceled,
    CancelCastResultInfo, cancelInfo)
{
    addCallCount("evCastCanceled");
    cancelInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerCastController, evCastCompleted,
    CompleteCastResultInfo, completeInfo)
{
    addCallCount("evCastCompleted");
    completeInfo;
}


DEFINE_SRPC_METHOD_1(MockPlayerCastController, evCastCompleteFailed,
    FailCompletedCastResultInfo, failInfo)
{
    addCallCount("evCastCompleteFailed");
    failInfo;
    lastErrorCode_ = failInfo.errorCode_;
}