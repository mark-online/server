#pragma once

#include <gideon/server/data/ServerGuildInfo.h>


namespace gideon { namespace zoneserver { namespace go {

class Player;

/**
 * @class GuildSkillRepository
 * guild skill 관리자
 */
class GuildSkillRepository
{
    typedef sne::core::HashMap<EffectScriptType, int32_t> EffectMap;
public:
    GuildSkillRepository(Player& owner);

    bool initialize(const SkillCodes& skillCodes);
    void clear();

    void activateSkill(SkillCode skillCode);
    void deactivateSkills();

    int32_t getEffectValue(EffectScriptType type) const;

private:
    Player& owner_;
    EffectMap effectMap_;
    GuildSkillManager guildSkillManager_;
};


}}} // namespace gideon { namespace zoneserver { namespace go {