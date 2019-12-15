#pragma once

#include <gideon/cs/shared/data/MailInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class MailCallback
 */
class MailCallback
{
public:
    virtual ~MailCallback() {}

public:
    virtual void mailSynchronized(const MailTitleInfos& titles) = 0;
    virtual void mailReceived(MailId mailId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
