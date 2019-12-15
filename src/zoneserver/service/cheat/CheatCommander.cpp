#include "ZoneServerPCH.h"
#include "CheatCommander.h"
#include "../../model/gameobject/Player.h"
#include "../../model/gameobject/Building.h"
#include "../../model/gameobject/status/StaticObjectStatus.h"
#include "../../model/gameobject/skilleffect/CreatureEffectScriptApplier.h"
#include "../../model/gameobject/skilleffect/PassiveSkillManager.h"
#include "../../model/state/CreatureState.h"
#include "../anchor/AnchorService.h"
#include "../arena/ArenaService.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../world/region/SpawnMapRegion.h"
#include "../spawn/SpawnService.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/BuildingStateCallback.h"
#include "../../controller/callback/BotCommandCallback.h"
#include "../../controller/callback/CreatureEffectCallback.h"
#include "../../controller/callback/GuildCallback.h"
#include "../../controller/callback/RewardCallback.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneArenaServerProxy.h"
#include "../../s2s/ZoneLoginServerProxy.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "../../service/world_event/WorldEventService.h"
#include "../../service/world_event/WorldEvent.h"
#include "../../helper/InventoryHelper.h"
#include <gideon/server/data/BanInfo.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/GemTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <boost/algorithm/string.hpp>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class BotFollowMeEvent
 */
class BotFollowMeEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BotFollowMeEvent>
{
public:
    BotFollowMeEvent(go::Player& player, bool shouldStopDestination) :
        player_(player),
        shouldStopDestination_(shouldStopDestination) {}

private:
    virtual void call(go::Entity& entity) {
        if (entity.isSame(player_)) {
            return;
        }
        gc::BotCommandCallback* callback = entity.getController().queryBotCommandCallback();
        if (callback != nullptr) {
            callback->commandMoved(player_.getPosition(), shouldStopDestination_);
        }
    }

private:
    go::Player& player_;
    bool shouldStopDestination_;
};


CheatCommander::Params makeParams(std::wstring& subCommand, const std::wstring& strParam) 
{
    // to lowercase: https://stackoverflow.com/a/313988
    std::wstring lowStrParam = boost::algorithm::to_lower_copy(strParam);

    CheatCommander::Params params;	
    std::wstring param;

    for (size_t i = 0; i < strParam.size(); ++i) {
        if (L' ' != strParam[i]) {
            param.push_back(strParam[i]);
            if (i == strParam.size() -1) {
                params.push_back(param);
            }
        }
        else {
            if (! param.empty()) {
                params.push_back(param);
                param.clear();
            }
        }
    }

    if (params.empty()) {
        return params;
    }

    subCommand = *params.begin();
    params.erase(params.begin());

    return params;
}

CheatValueType getCheatValueType(const std::wstring& str)
{
    if (L"gamemoney" == str || L"money" == str) {
        return cvtGameMoney;
    }
    else if (L"arenapoint" == str) {
        return cvtArenaPoint;
    }
    else if (L"eventcoin" == str) {
        return cvtEventCoin;
    }
    else if (L"forgecoin" == str) {
        return cvtForgeCoin;
    }
    else if (L"exp" == str)  {
        return cvtExp;
    }
    else if (L"speed" == str) {
        return cvtSpeed;
    }
    else if (L"majorlevel" == str || L"level" == str) {
        return cvtMajorLevel;
    }
    else if (L"minerlevel" == str || L"minlevel" == str) {
        return cvtMinerLevel;
    }
    else if (L"skillpoint" == str ) {
        return cvtSkillPoint;
    }
    else if (L"hp" == str) {
        return cvtHp;
    }
    else if (L"mp" == str) {
        return cvtMp;
    }	
    else if (L"chao" == str) {
        return cvtChao;
    }
    else if (L"tempchao" == str) {
        return cvtTempChao;
    }
    else if (L"buildtime" == str) {
        return cvtBuildTime;
    }
    else if (L"buildinghp" == str) {
        return cvtBuildingHp;
    }
    else if (L"strength" == str) {
        return cvtStrength;
    }
    else if (L"dexterity" == str) {
        return cvtDexterity;
    }
    else if (L"intellect" == str) {
        return cvtIntellect;
    }
    else if (L"energy" == str) {
        return cvtEnergy;
    }
    else if (L"minattack" == str) {
        return cvtMinAttack;
    }
    else if (L"maxattack" == str) {
        return cvtMaxAttack;
    }
    else if (L"magic" == str) {
        return cvtMagic;
    }
    else if (L"physical" == str) {
        return cvtPhysical;
    }
    else if (L"hit" == str) {
        return cvtHitRate;
    }
    else if (L"physicalcritical" == str) {
        return cvtPhysicalCriticalRate;
    }
    else if (L"magiccritical" == str) {
        return cvtMagicCriticalRate;
    }
    else if (L"parry" == str) {
        return cvtParryRate;
    }
    else if (L"block" == str) {
        return cvtBlockRate;
    }
    else if (L"dodge" == str) {
        return cvtDodgeRate;
    }
    else if (L"defence" == str) {
        return cvtDefence;
    }
    else if (L"light" == str) {
        return cvtLight;
    }
    else if (L"dark" == str) {
        return cvtDark;
    }
    else if (L"fire" == str) {
        return cvtFire;
    }
    else if (L"ice" == str) {
        return cvtIce;
    }
    else if (L"lightresist" == str) {
        return cvtLightResist;
    }
    else if (L"darkresist" == str) {
        return cvtDarkResist;
    }
    else if (L"fireresist" == str) {
        return cvtFireResist;
    }
    else if (L"iceresist" == str) {
        return cvtIceResist;
    }
    else if (L"guildexp" == str) {
        return cvtGuildExp;
    }

    return cvtUnknown;
}


