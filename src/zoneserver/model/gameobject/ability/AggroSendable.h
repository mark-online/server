#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class AggroSendable
 * 설치물을 생성 있다
 */
class AggroSendable
{
public:
    virtual ~AggroSendable() {}
	
public:
	virtual void setAggroTarget(const GameObjectInfo& targetInfo, go::Entity& entity) = 0;

	virtual void removeAggroTarget(const GameObjectInfo& targetInfo) = 0;

    virtual void updateTagetThreat(const GameObjectInfo& sender, uint32_t aggroValue, bool isDivision, bool isRemove) = 0;

    virtual void updateAdditionThreat(int32_t threat, bool isRemove) = 0;

    virtual uint32_t getApplyThreat(uint32_t threat) = 0;
	
};

}}} // namespace gideon { namespace zoneserver { namespace go {
