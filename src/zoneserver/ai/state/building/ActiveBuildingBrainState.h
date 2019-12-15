#pragma once

#include "../GlobalBrainState.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class ActiveBuildingBrainState
 * 본격적으로 활동하는 상태
 */
class ActiveBuildingBrainState : public GlobalBrainState
{
private:
    //virtual void entry(Brain& brain) {
    //}

    //virtual void exit(Brain& brain) {
    //}

    //virtual void update(Brain& brain, msec_t diff) {
    //}

private:
    // = BrainState overriding
    virtual bool handleEvent(Brain& brain, sne::core::fsm::Event& event) const {
        brain, event;
        return true; // 아무런 이벤트도 처리하지 않는다
    }

private:
    //go::Building* owner_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
