#include "ZoneServerPCH.h"
#include "StaticObject.h"
#include "status/StaticObjectStatus.h"
#include "../../controller/EntityController.h"

namespace gideon { namespace zoneserver { namespace go {


StaticObject::StaticObject(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller))
{
}


StaticObject::~StaticObject()
{
}


bool StaticObject::initialize(ObjectType ot, ObjectId objectId)
{
    std::lock_guard<LockType> lock(getLock());

    if (!Parent::initialize(ot, objectId)) {
        return false;
    }

    staticObjectStatus_ = std::make_unique<StaticObjectStatus>(*this, staticObjectStatusInfo_);
    return true;
}


void StaticObject::initStaticObjectStatusInfo(const StaticObjectStatusInfo& staticObjectStatusInfo)
{
    staticObjectStatusInfo_ = staticObjectStatusInfo;
}

}}} // namespace gideon { namespace zoneserver { namespace go {
