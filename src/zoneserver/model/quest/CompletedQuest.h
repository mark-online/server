#pragma once

#include "Quest.h"

namespace gideon { namespace zoneserver {

/***
 * @class CompletedQuest
 * 미션을 완료하고 보상까지 받은 퀘스트
 ***/
class CompletedQuest : public Quest
{
public:
	CompletedQuest(QuestCode questCode) : 
		questCode_(questCode) {}

public:
	QuestCode getQuestCode() const {
		return questCode_;
	}

public:
    virtual bool isEnoughAcceptTime() const {
        return false;
    }

private:
	QuestCode questCode_;
};


}} // namespace gideon { namespace zoneserver {