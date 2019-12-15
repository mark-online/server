#include "ZoneServerPCH.h"
#include "../GameScriptManager.h"
#include "../../ScriptedBrain.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/ability/Knowable.h"
#include "../../../controller/npc/NpcMoveController.h"
#include "../../../service/movement/MovementManager.h"
#include "../../../service/time/GameTimer.h"
#include <gideon/cs/datatable/ActionTable.h>

namespace gideon { namespace zoneserver { namespace ai {

namespace {

/**
 * @class MedicNpcBrain
 */
class MedicNpcBrain :
    public ScriptedBrain,
    private go::Knowable::EnumerateCommand
{
    enum State
    {
        stateNone = 0,
        stateSeek = 1,
        stateMove = 2,
        stateCure = 3
    };

    typedef sne::core::Vector<const go::Entity*> Candidates;

public:
    MedicNpcBrain(go::Entity& owner, NpcCode woundedNpcCode1, NpcCode woundedNpcCode2,
        datatable::ActionCode curerActionCode, datatable::ActionCode cureeActionCode) :
        ScriptedBrain(owner),
        woundedNpcCode1_(woundedNpcCode1),
        woundedNpcCode2_(woundedNpcCode2),
        currentState_(stateNone) {
        curerActionTemplate_ = ACTION_TABLE->getAction(curerActionCode);
        if (! curerActionTemplate_) {
            throw std::runtime_error("MedicNpcBrain - Can't find Curer Action");
        }
        cureeActionTemplate_ = ACTION_TABLE->getAction(cureeActionCode);
        if (! cureeActionTemplate_) {
            throw std::runtime_error("MedicNpcBrain - Can't find Curee Action");
        }

        candidates_.reserve(5);
    }

private:
    // = ScriptedBrain overriding
    virtual void activate() {
        ScriptedBrain::activate();

        startToSeek();
    }

    virtual void analyze(GameTime diff) {
        if (! isActivated()) {
            return;
        }

        switch (currentState_) {
        case stateNone:
            break;
        case stateSeek:
            seekInterval_.update(diff);
            if (seekInterval_.isPassed()) {
                candidates_.clear();
                (void)getOwner().queryKnowable()->enumerate(*this);
                if (candidates_.empty()) {
                    startToSeek();
                }
                else {
                    const go::Entity* target = candidates_[esut::random() % candidates_.size()];
                    moveTo(*target);
                }
            }
            break;
        case stateMove:
            {
                go::Npc& npc = static_cast<go::Npc&>(getOwner());
                if (npc.getNpcMoveController().isArrivedAtDestination()) {
                    cure();
                }
            }
            break;
        case stateCure:
            cureInterval_.update(diff);
            if (cureInterval_.isPassed()) {
                startToSeek();
            }
            break;
        }
    }

private:
    // = go::Knowable::EnumerateCommand overriding
    virtual bool execute(go::Entity& entity) {
        const DataCode entityCode = entity.getEntityCode();
        if ((entityCode == woundedNpcCode1_) || (entityCode == woundedNpcCode2_)) {
            candidates_.push_back(&entity);
        }
        return false;
    }

private:
    void startToSeek() {
        currentState_ = stateSeek;
        seekInterval_.reset(esut::random(5000, 10000));
    }

    void moveTo(const go::Entity& entity) {
        currentState_ = stateMove;

        go::Npc& npc = static_cast<go::Npc&>(getOwner());
        npc.queryTargetSelectable()->selectTarget(entity.getGameObjectInfo());
        npc.queryMoveable()->getMovementManager().setChaseMovement();
    }

    void cure() {
        currentState_ = stateCure;

        cureInterval_.reset(curerActionTemplate_->play_time());
        getOwner().playAction(curerActionTemplate_->action_code());

        go::Npc& npc = static_cast<go::Npc&>(getOwner());
        go::Entity* woundedNpc = npc.queryTargetSelectable()->getSelectedTarget();
        if (woundedNpc != nullptr) {
            woundedNpc->playAction(cureeActionTemplate_->action_code());
        }
    }

private:
    const NpcCode woundedNpcCode1_;
    const NpcCode woundedNpcCode2_;
    State currentState_;
    GameTimeTracker seekInterval_;
    GameTimeTracker cureInterval_;
    Candidates candidates_;

    const gdt::action_t* curerActionTemplate_;
    const gdt::action_t* cureeActionTemplate_;
};

} // namespace {


/**
 * @class MedicNpcScript
 * 랜덤하게 가장 가까운 부상자를 치료하는 애니를 연출한다
 */
class MedicNpcScript : public NpcScript
{
public:
    MedicNpcScript() :
        NpcScript("medic_npc_script") {}

private:
    virtual std::unique_ptr<Brain> createBrain(go::Npc& owner,
        uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4) {
        return std::make_unique<MedicNpcBrain>(owner, param1, param2, param3, param4);
    }
};


GameScriptConstructor<MedicNpcScript> s_medicNpcScriptConstructor;

}}} // namespace gideon { namespace zoneserver { namespace ai {
