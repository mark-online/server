#pragma once

#include "Quest.h"
#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/QuestInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace datatable {
class QuestTemplate;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

class Quest;
class AcceptedQuest;
class CompletedQuest;

/**
 * @class QuestRepository
 */
class QuestRepository
{
    typedef std::mutex LockType;
    // TODO: AcceptedQuest 메모리 관리 개선
	typedef sne::core::HashMap<QuestCode, AcceptedQuest*> AcceptedQuestMap;
    // TODO: CompletedQuest 메모리 관리 개선
    typedef sne::core::HashMap<QuestCode, CompletedQuest*> CompletedQuestMap;
	typedef sne::core::Vector<QuestCode> QuestCodeVector;

public:
	QuestRepository(go::Entity& owner, 	QuestCodes& completeQuests,
        AcceptedQuestInfos& acceptQuestInfos, AcceptedRepeatQuestInfos& repeatAcceptInfos,
        CompletedRepeatQuestInfos& completeRepeatQuests);
	~QuestRepository();

public:
	ErrorCode acceptQuest(QuestCode questCode, const GameObjectInfo& objectInfo);
	ErrorCode cheatAcceptQuest(QuestCode questCode);
	ErrorCode cheatRemoveCompleteQuest(QuestCode questCode);
	ErrorCode cheatClearAcceptQuest(QuestCode questCode);
	ErrorCode cancelQuest(QuestCode questCode); 

    ErrorCode completeTransportMission(QuestCode questCode, QuestMissionCode missionCode);
    ErrorCode completeProbeMission(QuestCode questCode, QuestMissionCode missionCode);

    ErrorCode checkActivationMission(DataCode dataCode);
    ErrorCode completeActivationMission(DataCode dataCode);

    ErrorCode completeQuest(QuestCode questCode, ObjectId npcId, DataCode selectItemCode);
    ErrorCode completeTransportQuest(QuestCode questCode, QuestMissionCode missionCode, ObjectId npcId, DataCode selectItemCode);

    size_t getCompleteQuestCount() const {
        return completedQuests_.size();
    }

public:
	void killed(ObjectType objectType, DataCode dataCode, Chaotic chaotic);
	ErrorCode canAddItem(QuestCode& questCode, QuestItemCode questItemCode) const;
	void doContents(QuestContentMissionType type, DataCode dataCode);
    ErrorCode pushQuestToParty(QuestCode questCode) const;

public:
    bool isAccepted(QuestCode questCode) const;
    bool isCompleted(QuestCode questCode) const;
    QuestToPartyResultType getAcceptConditionResult(QuestCode questCode) const;

private:
    ErrorCode acceptQuest(const GameObjectInfo& objectInfo, const datatable::QuestTemplate& questTemplate);
    ErrorCode acceptRepeatQuest(const GameObjectInfo& objectInfo, const datatable::QuestTemplate& questTemplate);
    ErrorCode acceptQuest_i(const GameObjectInfo& objectInfo, const datatable::QuestTemplate& questTemplate, bool isSkipSpecialCheck = false);

    void deleteAcceptedQuest(QuestCode questCode);
    void initialize(AcceptedQuestInfos& acceptQuestInfos, AcceptedRepeatQuestInfos& repeatAcceptInfos,
        CompletedRepeatQuestInfos& completeRepeatQuests);

    ErrorCode checkDistance(const datatable::QuestTemplate& questTemplate,
        const GameObjectInfo& objectInfo) const;
    ErrorCode checkSpecialCondition(QuestIssueSpecialCondition condition,
        uint32_t info, uint32_t value) const;

private:
    bool isAccepted_i(QuestCode questCode) const {
        return getAcceptedQuest(questCode) != nullptr;
    }

    bool isCompleted_i(QuestCode questCode) const {
        return completedQuestMap_.find(questCode) != completedQuestMap_.end();
    }

    const AcceptedQuest* getAcceptedQuest(QuestCode questCode) const {
        const AcceptedQuestMap::const_iterator pos = acceptedQuestMap_.find(questCode);
        if (pos != acceptedQuestMap_.end()) {
            return (*pos).second;
        }
        return nullptr;
    }

    AcceptedQuest* getAcceptedQuest(QuestCode questCode) {
        const AcceptedQuestMap::iterator pos = acceptedQuestMap_.find(questCode);
        if (pos != acceptedQuestMap_.end()) {
            return (*pos).second;
        }
        return nullptr;
    }

    const CompletedQuest* getCompletedQuest(QuestCode questCode) const {
        const CompletedQuestMap::const_iterator pos = completedQuestMap_.find(questCode);
        if (pos != completedQuestMap_.end()) {
            return (*pos).second;
        }
        return nullptr;
    }

	CompletedQuest* getCompletedQuest(QuestCode questCode) {
		CompletedQuestMap::iterator pos = completedQuestMap_.find(questCode);
		if (pos != completedQuestMap_.end()) {
			return (*pos).second;
		}
		return nullptr;
	}


public:
    mutable LockType lock_;

	go::Entity& owner_;
	QuestCodes& completedQuests_;

	CompletedQuestMap completedQuestMap_;
	AcceptedQuestMap acceptedQuestMap_;
	AcceptedQuestMap acceptedKillQuests_;
};

}} // namespace gideon { namespace zoneserver {
