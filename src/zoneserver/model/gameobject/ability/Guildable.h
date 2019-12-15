#pragma once

#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/EffectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {
class PlayerGuildController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Guildable
 * 길드를 사용할수 있다
 */
class Guildable
{
public:
    virtual ~Guildable() {}
	
public:
	virtual GuildId getGuildId() const = 0;    
	
	virtual void setGuildInfo(const BaseGuildInfo& guildInfo) = 0;

    virtual int32_t getGuildEffectValue(EffectScriptType type) const = 0;

    virtual void activateSkill(SkillCode skillCode) = 0;

    virtual void deactivateSkills() = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
