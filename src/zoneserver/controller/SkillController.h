#pragma once

#include "../zoneserver_export.h"
#include "Controller.h"
#include "callback/EffectCallback.h"
#include "callback/CreatureEffectCallback.h"
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon {
struct Position;
} // namespace gideon 


namespace gideon { namespace zoneserver {
class SkillCasterState;
class CreatureState;
}} // namespace gideon { namespace zoneserver {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class SkillController
 * 스킬 담당
 */
class ZoneServer_Export SkillController : public Controller
{
public:
    SkillController(go::Entity* owner);
    virtual ~SkillController() {}

private:
    SkillCasterState& skillCasterState_;
    CreatureState& creatureState_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
