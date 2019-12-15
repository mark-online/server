#pragma once

#include "ZoneServer/controller/NpcController.h"
#include <sne/test/CallCounter.h>

using namespace gideon::zoneserver;

/**
 * @class MockNpcController
 *
 * 테스트 용 mock NpcController
 */
class MockNpcController :
    public gc::NpcController,
    public sne::test::CallCounter
{
public:
    MockNpcController() :
        lastErrorCode_(ecWhatDidYouTest) {}

private:
    // = NpcController overriding
    virtual void entitiesAppeared(const go::EntityMap& entities) {
        gc::NpcController::entitiesAppeared(entities);
        addCallCount("entitiesAppeared");
    }

    virtual void entityAppeared(go::Entity& entity, const UnionEntityInfo& entityInfo) {
        gc::NpcController::entityAppeared(entity, entityInfo);
        addCallCount("entityAppeared");
    }

    virtual void entitiesDisappeared(const go::EntityMap& entities) {
        gc::NpcController::entitiesDisappeared(entities);
        addCallCount("entitiesDisappeared");
    }

    virtual void entityDisappeared(go::Entity& entity) {
        gc::NpcController::entityDisappeared(entity);
        addCallCount("entityDisappeared");
    }

public:
    ErrorCode lastErrorCode_;
};
