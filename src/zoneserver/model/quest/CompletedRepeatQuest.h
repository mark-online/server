#pragma once

#include "CompletedQuest.h"
#include <ctime>

namespace gideon { namespace zoneserver {

/***
 * @class CompletedRepeatQuest
 ***/
class CompletedRepeatQuest : public CompletedQuest
{
public:
	CompletedRepeatQuest(QuestCode questCode, sec_t acceptTime) : 
		CompletedQuest(questCode),
        acceptTime_(acceptTime) {}

private:
    virtual bool isEnoughAcceptTime() const {
        sec_t now = getTime();
        struct tm* nowInfo = _localtime64(&now);        
        if (! nowInfo) {
            return false;
        }
        struct tm nowTm = *nowInfo;
        time_t acceptTime = static_cast<time_t>(acceptTime_);
        struct tm* acceptInfo = _localtime64(&acceptTime);
        if (! acceptInfo) {
            return false;
        }
        struct tm acceptTm = *acceptInfo;
        return nowTm.tm_yday != acceptTm.tm_yday;
    }

private:
    sec_t acceptTime_;
};


}} // namespace gideon { namespace zoneserver {