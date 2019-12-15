#include "ZoneServerPCH.h"
#include "PlayerHarvestController.h"
#include "../../model/item/Inventory.h"
#include "../../model/item/QuestInventory.h"
#include "../../model/gameobject/Harvest.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/ability/Inventoryable.h"
#include "../../model/state/HarvestState.h"
#include "../../model/state/VehicleState.h"
#include "../../controller/EntityController.h"
#include "../../service/spawn/SpawnService.h"
#include <sne/server/utility/Profiler.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace gc {


IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerHarvestController);

PlayerHarvestController::PlayerHarvestController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerHarvestController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerHarvestController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerHarvestController::rewarded(ObjectId harvestId,
    const BaseItemInfos& baseItems, const QuestItemInfos& questItems)
{
	go::Inventoryable* inventoryable = getOwner().queryInventoryable();
	if (! inventoryable) {
		return;
	}
    if (! baseItems.empty()) {		
		inventoryable->getInventory().setTempHarvestItems(baseItems);		
    }
    if (! questItems.empty()) {
        inventoryable->getQuestInventory().setTempHarvestItems(questItems);
    }	

    evHarvestRewarded(harvestId, baseItems, questItems);
}


RECEIVE_SRPC_METHOD_1(PlayerHarvestController, acquireHarvest,
    DataCode, itemCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

	go::Inventoryable* inventoryable = getOwner().queryInventoryable();
	if (! inventoryable) {
		return;
	}

    CodeType codeType = getCodeType(itemCode);
    ErrorCode errorCode = ecItemNotExist;
    
    if (ctElement == codeType) {
        errorCode = inventoryable->getInventory().acquireHarvest(itemCode);        
    }
    else if (ctQuestItem == codeType) {
        errorCode = inventoryable->getQuestInventory().acquireHarvest(itemCode);        
    }
    onAcquireHarvest(errorCode, itemCode);    
}


FORWARD_SRPC_METHOD_2(PlayerHarvestController, onAcquireHarvest,
    ErrorCode, errorCode, DataCode, itemCode);


FORWARD_SRPC_METHOD_3(PlayerHarvestController, evHarvestRewarded,
    ObjectId, haravestId, BaseItemInfos, itemInfos,
    QuestItemInfos, questItemInfos);


// = sne::srpc::RpcForwarder overriding

void PlayerHarvestController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerHarvestController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerHarvestController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerHarvestController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

}}} // namespace gideon { namespace zoneserver { namespace gc {