go::Entity* getApplyPlayer(go::Player& player, const Nickname& nickname)
{
    if (! nickname.empty()) {
        return WORLD->getPlayer(nickname);
    }

    const GameObjectInfo& targetInfo = player.queryTargetSelectable()->getSelectedTargetInfo();
    if (! targetInfo.isValid()) {
        return &player;
    }

    if (! targetInfo.isPlayer()) {
        return &player;
    }

    if (player.isSame(player.queryTargetSelectable()->getSelectedTargetInfo())) {
        return &player;
    }
    
    return WORLD->getPlayer(targetInfo.objectId_);
}


template <typename T>
ErrorCode parserPlayerValueTypeParams(go::Entity*& target, T& value, go::Player& player, const CheatCommander::Params& params)
{
    if (params.size() == 2) {
        target = getApplyPlayer(player, params[0]);
        value = boost::lexical_cast<T>(params[1]);
    }
    else if (params.size() == 1) {
        target = &player;
        value = boost::lexical_cast<T>(params[0]);
    }

    if (! target) {
        return ecCheatInvalidCommand;
    }

    return ecOk;
}


ErrorCode parseSpawnParams(NpcCode& npcCode, msec_t& spawnDelay, msec_t& lifeTime, const CheatCommander::Params& params)
{
    if (params.size() < 1) {
        return ecCheatInvalidCommand;
    }

    npcCode = boost::lexical_cast<NpcCode>(params[0]);
    if (params.size() > 1) {
        spawnDelay = boost::lexical_cast<uint32_t>(params[1]) * 1000;
    }
    else {
        spawnDelay = 0;
    }
    if (params.size() > 2) {
        lifeTime = boost::lexical_cast<uint32_t>(params[2]) * 1000;
    }
    else {
        lifeTime = 0;
    }
    return ecOk;
}

} // namespace {


CheatCommander::CheatCommander(go::Player& player, AccountGrade accountGrade) :
    player_(player),
    accountGrade_(accountGrade)
{
}


