#pragma once

#include "StateBrain.h"

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @class BuildingStateBrain
 */
class BuildingStateBrain : public StateBrain
{
public:
    BuildingStateBrain(go::Entity& owner);
    virtual ~BuildingStateBrain();

private:
    // = StateBrain overriding
    virtual void initialize();
    virtual void finalize();

    //virtual void spawned();
    virtual void attacked(const go::Entity& attacker);
    virtual void died();

    virtual AggroList* getAggroList() {
        return aggroList_.get();
    }

private:
    // = Stateable overriding
    virtual void asyncHandleEvent(int /*eventId*/) {}
    virtual void handleEvent(int /*eventId*/) {}

    virtual bool isInState(int /*stateId*/) const {
        return false;
    }
    virtual BrainState& getEntryState() {
        return *globalState_;
    }
    virtual BrainState* getState(int /*stateId*/) {
        return nullptr;
    }

private:
    bool shouldStrikeBack(const go::Entity& attacker) const;

private:
    std::unique_ptr<BrainState> globalState_;
    std::unique_ptr<AggroList> aggroList_;
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
