#include "ZoneServerPCH.h"
#include "ItemOptionService.h"
#include <gideon/cs/datatable/ItemOptionTable.h>
#include <gideon/cs/datatable/ItemSuffixTable.h>

namespace gideon { namespace zoneserver {


ItemScriptInfos ItemOptionService::getItemOptions(ItemSuffixId id)
{
    ItemScriptInfos infos;
    const datatable::ItemSuffixInfo* suffixInfo = ITEM_SUFFIX_TABLE->getItemSuffixInfo(id);
    if (! suffixInfo) {
        return infos;
    }

    const ItemOptionId optionId = suffixInfo->getRandomItemOptionId();
    if (! isValidItemOptionId(optionId)) {
        return infos;
    }

    datatable::ItemOptionInfoList optionList = 
        ITEM_OPTION_TABLE->getItemOptionInfoList(optionId, suffixInfo->getOptionCount());
    for (datatable::ItemOptionInfo& info : optionList) {
        infos.push_back(info.getRandomItemScriptInfo());
    }

    return infos;
}

}} // namespace gideon { namespace zoneserver {
