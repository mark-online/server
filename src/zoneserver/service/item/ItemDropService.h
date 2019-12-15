#pragma once

#include "../../zoneserver_export.h"
#include <gideon/cs/shared/data/LootInvenItemInfo.h>
#include <gideon/cs/shared/data/ItemDropInfo.h>


namespace gideon { namespace zoneserver {

/**
 * @class ItemDropService
 *
 * 스킬 서비스
 */
class ZoneServer_Export ItemDropService : public boost::noncopyable
{
public:
    static void fillItemDropInfo(LootInvenItemInfoMap& graveStoneItemMap, ItemDropId itemDropId);
    static void fillWorldDropInfo(LootInvenItemInfoMap& lootInvenItemInfoMap, WorldDropSuffixId suffixId);
};

}} // namespace gideon { namespace zoneserver {