ErrorCode CheatCommander::execute(const std::wstring& command, const std::wstring& paramStr)
{
    std::wstring subCommand;
    try {
        CheatCommander::Params params = makeParams(subCommand, paramStr);
    
        if (L"up" == command) {
            return upCheat(subCommand, params);
        }
        else if (L"down" == command) {
            return downCheat(subCommand, params);
        }
        else if (L"set" == command) {
            return setCheat(subCommand, params);
        }
        else if (L"add" == command) {
            return addCheat(subCommand, params);
        }
        else if (L"remove" == command) {
            return removeCheat(subCommand, params);
        }
        else if (L"to" == command) {
            return toCheat(subCommand, params);
        }		
        else if (L"show" == command) {
            return showCheat(subCommand, params);
        }
        else if (L"reset" == command) {
            return resetCheat(subCommand, params);
        }
        else if (L"refill" == command) {
            return refillCheat(subCommand, params);
        }
        else if (L"logout" == command) {
            return logoutCheat(subCommand, params);
        }
        else if (L"spawn" == command) {
            return spawnCheat(subCommand, params);
        }
        else if (L"despawn" == command) {
            return despawnCheat(subCommand, params);
        }
        else if (L"bot" == command) {
            return botCheat(subCommand, params);
        }
        else if (L"clear" == command) {
            return clearCheat(subCommand, params);
        }
        else if (L"open" == command) {
            return openCheat(subCommand, params);
        }
        else if (L"close" == command) {
            return closeCheat(subCommand, params);
        }      
        else if (L"revive" == command) {
            return reviveCheat(subCommand, params);
        }
        else if (L"change" == command) {
            return changeCheat(subCommand, params);
        }
        else if (L"god" == command) {
            return godCheat(subCommand, params);
        }
        else if (L"hide" == command) {
            return hideCheat(subCommand, params);
        }
    }
    catch (boost::bad_lexical_cast&) {
        return ecCheatInvalidParam;
    }

    return ecCheatInvalidCommand;
}


