#pragma once

#include "AcceptedQuest.h"

namespace gideon { namespace zoneserver {

/***
 * @class AcceptedRepeatQuest
 ***/
class AcceptedRepeatQuest : public AcceptedQuest
{
public:
	AcceptedRepeatQuest(go::Entity& owner, const datatable::QuestTemplate& questTemplate,
		const AcceptedQuestInfo& acceptQuestInfo, sec_t acceptTime);

private:
    // = AcceptedQuest overriding
    virtual sec_t getAcceptTime() const {
        return acceptTime_;
    }

private:
    sec_t acceptTime_;
};


}} // namespace gideon { namespace zoneserver {