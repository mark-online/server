#include "ZoneServerPCH.h"
#include "PlayerGuildController.h"
#include "../EntityController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Guildable.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "../../service/guild/GuildService.h"
#include "../../service/guild/Guild.h"
#include "../../ZoneService.h"
#include <sne/core/memory/MemoryPoolMixin.h>
#include <sne/sgp/session/extension/RpcingExtension.h>

namespace gideon { namespace zoneserver { namespace gc {

namespace {


/**
 * @class GuildCreateEvent
 */
class GuildCreateEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<GuildCreateEvent>
{
public:
    GuildCreateEvent(ObjectId playerId, const BaseGuildInfo& guildInfo) :
        playerId_(playerId),
        guildInfo_(guildInfo) {}

private:
    virtual void call(go::Entity& entity) {
        GuildCallback* guildCallback = entity.getController().queryGuildCallback();
        if (guildCallback != nullptr) {
            guildCallback->created(playerId_, guildInfo_);
        }
    }

private:
	const ObjectId playerId_;
    const BaseGuildInfo guildInfo_;
};


/**
 * @class GuildJoinEvent
 */
class GuildJoinEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<GuildJoinEvent>
{
public:
    GuildJoinEvent(ObjectId playerId, const BaseGuildInfo& guildInfo) :
        playerId_(playerId),
        guildInfo_(guildInfo) {}

private:
    virtual void call(go::Entity& entity) {
        GuildCallback* guildCallback = entity.getController().queryGuildCallback();
        if (guildCallback != nullptr) {
            guildCallback->joined(playerId_, guildInfo_);
        }
    }

private:
	const ObjectId playerId_;
    const BaseGuildInfo guildInfo_;
};


/**
 * @class GuildLeaveEvent
 */
class GuildLeaveEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<GuildLeaveEvent>
{
public:
    GuildLeaveEvent(ObjectId playerId) :
        playerId_(playerId) {}

private:
    virtual void call(go::Entity& entity) {
        GuildCallback* guildCallback = entity.getController().queryGuildCallback();
        if (guildCallback != nullptr) {
            guildCallback->left(playerId_);
        }
    }

private:
	const ObjectId playerId_;
};

} // namespace {

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerGuildController);

PlayerGuildController::PlayerGuildController(go::Entity* owner) :
    Controller(owner)
{
}


void PlayerGuildController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerGuildController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}

// = GuildCallback overriding

void PlayerGuildController::create(const BaseGuildInfo& guildInfo)
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<GuildCreateEvent>(owner.getObjectId(), guildInfo);
    owner.queryKnowable()->broadcast(event, true);
}


void PlayerGuildController::join(const BaseGuildInfo& guildInfo)
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<GuildJoinEvent>(owner.getObjectId(), guildInfo);
    owner.queryKnowable()->broadcast(event, true);
}


void PlayerGuildController::leave()
{
    go::Entity& owner = getOwner();
    auto event = std::make_shared<GuildLeaveEvent>(owner.getObjectId());
    owner.queryKnowable()->broadcast(event, true);
}


void PlayerGuildController::addGuildExp(GuildExp exp)
{
    ZONE_SERVICE->getCommunityServerProxy().z2m_addGuildExp(getOwner().getAccountId(),
        getOwner().getObjectId(), exp);
}


void PlayerGuildController::addCheatGuildExp(GuildExp exp)
{
    ZONE_SERVICE->getCommunityServerProxy().z2m_addCheatGuildExp(getOwner().getAccountId(),
        getOwner().getObjectId(), exp);
}


void PlayerGuildController::addGuildGameMoney(GameMoney gameMoney)
{
    ZONE_SERVICE->getCommunityServerProxy().z2m_addGuildGameMoney(getOwner().getAccountId(),
        getOwner().getObjectId(), gameMoney);

}


