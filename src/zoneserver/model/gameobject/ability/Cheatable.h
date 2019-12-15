#pragma once

#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/LevelInfo.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/QuestInfo.h>
#include <gideon/cs/shared/data/Money.h>
#include <gideon/cs/shared/data/ArenaInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/CheatInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Cheatable
 *
 */
class Cheatable
{
public:
	virtual ~Cheatable() {}

public:
	/// 후날 작업
	virtual ErrorCode updateCheatGameMoney(ValueChangetype type, GameMoney value) = 0;
	virtual ErrorCode updateCheatArenaPoint(ValueChangetype type, ArenaPoint value) = 0;
	virtual ErrorCode updateCheatEventCoin(ValueChangetype type, EventCoin value) = 0;
	virtual ErrorCode updateCheatForgeCoin(ValueChangetype type, ForgeCoin value) = 0;
	virtual ErrorCode updateCheatExp(ValueChangetype type, ExpPoint value) = 0;
	virtual ErrorCode updateCheatSpeed(ValueChangetype type, float32_t value) = 0;
	virtual ErrorCode updateCheatLevel(ValueChangetype type, bool isMajorLevel, uint8_t value) = 0;
	virtual ErrorCode updateCheatSkillPoint(ValueChangetype type, SkillPoint value) = 0;
	virtual ErrorCode updateCheatPoint(ValueChangetype type, PointType pointType, uint32_t value) = 0;
	virtual ErrorCode updateChao(ValueChangetype type, bool isTempChao, Chaotic value) = 0;
    
    virtual ErrorCode setPlayerStatus(CheatValueType valueType, int32_t value) = 0;
    virtual ErrorCode setZeroAllCurrentStatus() = 0;
    virtual ErrorCode restoreCurrentStatus() = 0;

	virtual ErrorCode addCheatItem(DataCode itemCode, uint8_t itemCount) = 0;
	virtual ErrorCode addCheatSkill(SkillCode skillCode) = 0;
	virtual ErrorCode addCheatQuest(QuestCode questCode) = 0;
    virtual ErrorCode addCheatCharacterTitle(CharacterTitleCode titleCode) = 0;

	virtual ErrorCode removeCheatQuest(QuestCode questCode) = 0;

	virtual ErrorCode resetCheatCoolTime() = 0;

	virtual ErrorCode toCheatPlayer(Entity& target) = 0;
	virtual ErrorCode toCheatMe(Entity& target) = 0;
	virtual ErrorCode toCheatPosition(const Position& target) = 0;

	virtual ErrorCode whos() = 0;

	virtual ErrorCode refillCheatPoints() = 0;

	virtual ErrorCode clearAcceptQuest(QuestCode questCode) = 0;
};


}}} // namespace gideon { namespace zoneserver { namespace go {