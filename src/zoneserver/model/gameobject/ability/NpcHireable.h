#pragma once


namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
* @class NpcHireable
 * NPC를 고용할수 있다.
 */
class NpcHireable
{
public:
    virtual ~NpcHireable() {}
	
public:
	virtual ErrorCode buyBuildingGuard(Entity& player, BuildingGuardCode code, uint8_t rendDay) = 0;
    virtual ErrorCode spawnBuildingGuard(Entity& player, ObjectId mercenaryId, const ObjectPosition& position) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