ErrorCode CheatCommander::upCheat(const std::wstring& subCommand, const Params& params)
{
    CheatValueType type = getCheatValueType(subCommand);
    if (cvtGameMoney == type) {
        return updateGameMoney(vctUp, params);
    }
    else if (cvtArenaPoint == type) {
        return updateArenaPoint(vctUp, params);
    }
    else if (cvtEventCoin == type) {
        return updateEventCoin(vctUp, params);
    }	
    else if (cvtForgeCoin == type) {
        return updateForgeCoin(vctUp, params);
    }
    else if (cvtExp == type) {
        return updateExp(vctUp, params);
    }
    else if (cvtSpeed == type) {
        return updateSpeed(vctUp, params);
    }
    else if (cvtMajorLevel == type) {
        return updateLevel(vctUp, true, params);
    }
    else if (cvtMinerLevel == type) {
        return updateLevel(vctUp, false, params);
    }
    else if (cvtSkillPoint == type) {
        return updateSkillPoint(vctUp, params);
    }
    else if (cvtHp == type) {
        return updatePoints(vctUp, ptHp, params);
    }
    else if (cvtMp == type) {
        return updatePoints(vctUp, ptMp, params);
    }
    else if (cvtChao == type) {
        return updateChao(vctUp, false, params);
    }
    else if (cvtTempChao == type) {
        return updateChao(vctUp, true, params);
    }
    else if (cvtBuildingHp == type) {
        return updateBuildingHp(vctUp, params);
    }
    else if (cvtGuildExp == type) {
        return updateGuildExp(vctUp, params);
    }

    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::downCheat(const std::wstring& subCommand, const Params& params)
{
    CheatValueType type = getCheatValueType(subCommand);
    if (cvtGameMoney == type) {
        return updateGameMoney(vctDown, params);
    }
    else if (cvtArenaPoint == type) {
        return updateArenaPoint(vctDown, params);
    }
    else if (cvtEventCoin == type) {
        return updateEventCoin(vctDown, params);
    }
    else if (cvtForgeCoin == type) {
        return updateForgeCoin(vctDown, params);
    }
    else if (cvtExp == type) {
        return updateExp(vctDown, params);
    }
    else if (cvtSpeed == type) {
        return updateSpeed(vctDown, params);
    }
    else if (cvtMajorLevel == type) {
        return updateLevel(vctDown, true, params);
    }
    else if (cvtMinerLevel == type) {
        return updateLevel(vctDown, false, params);
    }
    else if (cvtSkillPoint == type) {
        return updateSkillPoint(vctDown, params);
    }
    else if (cvtHp == type) {
        return updatePoints(vctDown, ptHp, params);
    }
    else if (cvtMp == type) {
        return updatePoints(vctDown, ptMp, params);
    }
    else if (cvtChao == type) {
        return updateChao(vctDown, false, params);
    }
    else if (cvtTempChao == type) {
        return updateChao(vctDown, true, params);
    }
    else if (cvtBuildTime == type) {
        return downBuildTime(params);
    }
    else if (cvtBuildingHp == type) {
        return updateBuildingHp(vctDown, params);
    }

    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::setCheat(const std::wstring& subCommand, const Params& params)
{	
    CheatValueType type = getCheatValueType(subCommand);
    switch (type) {
    case cvtStrength:
    case cvtDexterity:
    case cvtIntellect:
    case cvtEnergy:
    case cvtMinAttack:
    case cvtMaxAttack:
    case cvtMagic:
    case cvtPhysical:
    case cvtHitRate:
    case cvtPhysicalCriticalRate:
    case cvtMagicCriticalRate:
    case cvtParryRate:
    case cvtBlockRate:
    case cvtDodgeRate:
    case cvtDefence:
    case cvtLight:
    case cvtDark:
    case cvtFire:
    case cvtIce:
    case cvtLightResist:
    case cvtDarkResist:
    case cvtFireResist:
    case cvtIceResist:
        return setPlayerStatus(type, params);
    case cvtGameMoney:
        return updateGameMoney(vctSet, params);
    case cvtArenaPoint:
        return updateArenaPoint(vctSet, params);
    case cvtEventCoin:
        return updateEventCoin(vctSet, params);
    case cvtForgeCoin:
        return updateForgeCoin(vctSet, params);
    case cvtExp:
        return updateExp(vctSet, params);
    case cvtSpeed:
        return updateSpeed(vctSet, params);
    case cvtMajorLevel:
        return updateLevel(vctSet, true, params);
    case cvtMinerLevel:
        return updateLevel(vctSet, false, params);
    case cvtSkillPoint:
        return updateSkillPoint(vctSet, params);
    case cvtHp:
        return updatePoints(vctSet, ptHp, params);
    case cvtMp:
        return updatePoints(vctSet, ptMp, params);
    case cvtChao:
        return updateChao(vctSet, false, params);
    case cvtTempChao:
        return updateChao(vctSet, true, params);
    }    
    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::addCheat(const std::wstring& subCommand, const Params& params)
{
    if (L"item" == subCommand) {
        return addItem(params);
    }
    else if (L"skill" == subCommand) {
        return addSkill(params);
    }
    else if (L"quest" == subCommand) {
        return addQuest(params);
    }
    else if (L"ban" == subCommand) {
        return addBan(params);
    }
    else if (L"buildingmaterial" == subCommand) {
        return addBuildingMaterials();
    }
    else if (L"charactertitle" == subCommand) {
        return addCharacterTitle(params);
    }

    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::removeCheat(const std::wstring& subCommand, const Params& params)
{
    if (L"quest" == subCommand) {
        return removeQuest(params);
    }
    else if (L"ban" == subCommand) {
        return removeBan(params);
    }

    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::toCheat(const std::wstring& subCommand, const Params& params)
{
    if (L"player" == subCommand) {
        return toPlayer(params);
    }
    else if (L"position" == subCommand) {
        return toPosition(params);
    }
    else if (L"me" == subCommand) {
        return toMe(params);
    }
    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::showCheat(const std::wstring& subCommand, const Params& /*params*/)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"zoneusers" == subCommand) {
        player_.queryCheatable()->whos();
        return ecOk;
    }
    else if (L"worldusers" == subCommand) {
        ZoneCommunityServerProxy& communityServerProxy =
            ZONE_SERVICE->getCommunityServerProxy();
        communityServerProxy.z2m_getWorldUserInfos(player_.getObjectId());
        return ecOk;
    }

    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::resetCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"cooltime" == subCommand) {
        Nickname nickname;
        if (! params.empty()) {
            nickname = params[0]; 
        }
        go::Entity* target = getApplyPlayer(player_, nickname);
        if (! target) {
            return ecCheatNotFindTarget;
        }
        return target->queryCheatable()->resetCheatCoolTime();
    }
    else if (L"deserter" == subCommand) {
        if (ZONE_SERVICE->isArenaServer()) {
            ARENA_SERVICE->releaseDeserter(player_.getObjectId());
        }    
        else {
            ZONE_SERVICE->getArenaServerProxy().z2a_releaseDeserter(player_.getObjectId());
        }
        return ecOk;
    }
    else if (L"playerstatus" ==  subCommand) {
        Nickname nickname;
        if (! params.empty()) {
            nickname = params[0]; 
        }
        go::Entity* target = getApplyPlayer(player_, nickname);
        if (! target) {
            return ecCheatNotFindTarget;
        }
        return target->queryCheatable()->restoreCurrentStatus();
    }

    return ecCheatInvalidParam;	
}


ErrorCode CheatCommander::spawnCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"npc" == subCommand) {
        NpcCode npcCode = invalidNpcCode;
        msec_t spawnDelay = 0;
        msec_t lifeTime = 0;
        const ErrorCode errorCode = parseSpawnParams(npcCode, spawnDelay, lifeTime, params);
        if (isFailed(errorCode)) {
            return errorCode;
        }

        WorldMap* worldMap = player_.getCurrentWorldMap();
        if (! worldMap) {
            assert(false);
            return ecCheatInvalidParam;
        }

        SpawnMapRegion& globalSpawnMapRegion =
            static_cast<SpawnMapRegion&>(worldMap->getGlobalMapRegion());
        return globalSpawnMapRegion.summonNpc(player_, npcCode, spawnDelay, lifeTime, sptSummonByCheat, 5);
    }

    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::despawnCheat(const std::wstring& /*subCommand*/, const Params& /*params*/)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    const GameObjectInfo& targetInfo = player_.queryTargetSelectable()->getSelectedTargetInfo();
    // TODO: anchor
    //if (targetInfo.isAnchorOrBuilding()) {
    //	go::AbstractAnchor* abstractAnchor = ANCHOR_SERVICE->getAnchor(targetInfo);
    //	if (! abstractAnchor) {
 //           return ecCheatNotFindTarget;
 //       }
    //	if (! abstractAnchor->isBuilding()) {
 //           return ecCheatNotFindTarget;
 //       }

    //	gc::BuildingStateCallback* callback = abstractAnchor->getController().queryBuildingStateCallback();
    //	if (callback) {
    //		callback->buildDestroyed();
    //	}				
 //   	return static_cast<go::Entity*>(abstractAnchor)->despawn();
    //}

    go::Entity* target = player_.queryKnowable()->getEntity(targetInfo);
    if (! target) {
        return ecCheatNotFindTarget;
    }
    (void)SPAWN_SERVICE->scheduleDespawn(*target, 10);
    return ecOk;
}


ErrorCode CheatCommander::refillCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"points" == subCommand) {
        Nickname nickname;
        if (! params.empty()) {
            nickname = params[0]; 
        }
        go::Entity* target = getApplyPlayer(player_, nickname);
        if (! target) {
            return ecCheatNotFindTarget;
        }
        return target->queryCheatable()->refillCheatPoints();
    }


    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::logoutCheat(const std::wstring& /*subCommand*/, const Params& params)
{
    if (params.empty()) {
        return ecCheatInvalidParam;
    }
    
    go::Entity* target = getApplyPlayer(player_, params[0]);
    if (! target) {
        return ecCheatNotFindTarget;
    }

    target->queryNetworkable()->logout();
    return ecOk;
}



