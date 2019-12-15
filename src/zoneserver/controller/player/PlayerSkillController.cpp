#include "ZoneServerPCH.h"
#include "PlayerSkillController.h"
#include "../PlayerController.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/GraveStone.h"
#include "../../model/gameobject/ability/SkillLearnable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../service/item/GraveStoneService.h"
#include "../../world/WorldMap.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/server/utility/Profiler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

namespace {
		

/**
 * @class ActivatePassiveEvent
 */
class ActivatePassiveEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<ActivatePassiveEvent>
{
public:
    ActivatePassiveEvent(const GameObjectInfo& info, SkillCode skillCode) :
		info_(info),
        skillCode_(skillCode) {}

private:
    virtual void call(go::Entity& entity) {
		PassiveSkillCallback* callback = 
            entity.getController().queryPassiveSkillCallback();
		if (callback != nullptr) {
			callback->passiveActivated(info_, skillCode_);
		}
    }

private:
	const GameObjectInfo info_;
    const SkillCode skillCode_;
};

} // namespace {

// = PlayerSkillController
IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerSkillController);

void PlayerSkillController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerSkillController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerSkillController::activateConcentrationCancelled(const GameObjectInfo& sourceInfo,
    SkillCode skillCode)
{
    evConcentrationSkillCancelled(sourceInfo, skillCode);
}


void PlayerSkillController::activateConcentrationCompleted(const GameObjectInfo& sourceInfo,
	SkillCode skillCode)
{
	evConcentrationSkillCompleted(sourceInfo, skillCode);
}


void PlayerSkillController::skillLearned(SkillCode oldSkillCode, SkillCode newSkillCode)
{
    go::Entity& owner = getOwner();
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncLearnSkill(owner.getAccountId(), owner.getObjectId(), oldSkillCode, newSkillCode);
    }

    evSkillLearned(oldSkillCode, newSkillCode);
}


void PlayerSkillController::skillRemoved(SkillCode skillCode)
{
    go::Entity& owner = getOwner();
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveSkill(owner.getAccountId(), owner.getObjectId(), skillCode);
    }
}

void PlayerSkillController::skillAllRemoved()
{
    go::Entity& owner = getOwner();
    {
        DatabaseGuard db(SNE_DATABASE_MANAGER);
        db->asyncRemoveAllSkill(owner.getAccountId(), owner.getObjectId());
    }
    evAllSkillRemoved();
}


void PlayerSkillController::activatePassive(SkillCode skillCode)
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<ActivatePassiveEvent>(owner.getGameObjectInfo(), skillCode);
    owner.queryKnowable()->broadcast(event);
}


void PlayerSkillController::passiveActivated(const GameObjectInfo& creatureInfo, SkillCode skillCode)
{
    evPassiveSkillActivated(creatureInfo, skillCode);
}

// = rpc::SkillRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerSkillController, learnSkills,
    SkillCodes, skillCodes)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = owner.querySkillLearnable()->learnSkills(skillCodes);
    onLearnSkills(errorCode, owner.querySkillLearnable()->getSkillPoint());
}


RECEIVE_SRPC_METHOD_0(PlayerSkillController, resetLearnedSkills)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }
   
    const ErrorCode errorCode = owner.querySkillLearnable()->resetLearnedSkills();
    onResetLearnedSkills(errorCode, owner.queryMoneyable()->getGameMoney(),
        owner.querySkillLearnable()->getSkillPoint());
}


FORWARD_SRPC_METHOD_2(PlayerSkillController, onLearnSkills,
    ErrorCode, errorCode, SkillPoint, currentSkillPoint);


FORWARD_SRPC_METHOD_3(PlayerSkillController, onResetLearnedSkills,
    ErrorCode, errorCode, GameMoney, currentGameMoney, SkillPoint, currentSkillPoint);


FORWARD_SRPC_METHOD_2(PlayerSkillController, evConcentrationSkillCancelled,
    GameObjectInfo, entityInfo, SkillCode, skillCode);


FORWARD_SRPC_METHOD_2(PlayerSkillController, evConcentrationSkillCompleted,
	GameObjectInfo, entityInfo, SkillCode, skillCode);


FORWARD_SRPC_METHOD_2(PlayerSkillController, evSkillLearned,
    SkillCode, removeSkillCode, SkillCode, addSkillCode);


FORWARD_SRPC_METHOD_2(PlayerSkillController, evPassiveSkillActivated,
     GameObjectInfo, entityInfo, SkillCode, skillCode);


FORWARD_SRPC_METHOD_0(PlayerSkillController, evAllSkillRemoved)
// = sne::srpc::RpcForwarder overriding

void PlayerSkillController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerSkillController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerSkillController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerSkillController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {
