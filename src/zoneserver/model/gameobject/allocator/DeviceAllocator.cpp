#include "ZoneServerPCH.h"
#include "DeviceAllocator.h"
#include "../Device.h"
#include "../../../controller/DeviceController.h"

namespace gideon { namespace zoneserver { namespace go {

go::Entity* DeviceAllocator::malloc()
{
    return new go::Device(
        std::make_unique<gc::DeviceController>());
}


void DeviceAllocator::free(go::Entity* entity)
{
    boost::checked_delete(static_cast<go::Device*>(entity));
}

}}} // namespace gideon { namespace zoneserver { namespace go {
