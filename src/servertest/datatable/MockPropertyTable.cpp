#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockPropertyTable.h>

namespace gideon { namespace servertest {

const std::wstring* MockPropertyTable::getProperty(const std::wstring& key) const
{
    const PropertyMap::const_iterator pos = propertyMap_.find(key);
    if (pos != propertyMap_.end()) {
        return &((*pos).second);
    }
    return nullptr;
}


void MockPropertyTable::fillProperties()
{
    propertyMap_.insert(
        PropertyMap::value_type(L"cp_recovery_interval", L"2000"));
    propertyMap_.insert(
        PropertyMap::value_type(L"cp_recovery_value", L"7"));
    propertyMap_.insert(
        PropertyMap::value_type(L"player_min_revive_time", L"30000"));	
    propertyMap_.insert(
        PropertyMap::value_type(L"gravestone_created_despawn_time", L"60000"));
    propertyMap_.insert(
        PropertyMap::value_type(L"gravestone_despawn_time", L"500000"));
    propertyMap_.insert(
        PropertyMap::value_type(L"gravestone_min_life_point", L"2000"));	
    propertyMap_.insert(
        PropertyMap::value_type(L"gravestone_max_life_point", L"5000"));
    propertyMap_.insert(
        PropertyMap::value_type(L"trade_max_accept_distance", L"5"));
    propertyMap_.insert(
        PropertyMap::value_type(L"harvest_distance", L"5"));
    propertyMap_.insert(
        PropertyMap::value_type(L"treasure_distance", L"5"));
    propertyMap_.insert(
        PropertyMap::value_type(L"quest_distance", L"5"));
    propertyMap_.insert(
        PropertyMap::value_type(L"player_forward_speed", L"1.0"));
}

}} // namespace gideon { namespace servertest {
