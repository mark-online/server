#include "ZoneServerPCH.h"
#include "CheatService.h"
#include "../arena/ArenaService.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/AbstractAnchor.h"
#include "../../model/gameobject/ability/Cheatable.h"
#include "../../model/gameobject/ability/Networkable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/BotCommandCallback.h"
#include "../../controller/callback/BuildingStateCallback.h"
#include "../../helper/InventoryHelper.h"
#include "../../service/anchor/AnchorService.h"
#include "../../service/arena/ArenaService.h"
#include "../../world/World.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneArenaServerProxy.h"
#include <sne/core/utility/Unicode.h>
#include <boost/lexical_cast.hpp>

namespace gideon { namespace zoneserver {

/**
 * @class BotMoveEvent
 */
class BotMoveEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BotMoveEvent>
{
public:
    BotMoveEvent(const Position& position) :
        position_(position) {}

private:
    virtual void call(go::Entity& entity) {
        gc::BotCommandCallback* callback = entity.getController().queryBotCommandCallback();
        if (callback != nullptr) {
            callback->commandMoved(position_);
        }
    }

private:
    const Position position_;
};


/**
 * @class BotSkillCastEvent
 */
class BotSkillCastEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<BotSkillCastEvent>
{
public:
    BotSkillCastEvent() {}

private:
    virtual void call(go::Entity& entity) {
        gc::BotCommandCallback* callback = entity.getController().queryBotCommandCallback();
        if (callback != nullptr) {
            callback->commandSkillCasted();
        }
    }
};


void CheatService::execute(go::Cheatable& cheatable, 
    ObjectId ownerId, const wstring_t& command, const wstring_t& params)
{
	if (! cheatable.canCheatable()) {
		return;
	}

    try {
        if (command == L"/additem") {
            const size_t find_pos = params.find_first_of(L" ");
            const string_t param1 = sne::core::toUtf8(params.substr(0, find_pos));
            const string_t param2 = sne::core::toUtf8(params.substr(find_pos + 1));
            const DataCode itemCode = boost::lexical_cast<DataCode>(param1);
            const uint8_t itemCount = uint8_t(boost::lexical_cast<uint32_t>(param2));
            const uint8_t revisionItemCount = itemCount > getStackItemCount(itemCode) ?
                getStackItemCount(itemCode) : itemCount;    
            const CodeType ct = getCodeType(itemCode);
            if (isItemType(ct)) {
                cheatable.cheatAddItem(itemCode, revisionItemCount);
            }
        }
        else if (command == L"/move") {
            const size_t find_pos = params.find_first_of(L" ");
            const wstring_t param1 = params.substr(0, find_pos);
            const wstring_t param2 = params.substr(find_pos + 1);
            const Position position(boost::lexical_cast<float32_t>(param1),
                boost::lexical_cast<float32_t>(param2), 0.0f);
            BotMoveEvent::Ref event(new BotMoveEvent(position));
            WORLD->broadcast(event);
            // 브로드 캐스팅                                   
        }
        else if (command == L"/castskill") {
            BotSkillCastEvent::Ref event(new BotSkillCastEvent());
            WORLD->broadcast(event);
        }
        else if (command == L"/fullpoints") {
            cheatable.cheatFullCharacterPoints();
        }
        else if (command == L"/rewardexp") {
            const ExpPoint rewardExp = toExpPoint(boost::lexical_cast<int>(params));
            cheatable.cheatRewardExp(rewardExp < ceMin ? ceMin: rewardExp);
        }
        else if (command == L"/showmethemoney") {
            const GameMoney money = boost::lexical_cast<GameMoney>(params);
            cheatable.cheatUpGameMoney(money < 0 ? 0 : money);
        }
        else if (command == L"/addskill") {
            const SkillCode skillCode = boost::lexical_cast<SkillCode>(params);
            cheatable.cheatAddSkill(skillCode);
        }
        else if (command == L"/recall") {
            const size_t find_pos = params.find_first_of(L" ");
            const Nickname nickname = params.substr(0, find_pos);
            cheatable.cheatRecall(nickname);            
        }
        else if (command == L"/toplayer") {
            const size_t find_pos = params.find_first_of(L" ");
            const Nickname nickname = params.substr(0, find_pos);
            cheatable.cheatTeleportToPlayer(nickname);            
        }
        else if (command == L"/toposition") {
            wstring_t tempParams = params;
            size_t find_pos = params.find_first_of(L" ");
            const wstring_t param1 = tempParams.substr(0, find_pos);
            tempParams = tempParams.substr(find_pos + 1);
            find_pos = tempParams.find_first_of(L" ");
            const wstring_t param2 = tempParams.substr(0, find_pos);
            tempParams = tempParams.substr(find_pos + 1);
            const wstring_t param3 = tempParams.substr(0, find_pos);
            const float32_t positionX = boost::lexical_cast<float32_t>(param1);
            const float32_t positionY = boost::lexical_cast<float32_t>(param2);
            const float32_t positionZ = boost::lexical_cast<float32_t>(param3);
            cheatable.cheatTeleportToPosition(Position(positionX, positionY, positionZ));            
        }
        else if (command == L"/mercenarypoint") {
            const MercenaryPoint mercenaryPoint = boost::lexical_cast<MercenaryPoint>(params);
            cheatable.cheatMercenaryPoint(mercenaryPoint < 0 ? 0 : mercenaryPoint);
        }
        else if (command == L"/logout") {
            const size_t find_pos = params.find_first_of(L" ");
            const Nickname nickname = params.substr(0, find_pos);
            go::Entity* player = WORLD->getPlayer(nickname);
            if (player) {
                player->queryNetworkable()->logout();
            }
        }
        else if (command == L"/arenapoint") {
            const ArenaPoint arenaPoint = boost::lexical_cast<ArenaPoint>(params);
            cheatable.cheatArenaPoint(arenaPoint < 0 ? 0 : arenaPoint);
        }
        else if (command == L"/releasedeserter") {
            if (ZONE_SERVICE->isArenaServer()) {
                ARENA_SERVICE->releaseDeserter(ownerId);
            }    
            else {
                ZONE_SERVICE->getArenaServerProxy().z2a_releaseDeserter(ownerId);
            }
        }
        else if (command == L"/who") {
            cheatable.cheatWhoIsInZone();
        }
        else if (command == L"/destorybuilding") {
            const ObjectId buildingId = boost::lexical_cast<ObjectId>(params);
            go::AbstractAnchor* abstractAnchor = ANCHOR_SERVICE->getAnchor(GameObjectInfo(otBuilding, buildingId));
            if (abstractAnchor) {
                gc::BuildingStateCallback* callback = abstractAnchor->getController().queryBuildingStateCallback();
                if (callback) {
                    callback->buildDestroyed();
                    static_cast<go::Entity*>(abstractAnchor)->despawn();
                }
            } 
        }
        else if (command == L"/destoryanchor") {
            const ObjectId buildingId = boost::lexical_cast<ObjectId>(params);
            go::AbstractAnchor* abstractAnchor = ANCHOR_SERVICE->getAnchor(GameObjectInfo(otAnchor, buildingId));
            if (abstractAnchor) {
                static_cast<go::Entity*>(abstractAnchor)->despawn();
            } 
        }		
		else if (command == L"/resetcooltime") {
			cheatable.cheatResetCooldown();
		}
		else if (command == L"/acceptquest") {
			const QuestCode questCode = boost::lexical_cast<QuestCode>(params);
			cheatable.cheatAcceptQuest(questCode);
		}
		else if (command == L"/forgeCoin") {
			const ForgeCoin forgeCoin = boost::lexical_cast<ForgeCoin>(params);
			cheatable.cheatForgeCoin(forgeCoin < 0 ? 0 : forgeCoin);
		}
    }
    catch (boost::bad_lexical_cast&) {
        SNE_LOG_ERROR("bad cheat key")
    }

}

}} // namespace gideon { namespace zoneserver {
