#pragma once

#include "../SkillController.h"
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class NpcSkillController
 * 스킬 담당
 */
class NpcSkillController : public SkillController
{
public:
    NpcSkillController(go::Entity* owner) :
        SkillController(owner) {}

private:
};


}}} // namespace gideon { namespace zoneserver { namespace gc {