void PlayerGuildController::created(ObjectId playerId, const BaseGuildInfo& guildInfo)
{
    evGuildCreated(playerId, guildInfo);
}


void PlayerGuildController::joined(ObjectId playerId, const BaseGuildInfo& guildInfo)
{
    evGuildJoined(playerId, guildInfo);
}


void PlayerGuildController::left(ObjectId playerId)
{
    evGuildLeft(playerId);
}


void PlayerGuildController::purchaseVaultResponsed(ErrorCode errorCode, const BaseVaultInfo& vaultInfo)
{
    onPurchaseVault(errorCode, vaultInfo);
}


void PlayerGuildController::activateGuildSkillResponsed(ErrorCode errorCode, SkillCode skillCode)
{
    onActivateGuildSkill(errorCode, skillCode);
}


void PlayerGuildController::deactivateGuildSkillsResponsed(ErrorCode errorCode)
{
    onDeactivateGuildSkills(errorCode);
}


void PlayerGuildController::guildSkillActivated(SkillCode skillCode)
{
    getOwner().queryGuildable()->activateSkill(skillCode);
    evGuildSkillActivated(skillCode);
}


void PlayerGuildController::guildSkillsDeactivated()
{
    getOwner().queryGuildable()->deactivateSkills();
    evGuildSkillsDeactivated();
}


// = rpc::GuildRpc overriding

RECEIVE_SRPC_METHOD_1(PlayerGuildController, purchaseVault,
    BaseVaultInfo, vaultInfo)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(getOwner().queryGuildable()->getGuildId());
    if (! guild.get()) {
        return;
    }

    if (vaultInfo.name_.empty()) {
        onPurchaseVault(ecGuild_VaultNameIsEmpty, vaultInfo);
        return ;
    }

    ZONE_SERVICE->getCommunityServerProxy().z2m_purchaseVault(getOwner().getAccountId(),
        vaultInfo, getOwner().getObjectId());
}


RECEIVE_SRPC_METHOD_1(PlayerGuildController, activateGuildSkill,
    SkillCode, skillCode)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(getOwner().queryGuildable()->getGuildId());
    if (! guild.get()) {
        return;
    }

    ZONE_SERVICE->getCommunityServerProxy().z2m_activateGuildSkill(getOwner().getAccountId(),
        getOwner().getObjectId(), skillCode);
}


RECEIVE_SRPC_METHOD_0(PlayerGuildController, deactivateGuildSkills)
{
    GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(getOwner().queryGuildable()->getGuildId());
    if (! guild.get()) {
        return;
    }

    ZONE_SERVICE->getCommunityServerProxy().z2m_deactivateGuildSkills(getOwner().getAccountId(),
        getOwner().getObjectId());
}


FORWARD_SRPC_METHOD_2(PlayerGuildController, onPurchaseVault,
    ErrorCode, errorCode, BaseVaultInfo, vaultInfo);


FORWARD_SRPC_METHOD_2(PlayerGuildController, onActivateGuildSkill,
    ErrorCode, errorCode, SkillCode, skillCode);


FORWARD_SRPC_METHOD_1(PlayerGuildController, onDeactivateGuildSkills,
    ErrorCode, errorCode);


FORWARD_SRPC_METHOD_2(PlayerGuildController, evGuildCreated,
    ObjectId, characterId, BaseGuildInfo, guildInfo);


FORWARD_SRPC_METHOD_2(PlayerGuildController, evGuildJoined,
    ObjectId, characterId, BaseGuildInfo, guildInfo);


FORWARD_SRPC_METHOD_1(PlayerGuildController, evGuildLeft,
    ObjectId, characterId);


FORWARD_SRPC_METHOD_1(PlayerGuildController, evGuildSkillActivated,
    SkillCode, skillCode);


FORWARD_SRPC_METHOD_0(PlayerGuildController, evGuildSkillsDeactivated);

}}} // namespace gideon { namespace zoneserver { namespace gc {
