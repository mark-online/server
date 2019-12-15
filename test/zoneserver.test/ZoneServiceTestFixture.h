#pragma once

#include "MockZoneClientSessionCallback.h"
#include <gideon/servertest/DatabaseTestFixture.h>
#include <gideon/cs/shared/data/AccountId.h>

namespace sne { namespace base {
class IoContextTask;
}} // namespace sne { namespace base {

namespace sne { namespace base {
class TaskScheduler;
}} // namespace sne { namespace base {

namespace gideon { namespace zoneserver {
class ZoneService;
class ZoneUser;
class GameTimer;
class ItemIdGenerator;
}} // namespace gideon { namespace zoneserver {

using namespace sne;
using namespace gideon;

class MockZoneLoginServerProxy;
class MockZoneCommunityServerProxy;
class MockZonePartyServerProxy;
class MockZoneUser;
class MockPlayerController;
class MockPlayerTradeController;
class MockPlayerMoveController;
class MockPlayerSkillController;
class MockPlayerBankController;
class MockPlayerQuestController;
class MockPlayerInventoryController;
class MockPlayerHarvestController;
class MockPlayerTreasureController;
class MockPlayerGraveStoneController;
class MockPlayerItemController;
class MockPlayerPartyController;
class MockPlayerAnchorController;
class MockPlayerNpcController;
class MockPlayerCastController;
class MockPlayerEffectController;
class MockGameTimerSource;
class MockPlayerMailController;
class MockPlayerAuctionController;
class MockPlayerBuildingController;
class MockPlayerTeleportController;
class MockPlayerTeleportController;

/**
* @class ZoneServiceTestFixture
*
* 존 서비스 테스트 Fixture
*/
class ZoneServiceTestFixture : public servertest::DatabaseTestFixture
{
public:
    enum {
        /// 존 서버에 입장 가능한 최대 인원 수
        maxZoneServerUserCount = 10
    };

    ZoneServiceTestFixture();
    virtual ~ZoneServiceTestFixture();

protected:
    void SetUp() override;
    void TearDown() override;

protected:
    void setValidAccount(AccountId accountId);

    ObjectId createCharacter(AccountId accountId);
    void loginUser(AccountId accountId);

    std::wstring makeId(AccountId accountId) const {
        return core::formatString(L"test%u", accountId);
    }

    std::wstring makeNickname(AccountId accountId) const {
        return makeId(accountId);
    }

    std::string makePassword(AccountId accountId) const {
        return core::formatString("test%u", accountId);
    }

    MockZoneLoginServerProxy& getLoginServerProxy();
    MockZoneCommunityServerProxy& getCommunityServerProxy();

protected:
    MockZoneUser* getMockZoneUser(AccountId accountId);

    MockPlayerController& getMockPlayerController(MockZoneUser& user);
    MockPlayerMoveController& getMockPlayerMoveController(MockZoneUser& user);
    MockPlayerSkillController& getMockPlayerSkillController(MockZoneUser& user);
    MockPlayerBankController& getMockPlayerBankController(MockZoneUser& user);
    MockPlayerInventoryController& getMockPlayerInventoryController(MockZoneUser& user);
    MockPlayerQuestController& getMockPlayerQuestController(MockZoneUser& user);
    MockPlayerTradeController& getMockPlayerTradeController(MockZoneUser& user);
    MockPlayerHarvestController& getMockPlayerHarvestController(MockZoneUser& user);
    MockPlayerTreasureController& getMockPlayerTreasureController(MockZoneUser& user);
    MockPlayerGraveStoneController& getMockPlayerGraveStoneController(MockZoneUser& user);
    MockPlayerItemController& getMockPlayerItemController(MockZoneUser& user);
    MockPlayerPartyController& getMockPlayerPartyController(MockZoneUser& user);
    MockPlayerAnchorController& getMockPlayerAnchorController(MockZoneUser& user);
    MockPlayerNpcController& getMockPlayerNpcController(MockZoneUser& user);
    MockPlayerCastController& getMockPlayerCastController(MockZoneUser& user);
    MockPlayerEffectController& getMockPlayerEffectController(MockZoneUser& user);
    MockPlayerMailController& getMockPlayerMailController(MockZoneUser& user);
    MockPlayerAuctionController& getMockPlayerAuctionController(MockZoneUser& user);
	MockPlayerTeleportController& getMockPlayerTeleportController(MockZoneUser& user);

protected:
    zoneserver::ZoneService* zoneService_;

    MockZoneClientSessionCallback sessionCallback_;
    AccountId accountId1_;

    MockGameTimerSource* mockGameTimerSource_;

private:
    std::unique_ptr<zoneserver::GameTimer> gameTimer_;
    std::unique_ptr<sne::base::IoContextTask> ioContextTask_;
    std::unique_ptr<sne::base::TaskScheduler> taskScheduler_;
    std::unique_ptr<zoneserver::ItemIdGenerator> itemIdGenerator_;
};
