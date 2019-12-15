#pragma once

#include <gideon/cs/shared/data/SkillInfo.h>

namespace gideon {

class GuildSkillManager : public CharacterSkillManager
{
public:
    SkillCodes getGuildSkills() const {
        SkillCodes skillCodes;
        for (const auto& value : getSkillIndexMap()) {
            skillCodes.push_back(value.second);
        }
        return skillCodes;
    }    
};
    
} // namespace gideon {
