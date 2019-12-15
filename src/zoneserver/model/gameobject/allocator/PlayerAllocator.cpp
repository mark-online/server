#include "ZoneServerPCH.h"
#include "PlayerAllocator.h"
#include "../Player.h"
#include "../../../controller/PlayerController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* PlayerAllocator::malloc()
{
    return new Player(
        std::make_unique<gc::PlayerController>());
}


void PlayerAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::Player*>(entity));
}

}}} // namespace gideon { namespace zoneserver { namespace go {
