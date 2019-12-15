#pragma once

#include "Entity.h"
#include <gideon/cs/shared/data/StaticObjectStatusInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {

class StaticObjectEffectScriptApplier;
class StaticObjectStatus;

/**
 * @class StaticObject
 * 생명체가 아닌 것
 */
class ZoneServer_Export StaticObject : public Entity
{
    typedef Entity Parent;

public:
	StaticObject(std::unique_ptr<gc::EntityController> controller);
	virtual ~StaticObject();
    
    virtual bool initialize(ObjectType ot, ObjectId objectId);
    
public:
    virtual bool isInvincible() const { return false; }
    virtual bool canDestory() const { return false; }

    
protected:
    void initStaticObjectStatusInfo(const StaticObjectStatusInfo& staticObjectStatusInfo);

public:
    StaticObjectStatus& getStaticObjectStatus() {
        return *staticObjectStatus_.get();
    }

    const StaticObjectStatus& getStaticObjectStatus() const {
        return *staticObjectStatus_.get();
    }

private:
    std::unique_ptr<StaticObjectStatus> staticObjectStatus_;
    StaticObjectStatusInfo staticObjectStatusInfo_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