ErrorCode CheatCommander::botCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"followme" == subCommand) {
        auto event = std::make_shared<BotFollowMeEvent>(player_, ! params.empty());
        WORLD->broadcast(event);
    }
    else {
        return ecCheatInvalidParam;
    }

    return ecOk;
}


ErrorCode CheatCommander::clearCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"quest" == subCommand) {
        if (params.size() != 1) {
            return ecCheatInvalidParam;
        }
        QuestCode clearQuestCode = boost::lexical_cast<QuestCode>(params[0]);
        go::Cheatable* cheatable = player_.queryCheatable();
        if (cheatable) {
            return cheatable->clearAcceptQuest(clearQuestCode);
        }
    }
    else if (L"playerstatus" == subCommand) {
        Nickname nickname;
        if (! params.empty()) {
            nickname = params[0]; 
        }
        go::Entity* target = getApplyPlayer(player_, nickname);
        if (! target) {
            return ecCheatNotFindTarget;
        }
        return target->queryCheatable()->setZeroAllCurrentStatus();
    }

    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::openCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"worldevent" == subCommand) {
        if (params.size() != 1) {
            return ecCheatInvalidParam;
        }
        WorldEventCode worldEventCode = boost::lexical_cast<WorldEventCode>(params[0]);
        WorldEvent* event = WORLD_EVENT_SERVICE->getWorldEvent(worldEventCode);
        if (event) {
            event->cheatOpen();
        }
    }
    return ecOk;
}


ErrorCode CheatCommander::closeCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"worldevent" == subCommand) {
        if (params.size() != 1) {
            return ecCheatInvalidParam;
        }
        WorldEventCode worldEventCode = boost::lexical_cast<WorldEventCode>(params[0]);
        WorldEvent* event = WORLD_EVENT_SERVICE->getWorldEvent(worldEventCode);
        if (event) {
            event->cheatClose();
        }
    }
    return ecOk;
}


ErrorCode CheatCommander::reviveCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"player" == subCommand) {
        if (! (params.size() == 1 || params.size() == 2)) {
            return ecCheatInvalidParam;
        }
        go::Entity* target = nullptr;
        permil_t perRefillHp = 0;
        const ErrorCode errorCode = parserPlayerValueTypeParams(target, perRefillHp, player_, params);
        if (isFailed(errorCode)) {
            return errorCode;
        }
        gc::CreatureEffectCallback* callback = 
            target->getController().queryCreatureEffectCallback();
        if (callback) {
            callback->reviveAtOnce(perRefillHp);
        }
    }
    return ecOk;
}


ErrorCode CheatCommander::changeCheat(const std::wstring& subCommand, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (L"class" == subCommand) {
        if (! (params.size() == 1 || params.size() == 2)) {
            return ecCheatInvalidParam;
        }
        go::Entity* target = nullptr;
        int value = 0;
        
        const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
        if (isFailed(errorCode)) {
            return errorCode;
        }
        CharacterClass characterClass = toCharacterClass(value);
        if (! isValid(characterClass)) {
            return ecCharacterInvalidClass;
        }

        go::CharacterClassable* classable = target->queryCharacterClassable();
        if (classable) {
            classable->changeCharacterClass(characterClass);

            gc::RewardCallback* callback =  target->getController().queryRewardCallback();
            if (callback != nullptr) {
                callback->changeCharacterClass(characterClass);
            }
        }
    }

    return ecOk;
}


ErrorCode CheatCommander::godCheat(const std::wstring& /*subCommand*/, const Params& /*params*/)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    player_.getEffectScriptApplier().revert();
    player_.queryPassiveSkillCastable()->getPassiveSkillManager().revert();

    gc::CreatureEffectCallback* callback = 
        player_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->effectiveMesmerization(meztInvincible, ! player_.queryCreatureState()->isInvincible());        
    }

    return ecOk;
}


ErrorCode CheatCommander::hideCheat(const std::wstring& /*subCommand*/, const Params& /*params*/)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    gc::CreatureEffectCallback* callback = 
        player_.getController().queryCreatureEffectCallback();
    if (callback) {
        callback->effectiveMesmerization(meztHide, ! player_.queryCreatureState()->isHidden());        
    }

    return ecOk;
}

// = 2 차 파싱
ErrorCode CheatCommander::updateGameMoney(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    GameMoney value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return cheatable->updateCheatGameMoney(type, value);
}


ErrorCode CheatCommander::updateArenaPoint(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    ArenaPoint value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return cheatable->updateCheatArenaPoint(type, value);;
}


ErrorCode CheatCommander::updateEventCoin(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    EventCoin value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return cheatable->updateCheatEventCoin(type, value);;
}


ErrorCode CheatCommander::updateForgeCoin(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    ForgeCoin value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return cheatable->updateCheatForgeCoin(type, value);;
}


ErrorCode CheatCommander::updateExp(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    uint16_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    ExpPoint exp = toExpPoint(value);
    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return cheatable->updateCheatExp(type, exp);
}


ErrorCode CheatCommander::updateSpeed(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    // 현재 플레이어만 가능
    go::Entity* target = nullptr;
    float32_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return 	cheatable->updateCheatSpeed(type, value);
}


ErrorCode CheatCommander::updateLevel(ValueChangetype type, bool isMajorLevel, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    uint32_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return cheatable->updateCheatLevel(type, isMajorLevel, static_cast<uint8_t>(value));
}


ErrorCode CheatCommander::updateSkillPoint(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    uint16_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }
    SkillPoint sp = toSkillPoint(value);
    return cheatable->updateCheatSkillPoint(type, sp);
}


ErrorCode CheatCommander::updatePoints(ValueChangetype type, PointType pointType, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    uint32_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }
    return cheatable->updateCheatPoint(type, pointType, value);
}


ErrorCode CheatCommander::updateChao(ValueChangetype type, bool isTempChao, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    uint32_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }
    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }
    return cheatable->updateChao(type, isTempChao, toChaotic(value));
}


