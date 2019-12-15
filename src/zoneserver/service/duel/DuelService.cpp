#include "ZoneServerPCH.h"
#include "DuelService.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/FieldDuelable.h"
#include "../../model/gameobject/ability/Chaoable.h"
#include "../../model/state/FieldDuelState.h"
#include "../../model/state/ChaoState.h"

namespace gideon { namespace zoneserver {


SNE_DEFINE_SINGLETON(DuelService);

DuelService::DuelService() :
    duelId_(invalidObjectId)
{
}


DuelService::~DuelService()
{
}


bool DuelService::initialize()
{
    DuelService::instance(this);
    return true;
}


ErrorCode DuelService::canFieldDueling(go::Entity& src, go::Entity& desc) const
{    
    FieldDuelState* srcState = src.queryFieldDuelState();
    if (! srcState) {
        return ecServerInternalError;
    }

    if (srcState->isFieldDueling()) {
        return ecDuelAlready;
    }

    if (src.queryChaoable()->isChao()) {
        return ecDuelChaoStateNotPlay;
    }

    FieldDuelState* descState = desc.queryFieldDuelState();
    if (! descState) {
        return ecDuelTargetInvalid;
    }

    if (descState->isFieldDueling()) {
        return ecDuelTargetAlready;
    }

    if (desc.queryChaoable()->isChao()) {
        return ecDuelTargetChaoStateNotPlay;
    }

    const float32_t maxDistanceSq = 10.0f * 10.0f;
    float32_t  distanceToTargetSq = src.getSquaredLength(desc.getPosition());
    if (maxDistanceSq > 0) {
        if (distanceToTargetSq > maxDistanceSq) {
            return ecDuelTargetFarDistance;
        }
    }

    return ecOk;
}


void DuelService::startFieldDueling(go::Entity& duealer1, go::Entity& duealer2)
{
    {
        std::lock_guard<LockType> lock(lock_);

        ++duelId_;

        auto fieldDual = std::make_shared<FieldDual>(this, duelId_, duealer1, duealer2);
        ptrMap_.emplace(duelId_, fieldDual);

        setFieldDuelInfo(fieldDual, duealer1);
        setFieldDuelInfo(fieldDual, duealer2);
    }
}


void DuelService::setFieldDuelInfo(FieldDualPtr ptr, go::Entity& duealer)
{
    FieldDuelState* state = duealer.queryFieldDuelState();
    if (state) {
        state->startFieldDuel(duelId_);
    }
    
    go::FieldDuelable* able = duealer.queryFieldDuelable();
    if (able) {
        able->setFieldDuel(ptr);
    }
}


void DuelService::destory(ObjectId objectId)
{
    std::lock_guard<LockType> lock(lock_);

    ptrMap_.erase(objectId);
}

}} // namespace gideon { namespace zoneserver {
