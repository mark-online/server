#include "ZoneServerPCH.h"
#include "AcceptedQuest.h"
#include "../gameobject/Entity.h"
#include "../gameobject/ability/Inventoryable.h"
#include "../gameobject/ability/Rewardable.h"
#include "../item/Inventory.h"
#include "../item/EquipInventory.h"
#include "../item/QuestInventory.h"
#include "../../service/item/ItemIdGenerator.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/QuestCallback.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../world/WorldMap.h"
#include "../../world/region/MapRegion.h"
#include <gideon/cs/shared/data/FragmentInfo.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/QuestTable.h>
#include <gideon/cs/datatable/QuestKillMissionTable.h>
#include <gideon/cs/datatable/QuestObtainMissionTable.h>
#include <gideon/cs/datatable/QuestTransportMissionTable.h>
#include <gideon/cs/datatable/QuestActivationMissionTable.h>
#include <gideon/cs/datatable/QuestContentsMissionTable.h>

namespace gideon { namespace zoneserver {


AcceptedQuest::AcceptedQuest(go::Entity& owner, const datatable::QuestTemplate& questTemplate,
	const AcceptedQuestInfo& questInfo) :
	owner_(owner),
	questTemplate_(questTemplate),
	questInfo_(questInfo),
	initObtainMissionMap_(false)
{
}


bool AcceptedQuest::isCompleted() const
{
    BaseItemInfos obtainItems;
    BaseItemInfos obtainQuestItems;
	for (QuestMissionCode missionCode : questTemplate_.getMissionCodes()) {
		QuestMissionType missionType = getQuestMissionType(missionCode);
        if (isKillMission(missionType)) {
            if (! isKillMissionCompleted(missionCode)) {
                return false;
            }
        }
        else if (isObtainMission(missionType)) {
            fillObtainMissionItem(missionCode, obtainItems, obtainQuestItems);
        }
        else if (isTransportMission(missionType)) {
            if (! isTransportMissionCompleted(missionCode)) {
                return false;
            }
        }
        else if (isProbeMission(missionType)) {
            if (! isProbeMissionCompleted(missionCode)) {
                return false;
            }
        }
        else if (isActivationMission(missionType)) {
            if (! isActivationMissionCompleted(missionCode)) {
                return false;
            }
        }
		else if (isContentMission(missionType)) {
			if (! isContentsMissionCompleted(missionCode)) {
				return false;
			}
		}
        else {
            assert(false);
        }
	}
    
	return isObtainMissionCompleted(obtainItems, obtainQuestItems);
}


bool AcceptedQuest::isMissionCompleted(QuestMissionCode missionCode) const
{
    const QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
    if (! playerGoalInfos) {
        return false;
    }

    BaseItemInfos obtainItems;
    BaseItemInfos obtainQuestItems;

    const QuestMissionType missionType = getQuestMissionType(missionCode);
    if (isKillMission(missionType)) {
        if (! isKillMissionCompleted(missionCode)) {
            return false;
        }
    }
    else if (isObtainMission(missionType)) {
        fillObtainMissionItem(missionCode, obtainItems, obtainQuestItems);
    }
    else if (isTransportMission(missionType)) {
        if (! isTransportMissionCompleted(missionCode)) {
            return false;
        }
    }
    else if (isProbeMission(missionType)) {
        if (! isProbeMissionCompleted(missionCode)) {
            return false;
        }
    }
    else if (isActivationMission(missionType)) {
        if (! isActivationMissionCompleted(missionCode)) {
            return false;
        }
    }
    else {
        assert(false);
    }

    return isObtainMissionCompleted(obtainItems, obtainQuestItems);
}


QuestMissionCode AcceptedQuest::kill(ObjectType objectType, DataCode code, Chaotic chaotic)
{
    // FYI: 하나의 퀘스트에 동일한 킬 미션이 없다고 가정함!
    QuestMissionCode completedMissionCode = invalidQuestMissionCode;

    for (QuestMissionCode missionCode : questTemplate_.getMissionCodes()) {
        const datatable::QuestKillMissionTemplate* missionTemplate =
            QUEST_KILL_MISSION_TABLE->getQuestKillMissionTemplate(missionCode);
        if (! missionTemplate) {
            continue;
        }

        QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
        const datatable::QuestMissionKillGoalInfo& goalInfo = missionTemplate->getGoalInfo();
        if ((goalInfo.goalType_ == qgtKillUser && objectType == otPc) ||
            (goalInfo.goalType_ == qgtKillNpc && (objectType == otMonster || objectType == otNpc)) ||
            (goalInfo.goalType_ == qgtKillChaotic && chaotic >= toChaotic(goalInfo.goalValue_))) {
            if (goalInfo.goalCode_ == code) {
                if (! playerGoalInfos) {
                    addMissionInfo(missionCode, code);
                }
                else {
                    updateKillMissionInfo(*playerGoalInfos, goalInfo, missionCode, code);
                }

                if (isKillMissionCompleted(missionCode)) {
                    completedMissionCode = missionCode;
                }
            }
        }
    }
    return completedMissionCode;
}


bool AcceptedQuest::transfer(QuestMissionCode missionCode, QuestItemCode questItemCode)
{
    if (questInfo_.getQuestGoalInfos(missionCode)) {
        return false;
    }

    addMissionInfo(missionCode, questItemCode);

    QuestInventory& inventory = owner_.queryInventoryable()->getQuestInventory();
    inventory.removeItem(questItemCode, questInfo_.questCode_);    
    
    return true;
}


bool AcceptedQuest::probe(QuestMissionCode missionCode, RegionCode regionCode)
{
    if (questInfo_.getQuestGoalInfos(missionCode)) {
        return false;
    }
    addMissionInfo(missionCode, regionCode);
    return true;
}


void AcceptedQuest::doContents(QuestContentMissionType type, DataCode dataCode)
{
	for (QuestMissionCode missionCode : questTemplate_.getMissionCodes()) {
		const datatable::QuestContentMissionTemplate* missionTemplate = QUEST_CONTENTS_MISSION_TABLE->getQuestContentsMission(missionCode);
		if (! missionTemplate) {
			continue;
		}
        const datatable::QuestContentsGoalInfo& goalInfo = missionTemplate->getGoalInfo();
        // TODO: 현재는 1회성만 있다 다수가 생길경우 코딩 해주어야 함.
		if (type == toQuestContentMissionType(goalInfo.contentMissionType_)) {
			if (type == qcmtArenaEnter || type == qcmtEquipUpgrade || type == qcmtEquipEnchant || type == qcmtSkillLearn) {
				QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
				if (! playerGoalInfos) {
					addMissionInfo(missionCode, invalidDataCode);
				}				
			}
            else if (type == qcmtTargetAttack && goalInfo.goalCode_ == dataCode) {
                QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
                if (! playerGoalInfos) {
                    addMissionInfo(missionCode, dataCode);
                }
            }
		}
	}
}


ErrorCode AcceptedQuest::checkActivate(QuestMissionCode missionCode, DataCode dataCode)
{
    const datatable::QuestActivationMissionTemplate* missionTemplate =
        QUEST_ACTIVATION_MISSION_TABLE->getQuestActivationMissionTemplate(missionCode);
    if (! missionTemplate) {
        assert(false);
        return ecQuestInvalidMission;
    }

    const datatable::QuestActivationGoalInfo& goalInfo = missionTemplate->getGoalInfo();
    if (goalInfo.goalCode_ != dataCode) {
        assert(false && "what happened? --kcando");
        return ecQuestInvalidMission;
    }

    const MapCode mapCode = missionTemplate->getAvailableMapCode();
    if (isValidMapCode(mapCode)) {
        WorldMap* worldMap = owner_.getCurrentWorldMap();
        if (! worldMap) {
            return ecQuestInvalidRegion;
        }
        if (mapCode != worldMap->getMapCode()) {
            return ecQuestInvalidRegion;
        }

        const RegionCode regionCode = missionTemplate->getAvailableRegionCode();
        if (isValidRegionCode(regionCode)) {
            const MapRegion* mapRegion = worldMap->getMapRegion(regionCode);
            if (! mapRegion) {
                return ecQuestInvalidRegion;
            }
            if (! mapRegion->isContained(owner_.getPosition())) {
                return ecQuestInvalidRegion;
            }
        }
    }

    if (isCompleted()) {
        return ecQuestAlreadyCompleteQuest;
    }
    return ecOk;
}


ErrorCode AcceptedQuest::activate(QuestMissionCode missionCode, DataCode dataCode)
{
    const datatable::QuestActivationMissionTemplate* missionTemplate =
        QUEST_ACTIVATION_MISSION_TABLE->getQuestActivationMissionTemplate(missionCode);
    if (! missionTemplate) {
        assert(false);
        return ecQuestInvalidMission;
    }

    const datatable::QuestActivationGoalInfo& goalInfo = missionTemplate->getGoalInfo();
    if (goalInfo.goalCode_ != dataCode) {
        assert(false);
        return ecQuestInvalidMission;
    }

    QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
    if (! playerGoalInfos) {
        addMissionInfo(missionCode, dataCode);
    }
    else {
        for (QuestGoalInfo& info : *playerGoalInfos) {
            if (info.goalCode_ != goalInfo.goalCode_) {
                continue;
            }
            if (info.goalValue_ >= goalInfo.goalValue_) {
                continue;
            }
            ++info.goalValue_;
            gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
            if (callback) {
                callback->missionUpdated(questTemplate_.getQuestCode(), missionCode,
                    info, questTemplate_.isRepeatQuest());
            }
        }
    }

    // FYI: 이미 작동한 경우에도 작동 성공으로 취급
    return ecOk;
}


void AcceptedQuest::cancel()
{
    removeMissionItems(false);
}


ErrorCode AcceptedQuest::reward(DataCode selectItemCode)
{
    removeMissionItems(true);

	BaseItemInfos itemInfos;
	BaseItemInfos questItemInfos;

	for (const datatable::QuestRemunerationInfo& info : questTemplate_.getQuestRemunerationInfos()) {
		if (info.kind_ == qrkItem) {
			itemInfos.push_back(BaseItemInfo(info.remunerationCode_, static_cast<uint8_t>(info.remunerationValue_)));
		}
		else if (info.kind_ == qrkQuestItem) {
			questItemInfos.push_back(BaseItemInfo(info.remunerationCode_, static_cast<uint8_t>(info.remunerationValue_)));
		}
        else if (info.kind_ == qrkArmorFragment) {
            FragmentCode fragmentCode = owner_.queryInventoryable()->getEquipInventory().getEquipFragmentCode(epArmor);
            if (! isValidFragmentCode(fragmentCode)) {
                return ecQuestNotNeedArmorItemEquipped;
            }
            itemInfos.push_back(BaseItemInfo(fragmentCode, static_cast<uint8_t>(info.remunerationValue_)));
        }
        else if (info.kind_ == qrkWeaponFragment) {
            FragmentCode fragmentCode = owner_.queryInventoryable()->getEquipInventory().getEquipFragmentCode(epTwoHands);
            if (! isValidFragmentCode(fragmentCode)) {
                fragmentCode = owner_.queryInventoryable()->getEquipInventory().getEquipFragmentCode(epRightHand);
            }
            if (! isValidFragmentCode(fragmentCode)) {
                return ecQuestNotNeedWeaponItemEquipped;
            }
            itemInfos.push_back(BaseItemInfo(fragmentCode, static_cast<uint8_t>(info.remunerationValue_)));
        }
	}

    if (isItemType(getCodeType(selectItemCode))) {
        const BaseItemInfo* itemInfo = questTemplate_.getSelectItem(selectItemCode);
        if (! itemInfo) {
            return ecQuestInvalidSelectItem;
        }
        itemInfos.push_back(*itemInfo);
    }

    if (! itemInfos.empty()) {
        if (! owner_.queryInventoryable()->getInventory().canAddItemByBaseItemInfos(itemInfos)) {
            return ecInventoryIsFull;
        }
    }

	if (! questItemInfos.empty()) {
		if (! owner_.queryInventoryable()->getQuestInventory().canAddQuestItem(uint8_t(questItemInfos.size()))) {
			return ecInventoryIsFull;
		}
	}

	if (! questItemInfos.empty()) {
		const ErrorCode errorCode = rewardQuestItem(questItemInfos);
		if (isFailed(errorCode)) {
			return errorCode;
		}
	}
	
	if (! itemInfos.empty()) {
		const ErrorCode errorCode = rewardItem(itemInfos);
		if (isFailed(errorCode)) {
			return errorCode;
		}
	}


    gc::RewardCallback* callback =  owner_.getController().queryRewardCallback();
    if (! callback) {
        assert(false);
        return ecOk;
    }

	for (const datatable::QuestRemunerationInfo& info : questTemplate_.getQuestRemunerationInfos()) {
		if (info.kind_ == qrkMoney) {
    		callback->gameMoneyRewarded(info.remunerationValue_, true);
		}
        else if (info.kind_ == qrkExp) {
            callback->expRewarded(toExpPoint(info.remunerationValue_));
        }
        else if (info.kind_ == qrkChaotic) {
            callback->chaoticUpRewared(toChaotic(info.remunerationValue_));
        }
        else if (info.kind_ == qrkChangeCharacterClass) {
            callback->changeCharacterClass(toCharacterClass(info.remunerationCode_));
        }
	}

	return ecOk;
}


QuestMissionCode AcceptedQuest::getUncompletedActivationMissionCodeBy(DataCode dataCode) const
{
    // lazy evaluation
    if (activationMissionMap_.empty()) {
        for (const QuestMissionCode missionCode : questTemplate_.getMissionCodes()) {
            if (! isActivationMission(getQuestMissionType(missionCode))) {
                continue;
            }
            const datatable::QuestActivationMissionTemplate* missionTemplate =
                QUEST_ACTIVATION_MISSION_TABLE->getQuestActivationMissionTemplate(missionCode);
            if (! missionTemplate) {
                assert(false);
                continue;
            }
            const datatable::QuestActivationGoalInfo& info = missionTemplate->getGoalInfo();
            if (info.goalCode_ == dataCode) {
                activationMissionMap_[dataCode] = missionCode;
            }
        }
    }

    const ActivationMissionMap::const_iterator pos = activationMissionMap_.find(dataCode);
    if (pos != activationMissionMap_.end()) {
        return (*pos).second;
    }
    return invalidQuestMissionCode;
}


const BaseItemInfo* AcceptedQuest::getHasQuestMissionItemInfo(QuestItemCode questItemCode) const
{
	if (! initObtainMissionMap_) {
		initObtainMissionMap_ = true;
		for (const QuestMissionCode missionCode : questTemplate_.getMissionCodes()) {
			if (! isObtainMission(getQuestMissionType(missionCode))) {
				continue;
			}
			const datatable::QuestObtainMissionTemplate* missionTemplate =
				QUEST_OBTAIN_MISSION_TABLE->getQuestObtainMissionTemplate(missionCode);
			if (! missionTemplate) {
				assert(false);
				continue;
			}
			const datatable::QuestMissionObtainGoalInfo& info = missionTemplate->getGoalInfo();
			if (isQuestItemType(getCodeType(info.goalCode_))) {
				obtainMissionMap_.insert(
                    ObtainMissionMap::value_type(info.goalCode_,
                        BaseItemInfo(info.goalCode_, uint8_t(info.goalValue_))));
			}
		}
	}

	const ObtainMissionMap::const_iterator pos = obtainMissionMap_.find(questItemCode);
	if (pos != obtainMissionMap_.end()) {
		return &(*pos).second;
	}
	return nullptr;
}


QuestCode AcceptedQuest::getQuestCode() const 
{
	return questTemplate_.getQuestCode();
}


bool AcceptedQuest::isRepeatQuest() const
{
    return questTemplate_.isRepeatQuest();
}


void AcceptedQuest::addMissionInfo(QuestMissionCode missionCode, DataCode dataCode)
{
    const QuestGoalInfo goalInfo(dataCode, 1);
	QuestGoalInfos goalInfos;
	goalInfos.push_back(goalInfo);
	questInfo_.questGoalInfosMap_.emplace(missionCode, goalInfos);

	gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
	if (callback) {
		callback->missionUpdated(questTemplate_.getQuestCode(), missionCode, goalInfo, questTemplate_.isRepeatQuest());
	}
}


void AcceptedQuest::updateKillMissionInfo(QuestGoalInfos& infos, 
    const datatable::QuestMissionKillGoalInfo& killGoalInfo, QuestMissionCode missionCode, DataCode dataCode)
{
    // 현재 킬만 쓴다
    bool isKillCountUpdated = false;
	for (QuestGoalInfo& goalInfo : infos) {
		if (goalInfo.goalCode_ == dataCode) {
            isKillCountUpdated = true;
            if (killGoalInfo.goalValue_ > goalInfo.goalValue_) {
			    ++goalInfo.goalValue_;
                gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
                if (callback) {
                    callback->missionUpdated(questTemplate_.getQuestCode(), missionCode,
                        goalInfo, questTemplate_.isRepeatQuest());
                }
                return;
            }
        }
	}

    QuestGoalInfo addGoalInfo(dataCode, 1);
    infos.push_back(addGoalInfo);
    gc::QuestCallback* callback = owner_.getController().queryQuestCallback();
    if (callback) {
        callback->missionUpdated(questTemplate_.getQuestCode(), missionCode,
            addGoalInfo, questTemplate_.isRepeatQuest());
    }
}


ErrorCode AcceptedQuest::rewardItem(const BaseItemInfos& itemInfos)
{
    if (itemInfos.size() == 1) {
		const BaseItemInfo& itemInfo = (*itemInfos.begin());
		ErrorCode errorCode = rewardItem(itemInfo);
		if (isFailed(errorCode)) {
			return errorCode;
		}
	}
	else if (itemInfos.size() > 1) {
		if (! owner_.queryInventoryable()->getInventory().canAddItemByBaseItemInfos(itemInfos)) {
			return ecInventoryIsFull;
		}
		for (const BaseItemInfo& info : itemInfos) {
			if (isFailed(rewardItem(info))) {
				assert(false);
				return ecInventoryIsFull;
			}
		}
	}
	return ecOk;
}


ErrorCode AcceptedQuest::rewardQuestItem(const BaseItemInfos& itemInfos)
{
	for (const BaseItemInfo& info : itemInfos) {
		const ErrorCode errorCode = rewardQuestItem(info);
		if (isFailed(errorCode)) {
			assert(false);
			return ecInventoryIsFull;
		}
	}
	return ecOk;
}


ErrorCode AcceptedQuest::rewardQuestItem(const BaseItemInfo& itemInfo)
{
	gc::RewardCallback* callback = owner_.getController().queryRewardCallback();
	if (! callback) {
		return ecServerInternalError;
	}

	const QuestItemInfo questItem(ITEM_ID_GENERATOR->generateItemId(),
		itemInfo.itemCode_,itemInfo.count_, 1, invalidQuestCode);
	return callback->questItemRewarded(questItem) ? ecOk : ecInventoryIsFull;
}


ErrorCode AcceptedQuest::rewardUpgradeEquip(EquipCode newEquipCode)
{
	gc::RewardCallback* callback = owner_.getController().queryRewardCallback();
	if (! callback) {
		return ecServerInternalError;
	}

	return callback->updateEquipRewared(newEquipCode);
}


ErrorCode AcceptedQuest::rewardItem(const BaseItemInfo& itemInfo)
{
	gc::RewardCallback* callback = owner_.getController().queryRewardCallback();
	if (! callback) {
		return ecServerInternalError;
	}

	return callback->itemRewarded(itemInfo);
}


bool AcceptedQuest::isKillMissionCompleted(QuestMissionCode missionCode) const
{
    const datatable::QuestKillMissionTemplate* missionTemplate =
        QUEST_KILL_MISSION_TABLE->getQuestKillMissionTemplate(missionCode);
    if (! missionTemplate) {
        assert(missionTemplate);
        return false;
    }

    const QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
    if (! playerGoalInfos) {
        return false;
    }

    const datatable::QuestMissionKillGoalInfo& goalInfo = missionTemplate->getGoalInfo();
    for (const QuestGoalInfo& playerGoalInfo : *playerGoalInfos) {	
        if (playerGoalInfo.goalCode_ == goalInfo.goalCode_ &&
            playerGoalInfo.goalValue_ >= goalInfo.goalValue_) {
            return true;
        }
    }

    return false;
}


bool AcceptedQuest::isObtainMissionCompleted(BaseItemInfos& obtainItems,
    BaseItemInfos& questObtainItems) const
{
    if (! obtainItems.empty()) {
        const Inventory& inventory = owner_.queryInventoryable()->getInventory();
        if (! inventory.checkHasItemsByBaseItemInfos(obtainItems)) {
            return false;
        }
    }
    if (! questObtainItems.empty()) {
        const QuestInventory& inventory = owner_.queryInventoryable()->getQuestInventory();
        if (! inventory.checkHasQuestItems(obtainItems)) {
            return false;
        }
    }
    return true;
}


bool AcceptedQuest::isTransportMissionCompleted(QuestMissionCode missionCode) const
{    
    return questInfo_.getQuestGoalInfos(missionCode) != nullptr;
}


bool AcceptedQuest::isProbeMissionCompleted(QuestMissionCode missionCode) const
{    
    return questInfo_.getQuestGoalInfos(missionCode) != nullptr;
}


bool AcceptedQuest::isActivationMissionCompleted(QuestMissionCode missionCode) const
{
    const QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
    if (! playerGoalInfos) {
        return false;
    }

    const datatable::QuestActivationMissionTemplate* missionTemplate =
        QUEST_ACTIVATION_MISSION_TABLE->getQuestActivationMissionTemplate(missionCode);
    if (! missionTemplate) {
        assert(missionTemplate);
        return false;
    }

    const datatable::QuestActivationGoalInfo& goalInfo = missionTemplate->getGoalInfo();
    for (const QuestGoalInfo& playerGoalInfo : *playerGoalInfos) {	
        if (playerGoalInfo.goalCode_ == goalInfo.goalCode_ &&
            playerGoalInfo.goalValue_ == goalInfo.goalValue_) {
            return true;
        }
    }

    return false;
}


bool AcceptedQuest::isContentsMissionCompleted(QuestMissionCode missionCode) const
{
	const QuestGoalInfos* playerGoalInfos = questInfo_.getQuestGoalInfos(missionCode);
	if (! playerGoalInfos) {
		return false;
	}
	// TODO 추후에 이것저것 내용이 들어갈것
	return true;

}

void AcceptedQuest::fillObtainMissionItem(QuestMissionCode missionCode,
    BaseItemInfos& obtainItems, BaseItemInfos& questObtainItems) const
{
    const datatable::QuestObtainMissionTemplate* missionTemplate =
        QUEST_OBTAIN_MISSION_TABLE->getQuestObtainMissionTemplate(missionCode);
    if (! missionTemplate) {
        assert(missionTemplate);
    }

    const datatable::QuestMissionObtainGoalInfo& goalInfo = missionTemplate->getGoalInfo();
    if (goalInfo.goalType_ == qgtObtainItem) {
        const BaseItemInfo itemInfo(goalInfo.goalCode_, uint8_t(goalInfo.goalValue_));
        const CodeType codeType = getCodeType(goalInfo.goalCode_);
        if (ctQuestItem == codeType) {
            questObtainItems.push_back(itemInfo);
        }
        else {
            obtainItems.push_back(itemInfo);
        }
    }
}


// 수집했던 퀘스트 아이템을 삭제해야 한다
// 지급 받은 퀘스트 아이템을 삭제해야 한다
void AcceptedQuest::removeMissionItems(bool isComplete)
{
    QuestInventory& inventory = owner_.queryInventoryable()->getQuestInventory();
    inventory.removeItemsByQuestCode(questInfo_.questCode_);
	if (isComplete) {
		for (const QuestMissionCode missionCode : questTemplate_.getMissionCodes()) {
			if (! isObtainMission(getQuestMissionType(missionCode))) {
				continue;
			}
			removeObtainMissionInvenItem(missionCode);
		}
	}
}


void AcceptedQuest::removeObtainMissionInvenItem(QuestMissionCode missionCode)
{
	const datatable::QuestObtainMissionTemplate* missionTemplate =
		QUEST_OBTAIN_MISSION_TABLE->getQuestObtainMissionTemplate(missionCode);
	assert(missionTemplate);

	const datatable::QuestMissionObtainGoalInfo& goalInfo = missionTemplate->getGoalInfo();
	if (goalInfo.goalType_ == qgtObtainItem) {
		const CodeType codeType = getCodeType(goalInfo.goalCode_);
		if (! isQuestItemType(codeType) && isItemType(codeType)) {
            const BaseItemInfo itemInfo(goalInfo.goalCode_, uint8_t(goalInfo.goalValue_));
            Inventory& inventory = owner_.queryInventoryable()->getInventory();
            inventory.useItemsByBaseItemInfo(itemInfo);
		}
	}
}


}} // namespace gideon { namespace zoneserver {