#include "ZoneServerPCH.h"
#include "Guild.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/ability/Moneyable.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/GuildCallback.h"
#include "../../world/World.h"
#include <gideon/cs/datatable/GuildLevelTable.h>

namespace gideon { namespace zoneserver {

Guild::Guild(const GuildInfo& guildInfo) :
    guildInfo_(guildInfo),
    canAddiableGuildExp_(true)
{
    for (const GuildMemberInfo& guildMemberInfo : guildInfo.guildMemberInfos_) {
        addMember(guildMemberInfo);
    }
    for (const GuildRelationshipInfo& relation : guildInfo.relytionships_) {
        addRelationship(relation);
    }
    guildLevel_ = GUILD_LEVEL_TABLE->getGuildLevel(guildInfo.levelInfo_.guildExp_);
    guildSkillManager_.initialize(guildInfo.guildSkillCodes_);
}


Guild::~Guild()
{
}


void Guild::activateSkill(ErrorCode errorCode, ObjectId playerId, SkillCode skillCode)
{
    OnlineMap onlineMap;
    if (isSucceeded(errorCode)) {
        std::lock_guard<LockType> lock(lock_);

        guildSkillManager_.removeSkill(getSkillTableType(skillCode), getSkillIndex(skillCode));
        guildSkillManager_.updateSkill(skillCode);

        onlineMap = onlineMap_;        
    }
    
    for (OnlineMap::value_type& value : onlineMap) {
        go::Entity* member = value.second;
        if (member) {
            if (member->getObjectId() == playerId) {
                member->getController().queryGuildCallback()->activateGuildSkillResponsed(errorCode, skillCode);
            }
            if (isSucceeded(errorCode)) {
                member->getController().queryGuildCallback()->guildSkillActivated(skillCode);
            }
        }
    }    
}


void Guild::deactivateSkills(ErrorCode errorCode, ObjectId playerId)
{
    OnlineMap onlineMap;
    if (isSucceeded(errorCode)) {
        std::lock_guard<LockType> lock(lock_);

        guildSkillManager_.finalize();

        onlineMap = onlineMap_;        
    }

    for (OnlineMap::value_type& value : onlineMap) {
        go::Entity* member = value.second;
        if (member) {
            if (member->getObjectId() == playerId) {
                member->getController().queryGuildCallback()->deactivateGuildSkillsResponsed(errorCode);
            }
            if (isSucceeded(errorCode)) {
                member->getController().queryGuildCallback()->guildSkillsDeactivated();
            }
        }
    }    
}


void Guild::addMember(const GuildMemberInfo& guildMemberInfo)
{
    std::lock_guard<LockType> lock(lock_);

    guildMemberInfoMap_.insert(GuildMemberInfoMap::value_type(
        guildMemberInfo.playerId_, guildMemberInfo));
}


void Guild::removeMember(ObjectId characterId)
{
    std::lock_guard<LockType> lock(lock_);

    guildMemberInfoMap_.erase(characterId);
}


void Guild::online(go::Entity& player)
{
    std::lock_guard<LockType> lock(lock_);

    onlineMap_.emplace(player.getObjectId(), &player);
}


void Guild::offline(ObjectId playerId)
{
    std::lock_guard<LockType> lock(lock_);

    onlineMap_.erase(playerId);
}


void Guild::changeGuildMemberPosition(ObjectId playerId, GuildMemberPosition position)
{
    std::lock_guard<LockType> lock(lock_);

    GuildMemberInfoMap::iterator pos = guildMemberInfoMap_.find(playerId);
    if (pos != guildMemberInfoMap_.end()) {
        GuildMemberInfo& info = (*pos).second;
        info.position_ = position;
    }
}


void Guild::addRelationship(const GuildRelationshipInfo& info)
{
    std::lock_guard<LockType> lock(lock_);

    guildRelationshipInfoMap_.emplace(info.targetGuildId_, info);
}


void Guild::removeRelationship(GuildId guildId)
{
    std::lock_guard<LockType> lock(lock_);

    guildRelationshipInfoMap_.erase(guildId);
}


const GuildRelationshipInfo* Guild::getRelaytionship(GuildId guildId) const
{
    std::lock_guard<LockType> lock(lock_);

    GuildRelationshipInfoMap::const_iterator pos = guildRelationshipInfoMap_.find(guildId);
    if (pos != guildRelationshipInfoMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


GuildName Guild::getGuildName() const
{
    std::lock_guard<LockType> lock(lock_);

    return guildInfo_.guildName_;
}


SkillCodes Guild::getGuildSkillCodes() const
{
    std::lock_guard<LockType> lock(lock_);

    return guildSkillManager_.getGuildSkills();
}

}} // namespace gideon { namespace zoneserver {
