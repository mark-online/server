#include "ZoneServerTestPCH.h"
#include "MockPlayerAllocator.h"
#include "MockPlayer.h"
#include "MockPlayerController.h"


gideon::zoneserver::go::Entity* MockPlayerAllocator::malloc()
{
    return new MockPlayer(std::make_unique<MockPlayerController>());
}


void MockPlayerAllocator::free(gideon::zoneserver::go::Entity* entity)
{
    boost::checked_delete(static_cast<MockPlayer*>(entity));
}
