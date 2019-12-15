#pragma once

#include "../../zoneserver_export.h"
#include "Skill.h"
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver {

class Skill;


/**
 * @class SkillService
 *
 * 스킬 서비스
 * TODO: Skill caching (Skill에서 ThreadSafeMemoryPoolMixin 삭제)
 */
class ZoneServer_Export SkillService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(SkillService);

public:
    SkillService();
    //~SkillService();

    /// 서비스를 초기화한다.
    bool initialize();

public:
    std::unique_ptr<Skill> createSkillFor(go::Entity& caster, SkillCode skillCode);

private:
    bool shouldCheckSafeRegion_;
};

}} // namespace gideon { namespace zoneserver {

#define SKILL_SERVICE gideon::zoneserver::SkillService::instance()
