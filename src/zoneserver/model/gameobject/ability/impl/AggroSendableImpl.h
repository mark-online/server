#pragma once

#include "../AggroSendable.h"
#include "../../Entity.h"

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class AggroSendable
 * 어그로를 줄수있다 생성 있다
 */
class AggroSendableImpl : public AggroSendable
{
    typedef std::mutex LockType;
public:
    AggroSendableImpl(Entity& owner);
    virtual ~AggroSendableImpl() {}
	
private:
	virtual void setAggroTarget(const GameObjectInfo& targetInfo, go::Entity& entity);

	virtual void removeAggroTarget(const GameObjectInfo& targetInfo);

	virtual void updateTagetThreat(const GameObjectInfo& sender, uint32_t aggroValue, bool isDivision, bool isRemove);

    virtual void updateAdditionThreat(int32_t threat, bool isRemove);

    virtual uint32_t getApplyThreat(uint32_t threat);
private:
    mutable LockType lockAggroble_;
    Entity& owner_;
    EntityMap targetMap_; // 나를 타켓으로 한놈들
    int additionThreatPer_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
