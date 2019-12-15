#include "ZoneServerPCH.h"
#include "MovementManager.h"
#include "IdleMovement.h"
#include "WanderMovement.h"
#include "PathMovement.h"
#include "ReturnMovement.h"
#include "ChaseMovement.h"
#include "FleeingMovement.h"
#include "SeekAssistMovement.h"
#include "PointMovement.h"
#include "ConfusedMovement.h"
#include "../../model/gameobject/Npc.h"
#include "../../model/gameobject/Player.h"

namespace gideon { namespace zoneserver {

MovementManager::MovementManager(go::Creature& owner) :
    owner_(owner)
{
    // FYI: initialize()에 위임한다
    //reset();
}


MovementManager::~MovementManager()
{
}


void MovementManager::initialize()
{
    reset();
}


void MovementManager::finalize()
{
    reset();
}


void MovementManager::setIdleMovement()
{
    auto& npc = static_cast<go::Npc&>(owner_);
    setMovement(std::make_unique<IdleMovement<go::Npc>>(npc));
}


void MovementManager::setWanderMovement()
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<WanderMovement<go::Npc>>(npc));
    }
    else {
        assert(false);
    }
}


void MovementManager::setPathMovement()
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<PathMovement<go::Npc>>(npc));
    }
    else {
        assert(false);
    }
}


void MovementManager::setMarchMovement()
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<MarchMovement<go::Npc>>(npc));
    }
    else {
        assert(false);
    }
}


void MovementManager::setReturnMovement()
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<ReturnMovement<go::Npc>>(npc));
    }
    else {
        assert(false);
    }
}


void MovementManager::setChaseMovement()
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<ChaseMovement<go::Npc>>(npc));
    }
    else {
        assert(false);
    }
}


void MovementManager::setTimedFleeingMovement(go::Entity* fright, GameTime duration)
{
    if (duration == 0) {
        duration = fleeDuration_;
    }

    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<TimedFleeingMovement<go::Npc>>(npc, fright, duration));
    }
    else {
        assert(false);
    }

    fleeDuration_ = 0;
}


void MovementManager::setSeekAssistMovement(go::Entity& assist)
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<SeekAssistMovement<go::Npc>>(npc, assist));
    }
    else {
        assert(false);
    }
}


void MovementManager::setPointMovement(const Position& destination)
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<PointMovement<go::Npc>>(npc, destination));
    }
    else {
        assert(false);
    }
}


void MovementManager::setConfusedMovement(GameTime duration)
{
    if (owner_.isNpcOrMonster()) {
        auto& npc = static_cast<go::Npc&>(owner_);
        setMovement(std::make_unique<ConfusedMovement<go::Npc>>(npc, duration),
            mvsControlled);
    }
    else if (owner_.isPlayer()) {
        auto& player = static_cast<go::Player&>(owner_);
        setMovement(std::make_unique<ConfusedMovement<go::Player>>(player, duration),
            mvsActive);
    }
    else {
        assert(false);
    }
}


void MovementManager::unsetConfusedMovement()
{
    if (owner_.isNpcOrMonster()) {
        setMovement(std::unique_ptr<Movement>(), mvsControlled);
    }
    else if (owner_.isPlayer()) {
        setIdleMovement();
    }
    else {
        assert(false);
    }
}


//void MovementManager::setKnockbackMovement(const Position& destination)
//{
//    if (owner_.isNpcOrMonster()) {
//        auto& npc = static_cast<go::Npc&>(owner_);
//        setMovement(std::make_unique<PointMovement<go::Npc>>(npc, destination)),
//            mvsControlled);
//    }
//    else {
//        assert(false);
//    }
//}


void MovementManager::updateMovement(GameTime diff)
{
    std::lock_guard<LockType> lock(lock_);

    assert(isValid(currentMovementSlot_));

    Movement* currentMovement = getCurrentMovement();
    if (! currentMovement) {
        return;
    }

    const bool result = currentMovement->update(diff);
    if (result) {
        const MovementSlot prevMovementSlot = currentMovementSlot_;
        setMovement_i(nullptr, prevMovementSlot);
        if (prevMovementSlot == mvsActive) {
            isCompleted_ = true;
        }
        currentMovementSlot_ = mvsActive;
    }
}


bool MovementManager::isCompleted() const
{
    std::lock_guard<LockType> lock(lock_);

    if (! getCurrentMovement()) {
        return true;
    }

    return isCompleted_;
}


void MovementManager::reset()
{
    std::lock_guard<LockType> lock(lock_);

    isCompleted_ = false;
    fleeDuration_ = 0;
    currentMovementSlot_ = mvsActive;

    for (auto& movement : movements_) {
        movement.reset();
    }
}


void MovementManager::setMovement(std::unique_ptr<Movement> movement, MovementSlot movementSlot)
{
    std::lock_guard<LockType> lock(lock_);

    setMovement_i(std::move(movement), movementSlot);
}


void MovementManager::setMovement_i(std::unique_ptr<Movement> movement, MovementSlot movementSlot)
{
    assert(isValid(movementSlot));

    Movement* prevMovement = movements_[movementSlot].get();
    if (prevMovement != nullptr) {
        prevMovement->finalize();
        boost::checked_delete(prevMovement);
        movements_[movementSlot].reset();
    }

    if (movement.get() != nullptr) {
        movement->initialize();
        movements_[movementSlot] = std::move(movement);
        Movement* controlledMovement = movements_[mvsControlled].get();
        if (controlledMovement != nullptr) {
            currentMovementSlot_ = mvsControlled;
        }
        else {
            currentMovementSlot_ = movementSlot;
        }
    }
    else {
        if (movementSlot == mvsControlled) {
            currentMovementSlot_ = mvsActive;
        }
    }

    isCompleted_ = false;
}

}} // namespace gideon { namespace zoneserver {
