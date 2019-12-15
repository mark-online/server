#pragma once

#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class GuildCallback
 */
class GuildCallback
{
public:
    virtual ~GuildCallback() {}
public:
    virtual void create(const BaseGuildInfo& guildInfo) = 0;
    virtual void join(const BaseGuildInfo& guildInfo) = 0;
    virtual void leave() = 0;
    virtual void addGuildExp(GuildExp exp) = 0;
    virtual void addCheatGuildExp(GuildExp exp) = 0;

    virtual void addGuildGameMoney(GameMoney gameMoney) = 0;

public:
    virtual void created(ObjectId playerId, const BaseGuildInfo& guildInfo) = 0;
    virtual void joined(ObjectId playerId, const BaseGuildInfo& guildInfo) = 0;
    virtual void left(ObjectId playerId) = 0;

    virtual void purchaseVaultResponsed(ErrorCode errorCode, const BaseVaultInfo& vaultInfo) = 0;
    virtual void activateGuildSkillResponsed(ErrorCode errorCode, SkillCode skillCode) = 0;
    virtual void deactivateGuildSkillsResponsed(ErrorCode errorCode) = 0;
    virtual void guildSkillActivated(SkillCode skillCode) = 0;
    virtual void guildSkillsDeactivated() = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
