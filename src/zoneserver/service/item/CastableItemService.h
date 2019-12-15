#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Code.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {

class CastableItem;


/**
 * @class SkillService
 *
 * 스킬 서비스
 * TODO: Skill caching (Skill에서 ThreadSafeMemoryPoolMixin 삭제)
 */
class ZoneServer_Export CastableItemService : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(CastableItemService);

public:
    CastableItemService();
    //~SkillService();

    /// 서비스를 초기화한다.
    bool initialize();

public:
    std::unique_ptr<CastableItem> createCastableItemFor(go::Entity& caster,
        ObjectId itemId, DataCode itemCode);

};

}} // namespace gideon { namespace zoneserver {

#define CASTABLE_ITEM_SERVICE gideon::zoneserver::CastableItemService::instance()
