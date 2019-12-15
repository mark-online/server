#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/GuildCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/GuildRpc.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerGuildController
 * 길드 담당
 */
class ZoneServer_Export PlayerGuildController : public Controller,
    public GuildCallback,
    public rpc::GuildRpc,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerGuildController);
public:
    PlayerGuildController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);

private:
    // = GuildCallback overriding
    virtual void create(const BaseGuildInfo& guildInfo);
    virtual void join(const BaseGuildInfo& guildInfo);
    virtual void leave();

    virtual void created(ObjectId playerId, const BaseGuildInfo& guildInfo);
    virtual void joined(ObjectId playerId, const BaseGuildInfo& guildInfo);
    virtual void left(ObjectId playerId);
    virtual void addGuildExp(GuildExp exp);
    virtual void addCheatGuildExp(GuildExp exp);
    virtual void addGuildGameMoney(GameMoney gameMoney);

    virtual void purchaseVaultResponsed(ErrorCode errorCode, const BaseVaultInfo& vaultInfo);
    virtual void activateGuildSkillResponsed(ErrorCode errorCode, SkillCode skillCode);
    virtual void deactivateGuildSkillsResponsed(ErrorCode errorCode);
    virtual void guildSkillActivated(SkillCode skillCode);
    virtual void guildSkillsDeactivated();

public:
    // = rpc::GuildRpc overriding
    OVERRIDE_SRPC_METHOD_1(purchaseVault,
        BaseVaultInfo, vaultInfo);
    OVERRIDE_SRPC_METHOD_1(activateGuildSkill,
        SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_0(deactivateGuildSkills);

    OVERRIDE_SRPC_METHOD_2(onPurchaseVault,
        ErrorCode, errorCode, BaseVaultInfo, vaultInfo);
    OVERRIDE_SRPC_METHOD_2(onActivateGuildSkill,
        ErrorCode, errorCode, SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_1(onDeactivateGuildSkills,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_2(evGuildCreated,
        ObjectId, characterId, BaseGuildInfo, guildInfo);
    OVERRIDE_SRPC_METHOD_2(evGuildJoined,
        ObjectId, characterId, BaseGuildInfo, guildInfo);
    OVERRIDE_SRPC_METHOD_1(evGuildLeft,
        ObjectId, characterId);

    OVERRIDE_SRPC_METHOD_1(evGuildSkillActivated,
        SkillCode, skillCode);
    OVERRIDE_SRPC_METHOD_0(evGuildSkillsDeactivated);
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