ErrorCode CheatCommander::updateGuildExp(ValueChangetype type, const Params& params)
{
    if (vctUp != type) {
        return ecCheatInvalidParam;
    }

    go::Entity* target = nullptr;
    uint32_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    const GuildId guildId = target->queryGuildable()->getGuildId();
    if (isValidGuildId(guildId)) {
        player_.getController().queryGuildCallback()->addCheatGuildExp(static_cast<GuildExp>(value));        
    }
    else {
        return ecGuildIsNotMember;
    }

    return ecOk;

}


ErrorCode CheatCommander::setPlayerStatus(CheatValueType type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    go::Entity* target = nullptr;
    uint32_t value = 0;
    const ErrorCode errorCode = parserPlayerValueTypeParams(target, value, player_, params);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    go::Cheatable* cheatable = target->queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }
    return cheatable->setPlayerStatus(type, value);
}


ErrorCode CheatCommander::downBuildTime(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 1) {
        return ecCheatInvalidParam;
    }

    const GameObjectInfo& targetInfo = player_.queryTargetSelectable()->getSelectedTargetInfo();
    if (! targetInfo.isBuilding()) {
        return ecCheatInvalidTarget;
    }
    go::Building* building = ANCHOR_SERVICE->getBuilding(targetInfo);
    if (! building) {
        return ecCheatNotFindTarget;
    }

    sec_t downTime = boost::lexical_cast<sec_t>(params[0]);
    
    return building->downBuildTime(downTime);
}


ErrorCode CheatCommander::updateBuildingHp(ValueChangetype type, const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 1) {
        return ecCheatInvalidParam;
    }

    const GameObjectInfo& targetInfo = player_.queryTargetSelectable()->getSelectedTargetInfo();
    if (! targetInfo.isBuilding()) {
        return ecCheatInvalidTarget;
    }
    go::Building* building = ANCHOR_SERVICE->getBuilding(targetInfo);
    if (! building) {
        return ecCheatNotFindTarget;
    }

    HitPoint hitPoint = toHitPoint(boost::lexical_cast<uint32_t>(params[0]));
    if (type == vctUp) {
        building->getStaticObjectStatus().fillHp(hitPoint);
        return ecOk;
    }
    else if (type == vctDown) {
        building->getStaticObjectStatus().reduceHp(hitPoint);
        return ecOk;
    }
    return ecCheatInvalidParam;
}


ErrorCode CheatCommander::addItem(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.empty() || params.size() > 2) {
        return ecCheatInvalidParam;
    }
    
    const DataCode itemCode = boost::lexical_cast<DataCode>(params[0]);
    if (! isExistItem(itemCode)) {
        return ecCheatInvalidParam;
    }
    uint8_t itemCount = 1;
    
    if (params.size() == 1) {
        itemCount = getStackItemCount(itemCode); 
    }
    else {
        itemCount = uint8_t(boost::lexical_cast<uint32_t>(params[1]));
        itemCount = itemCount > getStackItemCount(itemCode) ?
            getStackItemCount(itemCode) : itemCount;  
    }

    go::Cheatable* cheatable = player_.queryCheatable();
    if (! cheatable) {
        return ecServerInternalError;
    }

    return cheatable->addCheatItem(itemCode, itemCount);
}


ErrorCode CheatCommander::addSkill(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 1) {
        return ecCheatInvalidParam;
    }

    const SkillCode skillCode = boost::lexical_cast<SkillCode>(params[0]);
    return player_.queryCheatable()->addCheatSkill(skillCode);
}


ErrorCode CheatCommander::addQuest(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 1) {
        return ecCheatInvalidParam;
    }

    const QuestCode questCode = boost::lexical_cast<QuestCode>(params[0]);
    return player_.queryCheatable()->addCheatQuest(questCode);

}


