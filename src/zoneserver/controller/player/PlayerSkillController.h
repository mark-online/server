#pragma once

#include "../SkillController.h"
#include "../callback/SkillCallback.h"
#include "../callback/PassiveSkillCallback.h"
#include <gideon/cs/shared/rpc/player/SkillRpc.h>
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
class ZoneServer_Export PlayerSkillController : public SkillController,
    public rpc::SkillRpc,
    public SkillCallback,
    public PassiveSkillCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerSkillController);
public:
    PlayerSkillController(go::Entity* owner) :
        SkillController(owner) {}

    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

public:
    // = SkillCallback overriding
    virtual void activateConcentrationCancelled(const GameObjectInfo& sourceInfo,
        SkillCode skillCode);
	virtual void activateConcentrationCompleted(const GameObjectInfo& sourceInfo,
		SkillCode skillCode);
    virtual void skillLearned(SkillCode oldSkillCode, SkillCode newSkillCode);
    virtual void skillRemoved(SkillCode skillCode);
    virtual void skillAllRemoved();

public:
    // = PassiveSkillCallback overriding
    virtual void activatePassive(SkillCode skillCode);
    virtual void passiveActivated(const GameObjectInfo& creatureInfo, SkillCode skillCode);

public:
    // = rpc::SkillRpc overriding
    OVERRIDE_SRPC_METHOD_1(learnSkills,
        SkillCodes, skillCode);
    OVERRIDE_SRPC_METHOD_0(resetLearnedSkills);

    OVERRIDE_SRPC_METHOD_2(onLearnSkills,
        ErrorCode, errorCode, SkillPoint, currentSkillPoint);
    OVERRIDE_SRPC_METHOD_3(onResetLearnedSkills,
        ErrorCode, errorCode, GameMoney, currentGameMoney, SkillPoint, currentSkillPoint);

   
    OVERRIDE_SRPC_METHOD_2(evConcentrationSkillCancelled,
        GameObjectInfo, entityInfo, SkillCode, skillCode);
	OVERRIDE_SRPC_METHOD_2(evConcentrationSkillCompleted,
		GameObjectInfo, entityInfo, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_2(evSkillLearned,
        SkillCode, removeSkillCode, SkillCode, addSkillCode);
    OVERRIDE_SRPC_METHOD_2(evPassiveSkillActivated,
        GameObjectInfo, entityInfo, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_0(evAllSkillRemoved);
   
private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {