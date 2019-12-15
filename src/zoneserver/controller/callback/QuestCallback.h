#pragma once

#include <gideon/cs/shared/data/QuestInfo.h>
#include <gideon/cs/shared/data/Money.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class QuestCallback
 */
class QuestCallback
{
public:
    virtual ~QuestCallback() {}

public:
	virtual void accepted(QuestCode questCode) = 0;
    virtual void accepted(QuestCode questCode, sec_t repeatAcceptTime) = 0;
	virtual void canceled(QuestCode questCode, bool isRepeatQuest) = 0;
	virtual void completed(QuestCode questCode, bool isRepeatQuest) = 0;
	virtual void missionUpdated(QuestCode questCode, QuestMissionCode missionCode,
		const QuestGoalInfo& goalInfo, bool isRepeatQuest) = 0;

    
	virtual void completeQuestRemoved(QuestCode questCode) = 0;

    virtual void questToPartyResult(const GameObjectInfo& targetMember, QuestCode questCode,
        QuestToPartyResultType resultType) = 0;
    virtual void questToPartyPushed(const GameObjectInfo& targetMember, QuestCode questCode) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
