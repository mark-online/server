#include "ZoneServerPCH.h"
#include "CastNotificationableImpl.h"
#include "../Knowable.h"
#include "../../Entity.h"
#include "../../../../controller/EntityController.h"
#include "../../../../controller/callback/CastCallback.h"

namespace gideon { namespace zoneserver { namespace go {

namespace {

/**
 * @class StartCastEvent
 */
class StartCastEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<StartCastEvent>
{
public:
    StartCastEvent(const StartCastResultInfo& castInfo) :
        castInfo_(castInfo) {}

private:
    virtual void call(go::Entity& entity) {
        gc::CastCallback* callback = entity.getController().queryCastCallback();
        if (callback != nullptr) {
            callback->casted(castInfo_);
        }
    }

private:
    const StartCastResultInfo castInfo_;
};


/**
 * @class CancelCastEvent
 */
class CancelCastEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CancelCastEvent>
{
public:
    CancelCastEvent(const CancelCastResultInfo& castInfo) :
        castInfo_(castInfo) {}

private:
    virtual void call(go::Entity& entity) {
        gc::CastCallback* callback = entity.getController().queryCastCallback();
        if (callback != nullptr) {
            callback->canceled(castInfo_);
        }
    }

private:
    const CancelCastResultInfo castInfo_;
};


/**
 * @class CompleteCastEvent
 */
class CompleteCastEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CompleteCastEvent>
{
public:
    CompleteCastEvent(const CompleteCastResultInfo& castInfo) :
        castInfo_(castInfo) {}

private:
    virtual void call(go::Entity& entity) {
        gc::CastCallback* callback = entity.getController().queryCastCallback();
        if (callback != nullptr) {
            callback->completed(castInfo_);
        }
    }

private:
    const CompleteCastResultInfo castInfo_;
};


/**
 * @class CompleteFailCastEvent
 */
class CompleteFailCastEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CompleteFailCastEvent>
{
public:
    CompleteFailCastEvent(const FailCompletedCastResultInfo& castInfo) :
        castInfo_(castInfo) {}

private:
    virtual void call(go::Entity& entity) {
        gc::CastCallback* callback = entity.getController().queryCastCallback();
        if (callback != nullptr) {
            callback->failed(castInfo_);
        }
    }

private:
    const FailCompletedCastResultInfo castInfo_;
};

} // namespace {


CastNotificationableImpl::CastNotificationableImpl(Entity& owner) :
    owner_(owner)
{
}
 

CastNotificationableImpl::~CastNotificationableImpl()
{
}


void CastNotificationableImpl::notifyStartCasting(const StartCastResultInfo& startInfo)
{
    auto event = std::make_shared<StartCastEvent>(startInfo);
    owner_.queryKnowable()->broadcast(event);
}


void CastNotificationableImpl::notifyCompleteCasting(const CompleteCastResultInfo& completeInfo)
{
    auto event = std::make_shared<CompleteCastEvent>(completeInfo);
    owner_.queryKnowable()->broadcast(event);
}


void CastNotificationableImpl::notifyCancelCasting(const CancelCastResultInfo& cancelInfo)
{
    auto event = std::make_shared<CancelCastEvent>(cancelInfo);
    owner_.queryKnowable()->broadcast(event);
}


void CastNotificationableImpl::notifyCompleteFailedCasting(const FailCompletedCastResultInfo& failInfo)
{
    auto event = std::make_shared<CompleteFailCastEvent>(failInfo);
    owner_.queryKnowable()->broadcast(event);
}

}}} // namespace gideon { namespace zoneserver { namespace go {
