#pragma once

#include "StaticObjectController.h"
#include "callback/EffectCallback.h"


namespace gideon { namespace zoneserver {
class Skill;
}} // namespace gideon { namespace zoneserver 


namespace gideon { namespace zoneserver { namespace go {
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class GraveStoneController
 * Controller for GraveStone
 */
class ZoneServer_Export GraveStoneController : public StaticObjectController
{  
protected:
        // = EntityControllerAbility overriding
    virtual EffectCallback* queryEffectCallback() { return this; }

private:
	virtual void spawned(WorldMap& worldMap);
    virtual void despawned(WorldMap& worldMap);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {