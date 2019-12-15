#pragma once

#include <gideon/cs/shared/data/CheatInfo.h>
#include <gideon/cs/shared/data/AccountInfo.h>
#include <gideon/cs/shared/data/StatusInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver {

namespace go {
class Player;
} // namespace go


class CheatCommander
{
public:
	typedef sne::core::Vector<std::wstring> Params;
public:
	CheatCommander(go::Player& player, AccountGrade accountGrade);
	~CheatCommander() {}
	ErrorCode execute(const std::wstring& command, const std::wstring& paramStr);

private:
	// 1차 파싱 함수
	ErrorCode upCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode downCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode setCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode addCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode removeCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode toCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode showCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode resetCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode spawnCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode despawnCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode refillCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode logoutCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode botCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode clearCheat(const std::wstring& subCommand, const Params& params);
    ErrorCode openCheat(const std::wstring& subCommand, const Params& params);
	ErrorCode closeCheat(const std::wstring& subCommand, const Params& params);
    ErrorCode reviveCheat(const std::wstring& subCommand, const Params& params);
    ErrorCode changeCheat(const std::wstring& subCommand, const Params& params);
    ErrorCode godCheat(const std::wstring& subCommand, const Params& params);
    ErrorCode hideCheat(const std::wstring& subCommand, const Params& params);

private:
	ErrorCode updateGameMoney(ValueChangetype type, const Params& params);
	ErrorCode updateArenaPoint(ValueChangetype type, const Params& params);
	ErrorCode updateEventCoin(ValueChangetype type, const Params& params);
	ErrorCode updateForgeCoin(ValueChangetype type, const Params& params);
	ErrorCode updateMercenaryPoint(ValueChangetype type, const Params& params);
	ErrorCode updateExp(ValueChangetype type, const Params& params);
	ErrorCode updateSpeed(ValueChangetype type, const Params& params);
	ErrorCode updateLevel(ValueChangetype type, bool isMajorLevel, const Params& params);
	ErrorCode updateSkillPoint(ValueChangetype type, const Params& params);
	ErrorCode updatePoints(ValueChangetype type, PointType pointType, const Params& params);
	ErrorCode updateChao(ValueChangetype type, bool isTempChao, const Params& params);
    ErrorCode updateGuildExp(ValueChangetype type, const Params& params);
    
    ErrorCode setPlayerStatus(CheatValueType type, const Params& params);

    ErrorCode downBuildTime(const Params& params);
    ErrorCode updateBuildingHp(ValueChangetype type, const Params& params);

	ErrorCode addItem(const Params& params);
	ErrorCode addSkill(const Params& params);
	ErrorCode addQuest(const Params& params);
	ErrorCode addBan(const Params& params);
    ErrorCode addBuildingMaterials();
    ErrorCode addCharacterTitle(const Params& params);

	ErrorCode removeQuest(const Params& params);
	ErrorCode removeBan(const Params& params);

	ErrorCode toPlayer(const Params& params);
	ErrorCode toMe(const Params& params);
	ErrorCode toPosition(const Params& params);

private:
	go::Player& player_;
	AccountGrade accountGrade_;
};


}} // namespace gideon { namespace zoneserver {