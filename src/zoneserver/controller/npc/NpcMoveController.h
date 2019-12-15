#pragma once

#include "../../zoneserver_export.h"
#include "../MoveController.h"
#include "../../model/gameobject/Entity.h"
#include <gideon/cs/shared/data/Time.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class NpcMoveController
 * NPC 이동을 담당(AI)
 */
class ZoneServer_Export NpcMoveController :
    public MoveController
{
public:
    NpcMoveController(go::Entity* owner);

    void chaseTo(const go::Entity& target);

    void headTo(const go::Entity& target);

public:
    // = MoveController overriding
    virtual void initialize();
    virtual void finalize();

protected:
	// = MovementCallback overriding
	virtual void entityGoneToward(const GameObjectInfo& entityInfo,
		const ObjectPosition& position);
	virtual void entityMoved(const GameObjectInfo& entityInfo,
		const ObjectPosition& position);
	virtual void entityStopped(const GameObjectInfo& entityInfo,
		const ObjectPosition& position);
    virtual void entityTeleported(const GameObjectInfo& entityInfo,
        const ObjectPosition& position);
	virtual void entityTurned(const GameObjectInfo& entityInfo,
		Heading heading);
	virtual void entityJumped(const GameObjectInfo& entityInfo,
		const ObjectPosition& position);
    virtual void entityRun(const GameObjectInfo& entityInfo);
    virtual void entityWalked(const GameObjectInfo& entityInfo);

private:
    float32_t minApproachDistance_; //< 최소 접근 거리(최소한 이 거리를 유지한다)
    float32_t maxApproachDistance_; //< 최대 접근 거리(최소한 이 거리를 유지한다)
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
