#include "ZoneServerTestPCH.h"
#include "ZoneServiceTestFixture.h"
#include "MockGameTimer.h"
#include "ZoneServer/service/time/GameTimer.h"

using namespace gideon;
using namespace gideon::zoneserver;

/**
* @class GameTimerTest
*
*/
class GameTimerTest : public ZoneServiceTestFixture
{
};


TEST_F(GameTimerTest, testSetTime)
{
    const GameTime begin = GAME_TIMER->msec();

    mockGameTimerSource_->set(begin + 100);

    ASSERT_EQ(begin + 100, GAME_TIMER->msec());
}
