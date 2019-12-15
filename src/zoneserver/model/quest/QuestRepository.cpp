#include "ZoneServerPCH.h"
#include "QuestRepository.h"
#include "CompletedQuest.h"
#include "CompletedRepeatQuest.h"
#include "AcceptedQuest.h"
#include "AcceptedRepeatQuest.h"
#include "../gameobject/Creature.h"
#include "../gameobject/ability/Knowable.h"
#include "../gameobject/ability/Inventoryable.h"
#include "../gameobject/ability/Moneyable.h"
#include "../gameobject/ability/Chaoable.h"
#include "../gameobject/ability/Guildable.h"
#include "../gameobject/ability/CharacterClassable.h"
#include "../gameobject/ability/Partyable.h"
#include "../gameobject/ability/Achievementable.h"
#include "../item/Inventory.h"
#include "../item/QuestInventory.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/QuestCallback.h"
#include "../../ai/evt/EventTriggerManager.h"
#include "../../service/item/ItemIdGenerator.h"
#include "../../service/distance/DistanceChecker.h"
#include "../../service/party/Party.h"
#include <gideon/cs/datatable/QuestTable.h>
#include <gideon/cs/datatable/QuestItemTable.h>
#include <gideon/cs/datatable/QuestTransportMissionTable.h>
#include <gideon/cs/datatable/QuestActivationMissionTable.h>
#include <gideon/cs/datatable/QuestProbeMissionTable.h>

