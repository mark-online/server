#pragma once

#include "ZoneServer/model/gameobject/Player.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockMockPlayer
 *
 * 테스트 용 mock MockPlayer
 */
class MockPlayer :
    public zoneserver::go::Player
{
public:
    MockPlayer(std::unique_ptr<zoneserver::gc::EntityController> controller);

    virtual std::unique_ptr<zoneserver::gc::MoveController> createMoveController();
    virtual std::unique_ptr<zoneserver::gc::SkillController> createSkillController();
    virtual std::unique_ptr<zoneserver::gc::PlayerGraveStoneController> createPlayerGraveStoneController();
    virtual std::unique_ptr<zoneserver::gc::PlayerInventoryController> createPlayerInventoryController();
    virtual std::unique_ptr<zoneserver::gc::PlayerQuestController> createPlayerQuestController();
    virtual std::unique_ptr<zoneserver::gc::PlayerTradeController> createPlayerTradeController();
    virtual std::unique_ptr<zoneserver::gc::PlayerHarvestController> createPlayerHarvestController();
    virtual std::unique_ptr<zoneserver::gc::PlayerTreasureController> createPlayerTreasureController();
    virtual std::unique_ptr<zoneserver::gc::PlayerItemController> createPlayerItemController();
    virtual std::unique_ptr<zoneserver::gc::PlayerPartyController> createPlayerPartyController();
    virtual std::unique_ptr<zoneserver::gc::PlayerAnchorController> createPlayerAnchorController();
    virtual std::unique_ptr<zoneserver::gc::PlayerNpcController> createPlayerNpcController();
    virtual std::unique_ptr<zoneserver::gc::PlayerCastController> createPlayerCastController();
    virtual std::unique_ptr<zoneserver::gc::CreatureEffectController> createEffectController();
    virtual std::unique_ptr<zoneserver::gc::PlayerMailController> createPlayerMailController();
    virtual std::unique_ptr<zoneserver::gc::PlayerAuctionController> createPlayerAuctionController();
    virtual std::unique_ptr<zoneserver::gc::PlayerBuildingController> createPlayerBuildingController();
	virtual std::unique_ptr<zoneserver::gc::PlayerTeleportController> createPlayerTeleportController();

private:
    virtual ErrorCode spawn(zoneserver::WorldMap& worldMap, const ObjectPosition& spawnPosition);
};
