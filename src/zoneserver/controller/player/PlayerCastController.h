#pragma once

#include "../SkillController.h"
#include "../callback/CastCallback.h"
#include <gideon/cs/shared/data/CraftInfo.h>
#include <gideon/cs/shared/rpc/player/CastingRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
    class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerSkillController
 * 스킬 담당
 */
class ZoneServer_Export PlayerCastController : public Controller,
    public rpc::CastRpc,
    public CastCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerCastController);
public:
    PlayerCastController(go::Entity* owner) :
        Controller(owner) {}

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

    virtual void initialize() {}

private:
    virtual void casted(const StartCastResultInfo& startInfo);
    virtual void canceled(const CancelCastResultInfo& cancelInfo);
    virtual void failed(const FailCompletedCastResultInfo& failInfo);
    virtual void completed(const CompleteCastResultInfo& completeInfo);

public:
    // = CastRpc overriding
    OVERRIDE_SRPC_METHOD_1(startCasting,
        StartCastInfo, startInfo);
    OVERRIDE_SRPC_METHOD_0(cancelCasting);

    OVERRIDE_SRPC_METHOD_2(onStartCasting,
        ErrorCode, errorCode, StartCastInfo, startInfo);

    OVERRIDE_SRPC_METHOD_1(evCasted,
        StartCastResultInfo, startInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCanceled,
        CancelCastResultInfo, cancelInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCompleted,
        CompleteCastResultInfo, completeInfo);
    OVERRIDE_SRPC_METHOD_1(evCastCompleteFailed,
        FailCompletedCastResultInfo, failInfo);

private:
	ErrorCode castSkillTo(SkillCode skillCode, const GameObjectInfo& targetInfo);
	ErrorCode castSkillAt(SkillCode skillCode, const Position& position);
	ErrorCode castItemTo(ObjectId itemId, const GameObjectInfo& targetInfo);
	ErrorCode castItemAt(ObjectId itemId, const Position& position);

    ErrorCode openTreasure(const GameObjectInfo& treasueInfo);
    ErrorCode startHarvest(const GameObjectInfo& harvestInfo);
	ErrorCode startActiveDevice(const GameObjectInfo& deviceInfo);

	ErrorCode startVehicle();
	ErrorCode startGlider();

	ErrorCode startBindRecall(ObjectId linkId);

	ErrorCode startLootingItem(const GameObjectInfo& graveStoneInfo, LootInvenId itemId);

    ErrorCode startLootingBuildingItem(const GameObjectInfo& buildingInfo, ObjectId itemId, InvenType invenType);

    ErrorCode startNpcCraft(const GameObjectInfo& npcInfo, DataCode recipeCode);
    ErrorCode startPlayerCraft(const GameObjectInfo& npcInfo, ObjectId recipeId);
    ErrorCode startReprocess(const GameObjectInfo& npcInfo, ObjectId itemId);

    ErrorCode startEquipUpgrade(const GameObjectInfo& npcInfo, ObjectId equipItemId);
    ErrorCode startEquipEnchant(const GameObjectInfo& npcInfo, ObjectId equipItemId, ObjectId gemItemId);

    ErrorCode startUseFunctionItem(ObjectId itemId);

    ErrorCode checkCanCraftable(GameTime& completeMicSec, const GameObjectInfo& npcInfo, DataCode recipeCode, bool isPlayerCraft);
    ErrorCode checkCanUpgrade(const GameObjectInfo& npcInfo, CraftType craftType);

private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {