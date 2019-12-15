#pragma once

#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class RewardCallback
 */
class RewardCallback
{
public:
    virtual ~RewardCallback() {}

public:
    virtual void expRewarded(ExpPoint rewardExp) = 0;
	virtual void gameMoneyRewarded(GameMoney gameMoney, bool isUp) = 0;
	virtual ErrorCode itemRewarded(const BaseItemInfo& baseItemInfo) = 0;
	virtual bool questItemRewarded(const QuestItemInfo& baseItemInfo) = 0;

    virtual void chaoticUpRewared(Chaotic chaotic) = 0;
    virtual void changeCharacterClass(CharacterClass cc) = 0;
    virtual void skillRewared(SkillTableType skillType, SkillIndex index) = 0;

	virtual ErrorCode updateEquipRewared(EquipCode newEquipCode) = 0;
    /// 자시을 제외한 플레이어들이 받는다.
    virtual void characterClassChanged(ObjectId playerId, CharacterClass cc) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