ErrorCode CheatCommander::addBan(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (! (params.size() == 5 || params.size() == 6)) {
        return ecCheatInvalidParam;
    }

    BanInfo banInfo;
    if (L"character" == params[0]) {
        banInfo.banMode_ = bmCharacter;
        banInfo.characterId_ = boost::lexical_cast<ObjectId>(params[1]);
    }
    else if (L"account" == params[0]) {
        banInfo.banMode_ = bmAccount;
        banInfo.accountId_ = boost::lexical_cast<AccountId>(params[1]);
    }
    else if (L"ip" == params[0]) {
        banInfo.banMode_ = bmIpAddress;
        //banInfo.ip_ = boost::lexical_cast<std::string>(params[1]);
    }
    else {
        return ecCheatInvalidParam;
    }

    if (L"reason" == params[2]) {
        banInfo.reason_ = params[3];
    }


    sec_t now = getTime();
    if (L"hour" == params[4]) {
        sec_t value = boost::lexical_cast<sec_t>(params[5]);
        if (value < 0) {
            return ecCheatInvalidParam;
        }
        banInfo.banExpireTime_ = now + (60 * 60 * value);
    }
    else if (L"day" == params[4]) {		
        sec_t value = boost::lexical_cast<sec_t>(params[5]);
        if (value < 0) {
            return ecCheatInvalidParam;
        }
        banInfo.banExpireTime_ = now + (60 * 60 * 24 * value);
    }
    else if (L"unlimit" == params[4]) {
        banInfo.banExpireTime_ = 0;
    }
    else {
        return ecCheatInvalidParam;
    }

    ZONE_SERVICE->getLoginServerProxy().z2l_evAddBanInfo(banInfo);

    return ecOk;
}


ErrorCode CheatCommander::addBuildingMaterials()
{
    const GameObjectInfo& targetInfo = player_.queryTargetSelectable()->getSelectedTargetInfo();
    if (! targetInfo.isBuilding()) {
        return ecCheatInvalidTarget;
    }
    go::Building* building = ANCHOR_SERVICE->getBuilding(targetInfo);
    if (! building) {
        return ecCheatNotFindTarget;
    }
    return building->addBuildingMaterials();
}


ErrorCode CheatCommander::addCharacterTitle(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 1) {
        return ecCheatInvalidParam;
    }

    const CharacterTitleCode titleCode = boost::lexical_cast<CharacterTitleCode>(params[0]);
    if (! isValidCharacterTitleCode(titleCode)) {
        return ecCheatInvalidParam;
    }
    return player_.queryCheatable()->addCheatCharacterTitle(titleCode);
}


ErrorCode CheatCommander::removeQuest(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 1) {
        return ecCheatInvalidParam;
    }

    const QuestCode questCode = boost::lexical_cast<QuestCode>(params[0]);
    return player_.queryCheatable()->removeCheatQuest(questCode);
}


ErrorCode CheatCommander::removeBan(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 2) {
        return ecCheatInvalidParam;
    }

    BanMode banMode;
    AccountId accountId = invalidAccountId;
    ObjectId characterId = invalidObjectId;
    std::string ipAddress;

    if (L"character" == params[0]) {
        banMode = bmCharacter;
        characterId = boost::lexical_cast<ObjectId>(params[1]);
    }
    else if (L"account" == params[0]) {
        banMode = bmAccount;
        accountId = boost::lexical_cast<AccountId>(params[1]);
    }
    else if (L"ip" == params[0]) {
        banMode = bmIpAddress;
        //ipAddress = boost::lexical_cast<std::string>(params[1]);
    }
    else {
        return ecCheatInvalidParam;
    }

    ZONE_SERVICE->getLoginServerProxy().z2l_evRemoveBanInfo(banMode, accountId, characterId, ipAddress);

    return ecOk;
}


ErrorCode CheatCommander::toPlayer(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    Nickname nickname;
    if (! params.empty()) {
        nickname = params[0]; 
    }

    go::Entity* target = getApplyPlayer(player_, nickname);
    if (! target) {
        return ecCheatNotFindTarget;
    }
    
    return player_.queryCheatable()->toCheatPlayer(*target);
}


ErrorCode CheatCommander::toMe(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    Nickname nickname;
    if (! params.empty()) {
        nickname = params[0]; 
    }

    go::Entity* target = getApplyPlayer(player_, nickname);
    if (! target) {
        return ecCheatNotFindTarget;
    }

    return player_.queryCheatable()->toCheatMe(*target);
}


ErrorCode CheatCommander::toPosition(const Params& params)
{
    if (agModerator > accountGrade_) {
        return ecCheatNotEnoughGrade;
    }

    if (params.size() != 3) {
        return ecCheatInvalidParam;
    }

    Position target;
    target.x_ = -boost::lexical_cast<float32_t>(params[0]);
    target.y_ = boost::lexical_cast<float32_t>(params[1]);
    target.z_ = boost::lexical_cast<float32_t>(params[2]);

    return player_.queryCheatable()->toCheatPosition(target);
}

}} // namespace gideon { namespace zoneserver {
