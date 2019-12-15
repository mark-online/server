#pragma once

#include "ZoneServiceTestFixture.h"
#include <gideon/cs/shared/data/StatusInfo.h>
#include <gideon/cs/shared/data/CastInfo.h>

using namespace gideon::zoneserver;

namespace gideon { namespace zoneserver { namespace go {
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {
/**
* @class GameTestFixture
*
* 게임 테스트 Fixture
*/
class GameTestFixture : public ZoneServiceTestFixture
{ 
protected:
    virtual void SetUp();

protected:
    void killPlayer2();

    StartCastInfo createUnionSkillCastToInfo(const GameObjectInfo& info, SkillCode skillCode);
    StartCastInfo createUnionSkillCastAtInfo(const Position& info, SkillCode skillCode);
    StartCastInfo createUnionItemCastToInfo(const GameObjectInfo& info, ObjectId objectId);
    StartCastInfo createUnionItemCastAtInfo(const Position& info, ObjectId objectId);


protected:
    ObjectId characterId1_;
    MockZoneUser* user1_;
    zoneserver::go::Player* player1_;
    MockPlayerController* playerController1_;
    MockPlayerMoveController* playerMoveController1_;
    MockPlayerSkillController* playerSkillController1_;
    MockPlayerInventoryController* playerInventoryController1_;
    MockPlayerQuestController* playerQuestController1_;
    MockPlayerTradeController* playerTradeController1_;
    MockPlayerHarvestController* playerHarvestController1_;
    MockPlayerTreasureController* playerTreasureController1_;
    MockPlayerGraveStoneController* playerGraveStoneController1_;
    MockPlayerItemController* playerItemController1_;
    MockPlayerPartyController* playerPartyController1_;
    MockPlayerAnchorController* playerAnchorController1_;
    MockPlayerBuildingController* playerBuildingController1_;
    MockPlayerNpcController* playerNpcController1_;
    MockPlayerCastController* playerCastController1_;
    MockPlayerEffectController* playerEffectController1_;
    MockPlayerMailController* playerMailController1_;
    MockPlayerAuctionController* playerAuctionController1_;
	MockPlayerTeleportController* playerTeleportController1_;


    AccountId accountId2_;
    ObjectId characterId2_;
    MockZoneUser* user2_;
    zoneserver::go::Player* player2_;
    MockPlayerController* playerController2_;
    MockPlayerMoveController* playerMoveController2_;
    MockPlayerSkillController* playerSkillController2_;
    MockPlayerInventoryController* playerInventoryController2_;
    MockPlayerQuestController* playerQuestController2_;
    MockPlayerTradeController* playerTradeController2_;
    MockPlayerHarvestController* playerHarvestController2_;
    MockPlayerTreasureController* playerTreasureController2_;
    MockPlayerGraveStoneController* playerGraveStoneController2_;
    MockPlayerItemController* playerItemController2_;
    MockPlayerPartyController* playerPartyController2_;
    MockPlayerAnchorController* playerAnchorController2_;
    MockPlayerBuildingController* playerBuildingController2_;
    MockPlayerNpcController* playerNpcController2_;
    MockPlayerCastController* playerCastController2_;
    MockPlayerEffectController* playerEffectController2_;
    MockPlayerMailController* playerMailController2_;
    MockPlayerAuctionController* playerAuctionController2_;
	MockPlayerTeleportController* playerTeleportController2_;

    int player2KillCount_;
    HitPoint playerHp_;
    ManaPoint playerMp_;
};


// 주의! 같은 equip-code를 가진 아이템이 여러개 있을 경우 문제가 될 수 있다
inline const ItemInfo* getItemInfo(const InventoryInfo& inven, EquipCode equipCode)
{
    for (const ItemMap::value_type& value : inven.items_) {
        const ItemInfo& item = value.second;
        if (item.itemCode_ == equipCode) {
            return &item;
        }
    }
    return nullptr;
}
