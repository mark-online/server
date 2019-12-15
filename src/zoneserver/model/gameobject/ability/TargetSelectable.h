#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class TargetSelectable
 * 대상을 선택할 수 있다
 */
class TargetSelectable
{
public:
    typedef sne::core::Set<GameObjectInfo> EntitySet;

public:
    virtual ~TargetSelectable() {}
	
public:
    /// @return target of target
    virtual const go::Entity* selectTarget(const GameObjectInfo& targetInfo) = 0;

    virtual void unselectTarget() = 0;

    virtual go::Entity* getSelectedTarget() = 0;

    virtual const GameObjectInfo& getSelectedTargetInfo() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {

