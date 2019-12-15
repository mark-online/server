#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/server/data/ServerGuildInfo.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go


/***
 * @class Guild
 ***/
class ZoneServer_Export Guild : public sne::core::ThreadSafeMemoryPoolMixin<Guild>
{
    typedef std::mutex LockType;
    typedef sne::core::Map<ObjectId, go::Entity*> OnlineMap;
    typedef sne::core::Map<ObjectId, GuildMemberInfo> GuildMemberInfoMap;

public:
    Guild(const GuildInfo& guildInfo);
    ~Guild();
    
    void activateSkill(ErrorCode errorCode, ObjectId playerId, SkillCode skillCode);
    void deactivateSkills(ErrorCode errorCode, ObjectId playerId);

    void setGuildLevel(GuildLevel guildLevel) {
        guildLevel_ = guildLevel;
    }
    void setAddibleDayExpState(bool canAddiableGuildExp) {
        canAddiableGuildExp_ = canAddiableGuildExp;
    }

    void addMember(const GuildMemberInfo& guildMemberInfo);
    void removeMember(ObjectId characterId);
    void online(go::Entity& player);
    void offline(ObjectId playerId);
	void changeGuildMemberPosition(ObjectId playerId, GuildMemberPosition position);
    void addRelationship(const GuildRelationshipInfo& info);
    void removeRelationship(GuildId guildId);

    const GuildRelationshipInfo* getRelaytionship(GuildId guildId) const;
	bool shouldDestory() const {
		return guildMemberInfoMap_.empty();
	}

    bool canSaveExp() const {
        return ! isMaxGuildLevel(guildLevel_) && canAddiableGuildExp_;
    }

    SkillCodes getGuildSkillCodes() const;

public:
    GuildName getGuildName() const;

    GuildMarkCode getGuildMarkCode() const {
        return guildInfo_.guildMarkCode_;
    }

    const BaseGuildInfo getBaseGuildInfo() const {
        return guildInfo_;
    }
    
private:
    mutable LockType lock_;

    BaseGuildInfo guildInfo_;
    OnlineMap onlineMap_;
    GuildMemberInfoMap guildMemberInfoMap_;
    GuildRelationshipInfoMap guildRelationshipInfoMap_;
    GuildSkillManager guildSkillManager_;
    GuildLevel guildLevel_;
    bool canAddiableGuildExp_;
};

typedef std::shared_ptr<Guild> GuildPtr;

}} // namespace gideon { namespace zoneserver {
