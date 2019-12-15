#pragma once

#include "../CastNotificationable.h"

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class CastNotificationableImpl
 * 시전할수 있다
 */
class CastNotificationableImpl : public CastNotificationable
{
public:    
    CastNotificationableImpl(Entity& owner);
    virtual ~CastNotificationableImpl();
    
private:
    virtual void notifyStartCasting(const StartCastResultInfo& startInfo);
    virtual void notifyCompleteCasting(const CompleteCastResultInfo& cancelInfo);
    virtual void notifyCancelCasting(const CancelCastResultInfo& completeInfo);
    virtual void notifyCompleteFailedCasting(const FailCompletedCastResultInfo& failInfo);

private:
    Entity& owner_;
};


}}} // namespace gideon { namespace zoneserver { namespace go {