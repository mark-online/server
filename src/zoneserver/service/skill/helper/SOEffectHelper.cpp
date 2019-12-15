#include "ZoneServerPCH.h"
#include "SOEffectHelper.h"
#include "../../../model/gameobject/Entity.h"
#include "../../../model/gameobject/Npc.h"
#include "../../../model/gameobject/Anchor.h"
#include "../../../model/gameobject/ability/StaticObjectOwerable.h"
#include "../../../model/gameobject/ability/Partyable.h"
#include "../../../model/gameobject/ability/Guildable.h"
#include "../../../world/World.h"
#include "../../party/PartyService.h"
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/datatable/SkillEffectTable.h>

namespace gideon { namespace zoneserver {

SoEffectHelper::SoEffectHelper(go::Entity& caster) :
    EffectHelper(caster)
{

}


SoEffectHelper::~SoEffectHelper()
{

}


void SoEffectHelper::applyTargetEffect_i(SkillEffectResult& skillEffectResult,
    SkillType /*skillType*/, const EffectValues& effectValues, go::Entity& /*target*/, 
    const datatable::SkillEffectTemplate& effectTemplate, EffectDefenceType /*defenceType*/)
{    
    for (int i = 0; i < eanCount; ++i) {
        const datatable::EffectInfo* effectInfo = nullptr;
        if (skillEffectResult.isCasterEffect_) {
            effectInfo = &effectTemplate.casterEffects_[i];
        } 
        else {
            effectInfo = &effectTemplate.casteeEffects_[i];            
        }

        if (effectInfo && ! effectInfo->isValid()) {
            continue;
        }

        skillEffectResult.effects_[i].value_ = effectValues[i];
    }    
}


bool SoEffectHelper::isAvailableTarget(go::Entity& target, AvailableTarget availableTarget) const
{
    if (target.isNpcOrMonster()) {
        if (static_cast<go::Npc&>(target).isStaticNpc()) {
            return false;
        }
    }

    if (availableTarget == atAll) {
        return true;
    }

    if (availableTarget == atSelf) {
        if (target.isPlayer()) {            
            const AnchorOwnerInfo& ownerInfo =getCasterAs<go::Anchor>().getOwnerInfo();
            if (ownerInfo.ownerId_ == target.getObjectId()) {
                return true;
            }        
        }
    }
    else if (availableTarget == atParty) {
        return isMyPartyMember(target);
    }
    else if (availableTarget == atGuild) {
        return isMyGuildMember(target);
    }
    else if (availableTarget == atGuildOrParty) {
        return isMyPartyMember(target) || isMyGuildMember(target);
    }
    else if (availableTarget == atMoster) {
        if (! target.isMonster()) {
            return false;
        }
    }

    return false;
}


bool SoEffectHelper::isExceptTarget(go::Entity& target, ExceptTarget exceptTarget) const
{
    if (etNoneTaget ==  exceptTarget) {
        return false;   
    }

    if (etSelf == exceptTarget) {
        return getCaster().isSame(target);
    }

    if (exceptTarget == etParty) {
        return isMyPartyMember(target);
    }
    else if (exceptTarget == etGuild) {
        return isMyGuildMember(target);
    }
    else if (exceptTarget == etPartyOrGuild) {
        return isMyPartyMember(target) || isMyGuildMember(target);
    }
    else if (etNpcOrMonster == exceptTarget) {
        return target.isNpcOrMonster();
    }
    else if (etNpcOrMosterOrSelf == exceptTarget) {
        if (getCaster().isSame(target) || target.isMonster()) {
            return true;
        }
    }

    return false;
}


bool SoEffectHelper::isMyPartyMember(go::Entity& target) const
{
    if (! target.isPlayer()) {
        return false;
    }

    PartyPtr casterParty;
    PartyPtr targetParty;

    if (getCaster().isAnchor()) {
        const AnchorOwnerInfo& ownerInfo =getCasterAs<go::Anchor>().getOwnerInfo();
        go::Entity* owner = WORLD->getPlayer(ownerInfo.ownerId_);
        if (! owner) {
            return false;
        }
        const go::Partyable* ownerPartiable = owner->queryPartyable();
        if (! ownerPartiable) {
            return false;
        }
        casterParty = ownerPartiable->getParty();
    }

    const go::Partyable* targetPartiable = target.queryPartyable();
    if (! targetPartiable) {
        return false;
    }
    targetParty = targetPartiable->getParty();
    
    if (casterParty.get() == nullptr || targetParty.get() == nullptr) {
        return false;
    }
    return casterParty.get() == targetParty.get();
}


bool SoEffectHelper::isMyGuildMember(go::Entity& target) const
{
    if (! target.isPlayer()) {
        return false;
    }

    GuildId casterGuildId = invalidGuildId;
    GuildId targetGuildId = invalidGuildId;
    const AnchorOwnerInfo& ownerInfo =getCasterAs<go::Anchor>().getOwnerInfo();
    go::Entity* owner = WORLD->getPlayer(ownerInfo.ownerId_);
    if (! owner) {
        return false;
    }
    const go::Guildable* ownerGuildable = owner->queryGuildable();
    if (! ownerGuildable) {
        return false;
    }
    casterGuildId = ownerGuildable->getGuildId();   
    targetGuildId = target.queryGuildable()->getGuildId();
    
    if (! isValidGuildId(casterGuildId) || ! isValidGuildId(targetGuildId)) {
        return false;
    }

    return casterGuildId == targetGuildId;
}

}} // namespace gideon { namespace zoneserver {

