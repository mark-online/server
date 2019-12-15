#include "ZoneServerPCH.h"
#include "BuildingStateBrain.h"
#include "state/building/ActiveBuildingBrainState.h"
#include "state/BrainStateMachine.h"
#include "aggro/AggroList.h"
#include "evt/EventTrigger.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/ability/Buildable.h"
#include "../model/gameobject/ability/Guildable.h"
#include "../service/guild/GuildService.h"

namespace gideon { namespace zoneserver { namespace ai {

BuildingStateBrain::BuildingStateBrain(go::Entity& owner) :
    StateBrain(owner, nullptr),
    aggroList_(std::make_unique<AggroList>(owner))
{
}


BuildingStateBrain::~BuildingStateBrain()
{
}


bool BuildingStateBrain::shouldStrikeBack(const go::Entity& attacker) const
{
    go::Entity& owner = const_cast<go::Entity&>(getOwner());
    go::Buildable* buildable = owner.queryBuildable(); assert(buildable != nullptr);

    if (attacker.getObjectId() == buildable->getOwnerId()) {
        return false;
    }

    go::Guildable* attackerGuilable = const_cast<go::Entity&>(attacker).queryGuildable();
    if (! attackerGuilable) {
        return true;
    }

    const GuildId ownerGuildId = buildable->getOwnerGuildId();
    if (! isValidGuildId(ownerGuildId)) {
        return true;
    }

    const GuildId attackerGuildId = attackerGuilable->getGuildId();
    if (! isValidGuildId(attackerGuildId)) {
        return true;
    }

    if (ownerGuildId == attackerGuildId) {
        return false;
    }

    // FYI: 동맹 길드가 공격해도 적대로 판정
    //GuildPtr ownerGuild = GUILD_SERVICE->getGuildByGuildId(ownerGuildId);
    //if (! ownerGuild.get()) {
    //    return true;
    //}
    //const GuildRelationshipInfo* relationshipInfo = ownerGuild->getRelaytionship(attackerGuildId);
    //if (! relationshipInfo) {
    //    return true;
    //}
    //return relationshipInfo->relationship_ != grtFriendly;
    return true;
}

// = Brain overriding

void BuildingStateBrain::initialize()
{
    globalState_ = std::make_unique<ActiveBuildingBrainState>();
    getStateMachine().setGlobalState(*globalState_);
}


void BuildingStateBrain::finalize()
{
}


void BuildingStateBrain::attacked(const go::Entity& attacker)
{
    if (shouldStrikeBack(attacker)) {
        notifyToSummons(attacker);
    }

    aggroList_->clear();
}


void BuildingStateBrain::died()
{
    aggroList_->clear();

    StateBrain::died();
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
