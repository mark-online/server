#pragma once

#include <gideon/cs/shared/data/CastInfo.h>

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class CastNotificationable
 * 시전 통보를 있다
 */
class CastNotificationable
{
public:    
    virtual ~CastNotificationable() {}

    virtual void notifyStartCasting(const StartCastResultInfo& startInfo) = 0;
    virtual void notifyCompleteCasting(const CompleteCastResultInfo& cancelInfo) = 0;
    virtual void notifyCancelCasting(const CancelCastResultInfo& completeInfo) = 0;
    virtual void notifyCompleteFailedCasting(const FailCompletedCastResultInfo& failInfo) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace go {