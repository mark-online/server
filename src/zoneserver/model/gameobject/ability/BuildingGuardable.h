#pragma once

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class BuildingGuardable
 * 건물을 지킬 수 있다(건물 경비병)
 */
class BuildingGuardable
{
public:
    virtual ~BuildingGuardable() {}
	
public:
    virtual void setMercenaryId(ObjectId mercenaryId) = 0;

public:
    virtual ObjectId getMercenaryId() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
