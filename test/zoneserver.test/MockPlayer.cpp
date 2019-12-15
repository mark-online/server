#include "ZoneServerTestPCH.h"
#include "MockPlayer.h"
#include "MockPlayerMoveController.h"
#include "MockPlayerCastController.h"
#include "MockPlayerTeleportController.h"
#include "MockPlayerSkillController.h"
#include "MockPlayerItemController.h"
#include "MockPlayerGraveStoneController.h"
#include "MockPlayerQuestController.h"
#include "MockPlayerTradeController.h"
#include "MockPlayerInventoryController.h"
#include "MockPlayerTreasureController.h"
#include "MockPlayerHarvestController.h"
#include "MockPlayerPartyController.h"
#include "MockPlayerAnchorController.h"
#include "MockPlayerBuildingController.h"
#include "MockPlayerNpcController.h"
#include "MockPlayerEffectController.h"
#include "MockPlayerMailController.h"
#include "MockPlayerAuctionController.h"
#include "ZoneServer\controller\EntityController.h"

MockPlayer::MockPlayer(std::unique_ptr<zoneserver::gc::EntityController> controller) :
    Player(std::move(controller))
{
}


std::unique_ptr<zoneserver::gc::MoveController> MockPlayer::createMoveController()
{
    return std::make_unique<MockPlayerMoveController>(this);
}


std::unique_ptr<zoneserver::gc::SkillController> MockPlayer::createSkillController() 
{
    return std::make_unique<MockPlayerSkillController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerGraveStoneController> MockPlayer::createPlayerGraveStoneController()
{
    return std::make_unique<MockPlayerGraveStoneController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerInventoryController> MockPlayer::createPlayerInventoryController()
{
    return std::make_unique<MockPlayerInventoryController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerQuestController> MockPlayer::createPlayerQuestController()
{
    return std::make_unique<MockPlayerQuestController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerTradeController> MockPlayer::createPlayerTradeController()
{
    return std::make_unique<MockPlayerTradeController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerHarvestController> MockPlayer::createPlayerHarvestController()
{
    return std::make_unique<MockPlayerHarvestController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerTreasureController> MockPlayer::createPlayerTreasureController()
{
    return std::make_unique<MockPlayerTreasureController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerItemController> MockPlayer::createPlayerItemController()
{
    return std::make_unique<MockPlayerItemController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerPartyController> MockPlayer::createPlayerPartyController()
{
    return std::make_unique<MockPlayerPartyController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerAnchorController> MockPlayer::createPlayerAnchorController()
{
    return std::make_unique<MockPlayerAnchorController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerBuildingController> MockPlayer::createPlayerBuildingController()
{
    return std::make_unique<MockPlayerBuildingController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerNpcController> MockPlayer::createPlayerNpcController()
{
    return std::make_unique<MockPlayerNpcController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerCastController> MockPlayer::createPlayerCastController()
{
    return std::make_unique<MockPlayerCastController>(this);
}


std::unique_ptr<zoneserver::gc::CreatureEffectController> MockPlayer::createEffectController()
{
    return std::make_unique<MockPlayerEffectController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerMailController> MockPlayer::createPlayerMailController()
{
    return std::make_unique<MockPlayerMailController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerAuctionController> MockPlayer::createPlayerAuctionController()
{
    return std::make_unique<MockPlayerAuctionController>(this);
}


std::unique_ptr<zoneserver::gc::PlayerTeleportController> MockPlayer::createPlayerTeleportController()
{
	return std::make_unique<MockPlayerTeleportController>(this);
}

ErrorCode MockPlayer::spawn(zoneserver::WorldMap& worldMap, const ObjectPosition& spawnPosition)
{
    return Creature::spawn(worldMap, spawnPosition);
}
