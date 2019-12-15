#pragma once

#include <gideon/cs/shared/data/MailInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace serverbase {

/***
 * @class GameMailCallback
 ***/
class GameMailCallback
{
public:
    virtual ~GameMailCallback() {}

    virtual void onSyncMail(ObjectId playerId, const MailInfos& mails) = 0;
    virtual void onReceive(ObjectId playerId, const MailInfo& mail) = 0;
    virtual void onNotReadMail(ObjectId playerId) = 0;
};


}} // namespace gideon { namespace serverbase {