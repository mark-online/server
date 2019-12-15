#pragma once

#include "Quest.h"
#include <gideon/cs/shared/data/ItemInfo.h>

namespace gideon { namespace datatable {
class QuestTemplate;
struct QuestMissionKillGoalInfo;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go {

/**
 * @class AcceptedQuest
 * 수락한 퀘스트
 */
class AcceptedQuest : public Quest
{
    typedef sne::core::HashMap<DeviceCode, QuestMissionCode> ActivationMissionMap;
	typedef sne::core::HashMap<QuestItemCode, BaseItemInfo> ObtainMissionMap;

public:
	AcceptedQuest(go::Entity& owner, const datatable::QuestTemplate& questTemplate,
		const AcceptedQuestInfo& questInfo);

public:
    /// @return 완료된 미션 코드
	QuestMissionCode kill(ObjectType objectType, DataCode code, Chaotic chaotic);
	bool transfer(QuestMissionCode missionCode, QuestItemCode itemCode);
    bool probe(QuestMissionCode missionCode, RegionCode regionCode);
    ErrorCode activate(QuestMissionCode missionCode, DataCode dataCode);
	void doContents(QuestContentMissionType type, DataCode dataCode);
    void cancel();

	ErrorCode reward(DataCode selectItemCode);

    ErrorCode checkActivate(QuestMissionCode missionCode, DataCode dataCode);

public:
    QuestMissionCode getUncompletedActivationMissionCodeBy(DataCode dataCode) const;
	const BaseItemInfo* getHasQuestMissionItemInfo(QuestItemCode questItemCode) const;

public:
    QuestCode getQuestCode() const;

    bool isCompleted() const;
    bool isRepeatQuest() const;

    bool isMissionCompleted(QuestMissionCode missionCode) const;

public:
    virtual sec_t getAcceptTime() const {
        return 0;
    }

private:
	void addMissionInfo(QuestMissionCode missionCode, DataCode dataCode);
	void updateKillMissionInfo(QuestGoalInfos& infos,
        const datatable::QuestMissionKillGoalInfo& killGoalInfo,
        QuestMissionCode missionCode, DataCode dataCode);

	ErrorCode rewardItem(const BaseItemInfos& itemInfos);
    ErrorCode rewardItem(const BaseItemInfo& itemInfo);

	ErrorCode rewardQuestItem(const BaseItemInfos& itemInfos);
	ErrorCode rewardQuestItem(const BaseItemInfo& itemInfo);


	ErrorCode rewardUpgradeEquip(EquipCode newEquipCode);

    void fillObtainMissionItem(QuestMissionCode missionCode,
        BaseItemInfos& obtainItems, BaseItemInfos& questObtainItems) const;
    void removeMissionItems(bool isComplete);
	void removeObtainMissionInvenItem(QuestMissionCode missionCode);

private:
    bool isKillMissionCompleted(QuestMissionCode missionCode) const;
    bool isObtainMissionCompleted(BaseItemInfos& obtainItems,
        BaseItemInfos& questObtainItems) const;
    bool isTransportMissionCompleted(QuestMissionCode missionCode) const;
    bool isProbeMissionCompleted(QuestMissionCode missionCode) const;
    bool isActivationMissionCompleted(QuestMissionCode missionCode) const;
	bool isContentsMissionCompleted(QuestMissionCode missionCode) const;

    ErrorCode canUpgradeEquipItem(EquipCode& newEquipCode,
        DataCode checkCode, DataCode selectCode) const;
private:
	go::Entity& owner_;
	const datatable::QuestTemplate& questTemplate_;
	AcceptedQuestInfo questInfo_;
    mutable ActivationMissionMap activationMissionMap_;
	mutable ObtainMissionMap obtainMissionMap_;
	mutable bool initObtainMissionMap_;
};

}} // namespace gideon { namespace zoneserver {