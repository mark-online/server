#pragma once

#include <gideon/cs/shared/data/CastInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CastCallback
 */
class CastCallback
{
public:
    virtual ~CastCallback() {}

public:
    virtual void casted(const StartCastResultInfo& startInfo) = 0;

    virtual void canceled(const CancelCastResultInfo& cancelInfo) = 0;

    virtual void failed(const FailCompletedCastResultInfo& failInfo) = 0;

    virtual void completed(const CompleteCastResultInfo& completeInfo) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
