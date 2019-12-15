#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <sne/core/utility/Singleton.h>
#include <boost/noncopyable.hpp>
#include <atomic>

namespace gideon { namespace zoneserver {

/**
 * @class ItemIdGenerator
 *
 */
class ZoneServer_Export ItemIdGenerator : public boost::noncopyable
{
    SNE_DECLARE_SINGLETON(ItemIdGenerator);
public:
    ItemIdGenerator(ObjectId objectId) :
        itemIdGenerator_(objectId) {}

    ObjectId generateItemId() {
        return ++itemIdGenerator_;
    }

private:
    std::atomic<ObjectId> itemIdGenerator_;
};

}} // namespace gideon { namespace zoneserver {


#define ITEM_ID_GENERATOR gideon::zoneserver::ItemIdGenerator::instance()