namespace gideon { namespace zoneserver {

namespace {

std::unique_ptr<AcceptedQuest> createAcceptedQuest(go::Entity& owner,
    const datatable::QuestTemplate& questTemplate)
{
    if (questTemplate.isRepeatQuest()) {
        return std::make_unique<AcceptedRepeatQuest>(owner, questTemplate,
            AcceptedQuestInfo(questTemplate.getQuestCode()), getTime());
    }
    return std::make_unique<AcceptedQuest>(owner, questTemplate,
        AcceptedQuestInfo(questTemplate.getQuestCode()));
}

} // namespace {

// = QuestRepository


QuestRepository::QuestRepository(go::Entity& owner,	QuestCodes& completeQuests,
	AcceptedQuestInfos& acceptQuestInfos, AcceptedRepeatQuestInfos& repeatAcceptInfos,
    CompletedRepeatQuestInfos& completeRepeatQuests) :
	owner_(owner),
	completedQuests_(completeQuests)
{
	initialize(acceptQuestInfos, repeatAcceptInfos, completeRepeatQuests);
}


QuestRepository::~QuestRepository()
{
    for (const AcceptedQuestMap::value_type& value : acceptedQuestMap_) {
        const AcceptedQuest* quest = value.second;
        boost::checked_delete(quest);
    }

    for (const CompletedQuestMap::value_type& value : completedQuestMap_) {
        const CompletedQuest* quest = value.second;
        boost::checked_delete(quest);
    }
}


ErrorCode QuestRepository::acceptQuest(QuestCode questCode, const GameObjectInfo& objectInfo)
{
	const datatable::QuestTemplate* questTemplate = 
		QUEST_TABLE->getQuestTemplate(questCode);
	if (! questTemplate) {
		return ecQuestInvalidQuest;
	}

    if (maxAcceptQuestCount <= acceptedQuestMap_.size()) {
        return ecQuestMaxOverAcceptCount;
    }

    if (questTemplate->isRepeatQuest()) {
        return acceptRepeatQuest(objectInfo, *questTemplate);
    }

    return acceptQuest(objectInfo, *questTemplate);
}


ErrorCode QuestRepository::cheatAcceptQuest(QuestCode questCode)
{
	const datatable::QuestTemplate* questTemplate = 
		QUEST_TABLE->getQuestTemplate(questCode);
	if (! questTemplate) {
		return ecQuestInvalidQuest;
	}

	if (maxAcceptQuestCount <= acceptedQuestMap_.size()) {
		return ecQuestMaxOverAcceptCount;
	}

	if (questTemplate->isRepeatQuest()) {
		return ecServerInternalError;
	}

    {
        std::lock_guard<LockType> lock(lock_);

        if (isCompleted_i(questTemplate->getQuestCode())) {
            return ecQuestAlreadyCompleteQuest;
        }

        if (isAccepted_i(questTemplate->getQuestCode())) {
            return ecQuestAlreayAcceptedQuest;
        }

        return acceptQuest_i(GameObjectInfo(), *questTemplate, true);
    }
}


ErrorCode QuestRepository::cheatRemoveCompleteQuest(QuestCode questCode)
{
    std::lock_guard<LockType> lock(lock_);

	CompletedQuest* quest =  getCompletedQuest(questCode);
	if (! quest) {
		return ecQuestNotCompleteQuest;
	}
	delete quest;
	completedQuestMap_.erase(questCode);
	owner_.getController().queryQuestCallback()->completeQuestRemoved(questCode);
	return ecOk;
}


ErrorCode QuestRepository::cheatClearAcceptQuest(QuestCode questCode)
{
	bool isRepeatQuest = false;
	{
        std::lock_guard<LockType> lock(lock_);

		if (isCompleted_i(questCode)) {
			return ecQuestAlreadyCompleteQuest;
		}

		AcceptedQuest* quest = getAcceptedQuest(questCode);
		if (! quest) {
			return ecQuestNotFindQuest;
		}

		isRepeatQuest = quest->isRepeatQuest();
		const sec_t acceptTime = quest->getAcceptTime();
		deleteAcceptedQuest(questCode);

		if (isRepeatQuest) {
			CompletedQuest* completedQuest = new CompletedRepeatQuest(questCode, acceptTime);
			completedQuestMap_.emplace(questCode, completedQuest);
		}
		else {
			CompletedQuest* completedQuest = new CompletedQuest(questCode);
			completedQuestMap_.emplace(questCode, completedQuest);
            completedQuests_.insert(questCode);
		}

	}
	
	gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
	if (callback) {
		callback->completed(questCode, isRepeatQuest);
	}

	return ecOk;
}


ErrorCode QuestRepository::cancelQuest(QuestCode questCode)
{
    bool isRepeatQuest = false;
    {
        std::lock_guard<LockType> lock(lock_);

        AcceptedQuest* quest = getAcceptedQuest(questCode);
        if (! quest) {
            return ecQuestNotFindQuest;
        }

        isRepeatQuest = quest->isRepeatQuest();
        quest->cancel();
        deleteAcceptedQuest(questCode);
    }


	gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
	if (callback) {
		callback->canceled(questCode, isRepeatQuest);
	}
	return ecOk;
}


ErrorCode QuestRepository::completeTransportMission(QuestCode questCode, QuestMissionCode missionCode)
{
    std::lock_guard<LockType> lock(lock_);

    if (isCompleted_i(questCode)) {
        return ecQuestAlreadyCompleteQuest;
    }

    AcceptedQuest* quest = getAcceptedQuest(questCode);
    if (! quest) {
        return ecQuestNotFindQuest;
    }

    const datatable::QuestTransportMissionTemplate* missionTemplate =
        QUEST_TRANSPORT_MISSION_TABLE->getQuestTransportMissionTemplate(missionCode);
    if (! missionTemplate) {
        return ecQuestInvalidMission;
    }

    const datatable::QuestMissionTransportGoalInfo& goalInfo = 
        missionTemplate->getQuestMissionTransportGoalInfo();
    
    // TODO, 위치및 거리 체크
    if (! quest->transfer(missionCode, goalInfo.questItemCode_)) {
        return ecQuestAlreadyCompleteQuest;
    }

    return ecOk;
}


ErrorCode QuestRepository::completeProbeMission(QuestCode questCode, QuestMissionCode missionCode)
{
    assert(isProbeMission(getQuestMissionType(missionCode)));

    std::lock_guard<LockType> lock(lock_);

    if (isCompleted_i(questCode)) {
        return ecQuestAlreadyCompleteQuest;
    }

    AcceptedQuest* quest = getAcceptedQuest(questCode);
    if (! quest) {
        return ecQuestNotFindQuest;
    }

    const datatable::QuestProbeMissionTemplate* missionTemplate =
        QUEST_PROBE_MISSION_TABLE->getQuestProbeMission(missionCode);
    if (! missionTemplate) {
        return ecQuestInvalidMission;
    }

    if (! quest->probe(missionCode, missionTemplate->getMissionCode())) {
        return ecQuestAlreadyProbMission;
    }

    return ecOk;
}


ErrorCode QuestRepository::completeQuest(QuestCode questCode, ObjectId npcId, DataCode selectItemCode)
{
    const datatable::QuestTemplate* questTemplate = 
        QUEST_TABLE->getQuestTemplate(questCode);
    if (! questTemplate) {
        return ecQuestInvalidQuest;
    }

    if (isValidNpcCode(questTemplate->getCompleteNpcCode())) {
        go::Entity* entity =
            owner_.queryKnowable()->getEntity(GameObjectInfo(otNpc, npcId));
        if (! entity) {
            return ecQuestInvalidNpcInfo;
        }

        if (entity->getEntityCode() != questTemplate->getCompleteNpcCode()) {
            return ecQuestInvalidNpcInfo;
        }
    }

    bool isRepeatQuest = false;
    {
        std::lock_guard<LockType> lock(lock_);

        if (! questTemplate->isRepeatQuest()) {
            if (isCompleted_i(questCode)) {
                return ecQuestAlreadyCompleteQuest;
            }
        }
        else {
            CompletedQuest* complateQuest = getCompletedQuest(questCode);
            if (complateQuest && ! complateQuest->isEnoughAcceptTime()) {
                return ecQuestAlreadyCompleteQuest;
            }
        }
        

        AcceptedQuest* quest = getAcceptedQuest(questCode);
        if (! quest) {
            return ecQuestNotFindQuest;
        }

        if (! quest->isCompleted()) {
            return ecQuestNotCompleteQuest;
        }

        const ErrorCode errorCode = quest->reward(selectItemCode);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        isRepeatQuest = quest->isRepeatQuest();
        const sec_t acceptTime = quest->getAcceptTime();
        deleteAcceptedQuest(questCode);

        if (isRepeatQuest) {
            CompletedQuest* completedQuest = new CompletedRepeatQuest(questCode, acceptTime);
            completedQuestMap_.emplace(questCode, completedQuest);
        }
        else {
            CompletedQuest* completedQuest = new CompletedQuest(questCode);
            completedQuestMap_.emplace(questCode, completedQuest);
            completedQuests_.insert(questCode);
        }
    }

    gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
    if (callback) {
        callback->completed(questCode, isRepeatQuest);
    }

    if (! isRepeatQuest) {
        owner_.queryAchievementable()->updateAchievement(acmtQuest, nullptr);
    }
	return ecOk;
}


ErrorCode QuestRepository::completeTransportQuest(QuestCode questCode, QuestMissionCode missionCode, ObjectId npcId, DataCode selectItemCode)
{
    const ErrorCode errorCode = completeTransportMission(questCode, missionCode);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    return completeQuest(questCode, npcId, selectItemCode);
}


ErrorCode QuestRepository::checkActivationMission(DataCode dataCode)
{
    std::lock_guard<LockType> lock(lock_);

    AcceptedQuest* theQuest = nullptr;
    QuestMissionCode missionCode = invalidQuestMissionCode;
    for (const AcceptedQuestMap::value_type& value : acceptedQuestMap_) {
        AcceptedQuest* quest = value.second;
        missionCode = quest->getUncompletedActivationMissionCodeBy(dataCode);
        if (isValidQuestMissionCode(missionCode)) {
            theQuest = quest;
            break;
        }
    }

    if (! theQuest) {
        return ecQuestNotFindQuest;
    }

    return theQuest->checkActivate(missionCode, dataCode);
}


ErrorCode QuestRepository::completeActivationMission(DataCode dataCode)
{
    QuestMissionCode missionCode = invalidQuestMissionCode;
    bool isMissionCompleted = false;
    {
        std::lock_guard<LockType> lock(lock_);

        AcceptedQuest* theQuest = nullptr;
        for (const AcceptedQuestMap::value_type& value : acceptedQuestMap_) {
            AcceptedQuest* quest = value.second;
            missionCode = quest->getUncompletedActivationMissionCodeBy(dataCode);
            if (isValidQuestMissionCode(missionCode)) {
                theQuest = quest;
                break;
            }
        }

        if (! theQuest) {
            return ecQuestNotFindQuest;
        }

        const ErrorCode errorCode = theQuest->activate(missionCode, dataCode);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        isMissionCompleted = theQuest->isMissionCompleted(missionCode);
    }

    if (isMissionCompleted) {
        EVT_MANAGER.trigger(ai::evt::evtActivationMissionCompleted, owner_, missionCode);
    }
    return ecOk;
}


void QuestRepository::killed(ObjectType objectType, DataCode dataCode, Chaotic chaotic)
{
    typedef sne::core::Vector<QuestMissionCode> MissionCodes;
    MissionCodes completedMissions;
    {
        std::lock_guard<LockType> lock(lock_);

        AcceptedQuestMap::iterator pos = acceptedKillQuests_.begin();
        while (pos != acceptedKillQuests_.end()) {
            AcceptedQuest* quest = (*pos).second;
	        const QuestMissionCode completedMissionCode = quest->kill(objectType, dataCode, chaotic);
            if (isValidQuestMissionCode(completedMissionCode)) {
                completedMissions.push_back(completedMissionCode);
            }
	        if (quest->isCompleted()) {
                acceptedKillQuests_.erase(pos++);
	        }
            else {
                ++pos;
            }
        }
    }

    for (QuestMissionCode missionCode : completedMissions) {
        EVT_MANAGER.trigger(ai::evt::evtKillMissionCompleted, owner_, missionCode);
    }
}


void QuestRepository::doContents(QuestContentMissionType type, DataCode dataCode)
{
    std::lock_guard<LockType> lock(lock_);

	for (AcceptedQuestMap::value_type& value : acceptedQuestMap_) {
		AcceptedQuest* quest = value.second;
		quest->doContents(type, dataCode);
	}
}


ErrorCode QuestRepository::pushQuestToParty(QuestCode questCode) const
{
    const datatable::QuestTemplate* questTemplate = 
        QUEST_TABLE->getQuestTemplate(questCode);
    if (! questTemplate) {
        return ecQuestNotFindQuest;
    }

    if (! questTemplate->isShareable()) {
        return ecQuestNotShareable;
    }

    {
        const bool hasQuest = isCompleted_i(questCode) || isAccepted_i(questCode);
        if (! hasQuest) {
            return ecQuestNotFindQuest;
        }
    }

    PartyPtr party = owner_.queryPartyable()->getParty();
    if (! party.get()) {
        return ecQuestInNotPartyMember;
    }

    party->notifyPushQuest(owner_, questCode);

    return ecOk;
}

ErrorCode QuestRepository::canAddItem(QuestCode& questCode, QuestItemCode questItemCode) const
{
    std::lock_guard<LockType> lock(lock_);

    for (AcceptedQuestMap::value_type value : acceptedQuestMap_) {
		AcceptedQuest* quest = value.second;
		const BaseItemInfo* missionItem = quest->getHasQuestMissionItemInfo(questItemCode);
		if (! missionItem) {
			continue;
		}

		questCode = quest->getQuestCode();
		const QuestItemInfo* questItemInfo = 
			owner_.queryInventoryable()->getQuestInventory().getItemInfoByCode(questItemCode);
		if (questItemInfo) {
			if (questItemInfo->stackCount_ >= missionItem->count_) {
				return ecQuestAlreadyEnoughObtainQuestItem;
			}
		}
		return ecOk;		
	}

	return ecQuestNotExistNeedObtainQuest;
}



bool QuestRepository::isAccepted(QuestCode questCode) const
{
    std::lock_guard<LockType> lock(lock_);

    return isAccepted_i(questCode);
}


bool QuestRepository::isCompleted(QuestCode questCode) const
{
    std::lock_guard<LockType> lock(lock_);

    return isCompleted_i(questCode);
}


QuestToPartyResultType QuestRepository::getAcceptConditionResult(QuestCode questCode) const
{
    {
        std::lock_guard<LockType> lock(lock_);

        if (isAccepted_i(questCode)) {
            return qtprtAlreadyAcceptQuest;
        }

        if (isCompleted_i(questCode)) {
            return qtprtCompleteQuest;
        }
    }

    const datatable::QuestTemplate* questTemplate =  QUEST_TABLE->getQuestTemplate(questCode);
    if (! questTemplate) {
        return qtprtUnknown;
    }

    if (questTemplate->getIssueLevel() != clInvalidLevel && 
        questTemplate->getIssueLevel() > static_cast<go::Creature&>(owner_).getCreatureLevel()) {
            return qtprtNotEnoughLevel;
    }

    for (const datatable::IssueSpecialConditionInfo& info : questTemplate->getIssueSpecialConditionInfos()) {
        const ErrorCode errorCode = checkSpecialCondition(info.condition_, info.info_, info.value_);
        if (isFailed(errorCode)) {
            return qtprtNotAcceptCondtion;
        }            
    }

    return qtprtOk;
}

void QuestRepository::initialize(AcceptedQuestInfos& acceptQuestInfos,
    AcceptedRepeatQuestInfos& repeatAcceptInfos,
    CompletedRepeatQuestInfos& completeRepeatQuests)
{
	for (QuestCode code : completedQuests_) {
        CompletedQuest* quest = new CompletedQuest(code);
		completedQuestMap_.emplace(code, quest);
	}

    for (const CompletedRepeatQuestInfo& completeRepeatQuest : completeRepeatQuests) {
        CompletedQuest* quest = new CompletedRepeatQuest(completeRepeatQuest.questCode_, completeRepeatQuest.acceptTime_);
        completedQuestMap_.emplace(completeRepeatQuest.questCode_, quest);
    }

	for (AcceptedQuestInfo& acceptQuestInfo : acceptQuestInfos) {
		const datatable::QuestTemplate* questTemplate = 
			QUEST_TABLE->getQuestTemplate(acceptQuestInfo.questCode_);
		if (! questTemplate) {
			assert(questTemplate);
			continue;
		}

        AcceptedQuest* quest = new AcceptedQuest(owner_, *questTemplate, acceptQuestInfo);
		acceptedQuestMap_.emplace(acceptQuestInfo.questCode_, quest);

		for (QuestMissionCode missionCode : questTemplate->getMissionCodes()) {
			if (getQuestMissionType(missionCode) == qmtKill) {
				if (acceptedKillQuests_.find(acceptQuestInfo.questCode_) == acceptedKillQuests_.end()) {
                    acceptedKillQuests_.emplace(acceptQuestInfo.questCode_, quest);
                    break;
				}
			}
		}
	}

    for (AcceptedRepeatQuestInfo& acceptQuestInfo : repeatAcceptInfos) {
        const datatable::QuestTemplate* questTemplate = 
            QUEST_TABLE->getQuestTemplate(acceptQuestInfo.questCode_);
        if (! questTemplate) {
            assert(questTemplate);
            continue;
        }

        AcceptedQuest* quest = new AcceptedRepeatQuest(owner_, *questTemplate, acceptQuestInfo,
            acceptQuestInfo.acceptTime_);
        acceptedQuestMap_.emplace(acceptQuestInfo.questCode_, quest);

        for (QuestMissionCode missionCode : questTemplate->getMissionCodes()) {
            if (getQuestMissionType(missionCode) == qmtKill) {
                if (acceptedKillQuests_.find(acceptQuestInfo.questCode_) == acceptedKillQuests_.end()) {
                    acceptedKillQuests_.emplace(acceptQuestInfo.questCode_, quest);
                    break;
                }
            }
        }
    }
}


ErrorCode QuestRepository::acceptQuest(const GameObjectInfo& objectInfo,
    const datatable::QuestTemplate& questTemplate)
{
    ErrorCode errorCode = ecOk;
    {
        std::lock_guard<LockType> lock(lock_);

        if (isCompleted_i(questTemplate.getQuestCode())) {
            return ecQuestAlreadyCompleteQuest;
        }

        if (isAccepted_i(questTemplate.getQuestCode())) {
            return ecQuestAlreayAcceptedQuest;
        }

        if (questTemplate.getIssueLevel() != clInvalidLevel && 
            questTemplate.getIssueLevel() > static_cast<go::Creature&>(owner_).getCreatureLevel()) {
            return ecQuestInvalidCharacterLevel;
        }

        errorCode = acceptQuest_i(objectInfo, questTemplate);
    }

    if (isSucceeded(errorCode)) {
        gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
        if (callback) {
            callback->accepted(questTemplate.getQuestCode());
        }
    }
	
    return errorCode;
}


ErrorCode QuestRepository::acceptRepeatQuest(const GameObjectInfo& objectInfo,
    const datatable::QuestTemplate& questTemplate)
{
    ErrorCode errorCode = ecOk;
    {
        std::lock_guard<LockType> lock(lock_);

        if (isAccepted_i(questTemplate.getQuestCode())) {
            return ecQuestAlreayAcceptedQuest;
        }

        const CompletedQuest* quest = getCompletedQuest(questTemplate.getQuestCode());
        if (quest != nullptr) {
            if (! quest->isEnoughAcceptTime()) {
                return ecQuestNotEnoughAcceptTime;
            }
        }

        errorCode = acceptQuest_i(objectInfo, questTemplate);
        if (isSucceeded(errorCode)) {
            completedQuestMap_.erase(questTemplate.getQuestCode());
        }
    }

    if (isSucceeded(errorCode)) {
        gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
        if (callback) {
            callback->accepted(questTemplate.getQuestCode(), getTime());
        }
    }
    return errorCode;
}


ErrorCode QuestRepository::acceptQuest_i(const GameObjectInfo& objectInfo,
	const datatable::QuestTemplate& questTemplate, bool isSkipSpecialCheck)
{
    if (objectInfo.isValid()) {
    // TODO: 거리 검사
    
	/*if (questTemplate->getIssueCondition() != qicNpc) {
		const ErrorCode errorCode = checkDistance(*questTemplate, objectInfo);
		if (isFailed(errorCode)) {
			return errorCode;
		}
	}*/
    }
    
    bool shouldRemoveItem = false;
	if (! isSkipSpecialCheck) {
		for (const datatable::IssueSpecialConditionInfo& info : questTemplate.getIssueSpecialConditionInfos()) {
			const ErrorCode errorCode = checkSpecialCondition(info.condition_, info.info_, info.value_);
			if (isFailed(errorCode)) {
				return errorCode;
			}
			if (info.shoudRemoveItem()) {
				shouldRemoveItem = true;
			}
		}
	}
    

    if (shouldRemoveItem) {
        for (const datatable::IssueSpecialConditionInfo& info : questTemplate.getIssueSpecialConditionInfos()) {
            if (info.condition_ == qiscRemoveInvenItem) {
                const BaseItemInfo baseItemInfo(info.info_, uint8_t(info.value_));                
                owner_.queryInventoryable()->getInventory().useItemsByBaseItemInfo(baseItemInfo);
            }
            else if (info.condition_ == qiscRemoveQuestItem) {                
                owner_.queryInventoryable()->getQuestInventory().removeItemsByItemCode(info.info_, uint8_t(info.value_));
            }         
        }
    }

    const QuestCode questCode = questTemplate.getQuestCode();
    std::unique_ptr<AcceptedQuest> quest = createAcceptedQuest(owner_, questTemplate);

    for (const QuestMissionCode missionCode : questTemplate.getMissionCodes()) {
        const QuestMissionType missionType = getQuestMissionType(missionCode);
        if (missionType == qmtKill) {
            if (acceptedKillQuests_.find(quest->getQuestCode()) == acceptedKillQuests_.end()) {
                acceptedKillQuests_.emplace(quest->getQuestCode(), quest.get());
            }
        }
        else if (missionType == qmtTrasport) {
            const datatable::QuestTransportMissionTemplate* missionTemplate =
                QUEST_TRANSPORT_MISSION_TABLE->getQuestTransportMissionTemplate(missionCode);
            assert(missionTemplate != nullptr);
            const datatable::QuestMissionTransportGoalInfo& goalInfo = 
                missionTemplate->getQuestMissionTransportGoalInfo();
            const datatable::QuestItemTemplate* itemTemplate = 
                QUEST_ITEM_TABLE->getQuestItemTemplate(goalInfo.questItemCode_);
            assert(itemTemplate != nullptr);

            const QuestItemInfo questItem(ITEM_ID_GENERATOR->generateItemId(),
                itemTemplate->getQuestItemCode(), itemTemplate->getUsableCount(), 1, questCode);
            if (! owner_.queryInventoryable()->getQuestInventory().addItem(questItem)) {
                SNE_LOG_ERROR("Quest Item Add Failed(ID:%" PRIu64 ", QuestCode:%u, QuestItemCode%u)",
                    questItem.itemId_, questItem.questCode_, questItem.questItemCode_);
                return ecServerInternalError;
            }
        }
        else if (missionType == qmtActivation) {
            const datatable::QuestActivationMissionTemplate* missionTemplate =
                QUEST_ACTIVATION_MISSION_TABLE->getQuestActivationMissionTemplate(missionCode);
            assert(missionTemplate != nullptr);
            const datatable::QuestActivationGoalInfo& goalInfo = missionTemplate->getGoalInfo();
            const datatable::QuestItemTemplate* itemTemplate = 
                QUEST_ITEM_TABLE->getQuestItemTemplate(goalInfo.goalCode_);
            if (itemTemplate != nullptr) {
                const QuestItemInfo questItem(ITEM_ID_GENERATOR->generateItemId(),
                    itemTemplate->getQuestItemCode(), itemTemplate->getUsableCount(), 1, questCode);
                if (! owner_.queryInventoryable()->getQuestInventory().addItem(questItem)) {
                    SNE_LOG_ERROR("Quest Item Add Failed(ID:%" PRIu64 ", QuestCode:%u, QuestItemCode%u)",
                        questItem.itemId_, questItem.questCode_, questItem.questItemCode_);
                    return ecServerInternalError;
                }
            }
        }
    }
    acceptedQuestMap_.emplace(questCode, quest.release());

    return ecOk;
}


void QuestRepository::deleteAcceptedQuest(QuestCode questCode)
{
    const AcceptedQuestMap::const_iterator pos = acceptedQuestMap_.find(questCode);
    if (pos != acceptedQuestMap_.end()) {
        AcceptedQuest* quest = (*pos).second;
        boost::checked_delete(quest);
        acceptedQuestMap_.erase(pos);
    }
	acceptedKillQuests_.erase(questCode);
}


ErrorCode QuestRepository::checkDistance(const datatable::QuestTemplate& questTemplate,
	const GameObjectInfo& objectInfo) const
{		
	go::Entity* target = owner_.queryKnowable()->getEntity(objectInfo);
	if (target == nullptr) {
		return ecQuestTargetNotFind;
	}

	const UnionEntityInfo& entityInfo = target->getUnionEntityInfo();
	if (entityInfo.asNpcInfo().npcCode_ != questTemplate.getIssConditionCode()) {
		return ecQuestInvalidNpcInfo;
	}

	if (! DISTANCE_CHECKER->checkNpcDistance(owner_.getPosition(), target->getPosition())) {
		return ecQuestTargetTooFar;
	}
	
	return ecOk;
}



ErrorCode QuestRepository::checkSpecialCondition(
    QuestIssueSpecialCondition condition, uint32_t info, uint32_t value) const
{
    if (condition == qiscNone) {
        return ecOk;
    }
    else if (condition == qiscPreQuest) {
        if (! isCompleted_i(static_cast<QuestCode>(info))) {
            return ecQuestNotFindPreQuest;
        }
        if (toCreatureLevel(value) > static_cast<go::Creature&>(owner_).getCreatureLevel()) {
            return ecQuestNotEnoughLevel;
        }
    }
    else if (condition == qiscChaotic) {
        if (toChaotic(info) > owner_.queryChaoable()->getChaotic()) {
            return  ecQuestNotEnoughCaotic;
        }
    }
    else if (condition == qiscLawful) {

    }
    else if (condition == qiscGameMoney) {
        if (info > owner_.queryMoneyable()->getGameMoney()) {
            return ecQuestNotEnoughGameMoney;
        }
    }
    else if (condition == qiscQuestPoint) {

    }
    else if (condition == qiscKeyword) {

    }
    else if (condition == qiscItem) {
        if (! owner_.queryInventoryable()->getInventory().checkHasItemsByBaseItemInfo(BaseItemInfo(info, uint8_t(value)))) {
            return ecItemNotUseItem;
        }
    }
    else if (condition == qiscQuestItem) {
        if (! owner_.queryInventoryable()->getQuestInventory().checkHasQuestItem(info, uint8_t(value))) {
            return ecItemNotUseItem;
        }
    }
    else if (condition == qiscTime) {
        
    }
    else if (condition == qiscLevel) {
        if (toCreatureLevel(info) > static_cast<go::Creature&>(owner_).getCreatureLevel()) {
            return ecQuestNotEnoughLevel;
        }
    }
    else if (condition == qiscCharacterClass) {
        if (owner_.queryCharacterClassable()->getCharacterClass() != toCharacterClass(info)) {
            return ecQuestInvalidCharacterClass;
        }
    }
    else if (condition == qiscCharacterClassAndLevel) {
        if (owner_.queryCharacterClassable()->getCharacterClass() != toCharacterClass(info)) {
            return ecQuestInvalidCharacterClass;
        }
        if (toCreatureLevel(value) > static_cast<go::Creature&>(owner_).getCreatureLevel()) {
            return ecQuestNotEnoughLevel;
        }
    }
    else if (condition == qiscRemoveInvenItem) {
        const BaseItemInfo baseItemInfo(info, uint8_t(value));
        if (! owner_.queryInventoryable()->getInventory().checkHasItemsByBaseItemInfo(baseItemInfo)) {
            return ecItemNotUseItem;
        }
    }
    else if (condition == qiscRemoveQuestItem) {
        if (! owner_.queryInventoryable()->getQuestInventory().checkHasQuestItem(info, uint8_t(value))) {
            return ecItemNotUseItem;
        }
    }
    else {
        assert(false);
    }
    return ecOk;
}

}} // namespace gideon { namespace zoneserver {
