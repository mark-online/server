#include "ServerBasePCH.h"
#include <gideon/serverbase/datatable/DataTableLoader.h>
#include <gideon/cs/datatable/DataTableFactory.h>
#include <gideon/cs/datatable/EquipTable.h>
#include <gideon/cs/datatable/ReprocessTable.h>
#include <gideon/cs/datatable/PlayerActiveSkillTable.h>
#include <gideon/cs/datatable/SOActiveSkillTable.h>
#include <gideon/cs/datatable/ItemActiveSkillTable.h>
#include <gideon/cs/datatable/PlayerPassiveSkillTable.h>
#include <gideon/cs/datatable/NpcActiveSkillTable.h>
#include <gideon/cs/datatable/SkillEffectTable.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/RecipeTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/FragmentTable.h>
#include <gideon/cs/datatable/WorldMapTable.h>
#include <gideon/cs/datatable/RegionTable.h>
#include <gideon/cs/datatable/RegionCoordinates.h>
#include <gideon/cs/datatable/RegionSpawnTable.h>
#include <gideon/cs/datatable/PositionSpawnTable.h>
#include <gideon/cs/datatable/ExpTable.h>
#include <gideon/cs/datatable/SelectRecipeProductionTable.h>
#include <gideon/cs/datatable/TreasureTable.h>
#include <gideon/cs/datatable/NpcSellTable.h>
#include <gideon/cs/datatable/NpcBuyTable.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/EntityPathTable.h>
#include <gideon/cs/datatable/ResourcesProductionTable.h>
#include <gideon/cs/datatable/ArenaTable.h>
#include <gideon/cs/datatable/AccessoryTable.h>
//#include <gideon/cs/datatable/StaticObjectSkillTable.h>
#include <gideon/cs/datatable/ActionTable.h>
#include <gideon/cs/datatable/FunctionTable.h>
#include <gideon/cs/datatable/ItemDropTable.h>
#include <gideon/cs/datatable/WorldDropTable.h>
#include <gideon/cs/datatable/WorldDropSuffixTable.h>
#include <gideon/cs/datatable/PropertyTable.h>
#include <gideon/cs/datatable/GemTable.h>
#include <gideon/cs/datatable/QuestTable.h>
#include <gideon/cs/datatable/QuestItemTable.h>
#include <gideon/cs/datatable/QuestKillMissionTable.h>
#include <gideon/cs/datatable/QuestActivationMissionTable.h>
#include <gideon/cs/datatable/QuestProbeMissionTable.h>
#include <gideon/cs/datatable/QuestTransportMissionTable.h>
#include <gideon/cs/datatable/QuestObtainMissionTable.h>
#include <gideon/cs/datatable/QuestContentsMissionTable.h>
#include <gideon/cs/datatable/RandomDungeonTable.h>
#include <gideon/cs/datatable/HarvestTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/FactionTable.h>
#include <gideon/cs/datatable/EventTriggerTable.h>
#include <gideon/cs/datatable/DeviceTable.h>
#include <gideon/cs/datatable/GliderTable.h>
#include <gideon/cs/datatable/VehicleTable.h>
#include <gideon/cs/datatable/HarnessTable.h>
#include <gideon/cs/datatable/NpcFormationTable.h>
#include <gideon/cs/datatable/NpcTalkingTable.h>
#include <gideon/cs/datatable/BuildingGuardTable.h>
#include <gideon/cs/datatable/BuildingGuardSellTable.h>
#include <gideon/cs/datatable/WorldEventTable.h>
#include <gideon/cs/datatable/WorldEventMissionTable.h>
#include <gideon/cs/datatable/WorldEventInvaderSpawnTable.h>
#include <gideon/cs/datatable/WorldEventMissionSpawnTable.h>
#include <gideon/cs/datatable/ItemOptionTable.h>
#include <gideon/cs/datatable/ItemSuffixTable.h>
#include <gideon/cs/datatable/CharacterStatusTable.h>
#include <gideon/cs/datatable/CharacterDefaultItemTable.h>
#include <gideon/cs/datatable/CharacterDefaultSkillTable.h>
#include <gideon/cs/datatable/AchievementTable.h>
#include <gideon/cs/datatable/GuildLevelTable.h>
#include <gideon/cs/datatable/GuildSkillTable.h>
#include <sne/server/common/Property.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Logger.h>
#include <fstream>

namespace gideon { namespace serverbase {

namespace {

template <typename Table>
inline std::unique_ptr<Table> loadTable(const std::string& tableUrl,
    std::unique_ptr<Table> (*createTable)(std::istream& is))
{
    std::unique_ptr<Table> table;

    std::string why;
    std::ifstream ifs(tableUrl.c_str());
    if ((! tableUrl.empty()) && ifs.is_open()) {
        table = (*createTable)(ifs);
        if (table->isLoaded()) {
            return table;
        }
        else {
            why = table->getLastError();
        }
    }
    else {
        why = "file not found";
    }

    table.reset();
    SNE_LOG_ERROR("Can't load data-table(%s) - %s",
        tableUrl.c_str(), why.c_str());
    return table;
}

} // namespace


bool DataTableLoader::loadPropertyTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadPropertyTable()");

    static std::unique_ptr<datatable::PropertyTable> s_propertyTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.property_table_url");

    s_propertyTable = loadTable<datatable::PropertyTable>(tableUrl,
        datatable::DataTableFactory::createPropertyTable);
    if (! s_propertyTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadPropertyTable() FAILED!");
        return false;
    }

    assert(GIDEON_PROPERTY_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadCharacterStatusTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadCharacterStatusTable()");

    static std::unique_ptr<datatable::CharacterStatusTable> s_characterStatusTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.character_status_table_url");

    s_characterStatusTable = loadTable<datatable::CharacterStatusTable>(tableUrl,
        datatable::DataTableFactory::createCharacterStatusTable);
    if (! s_characterStatusTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadCharacterStatusTable() FAILED!");
        return false;
    }

    assert(CHARACTER_STATUS_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadWorldMapTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadWorldMapTable()");

    static std::unique_ptr<datatable::WorldMapTable> s_worldMapTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.world_map_table_url");

    s_worldMapTable = loadTable<datatable::WorldMapTable>(tableUrl,
        datatable::DataTableFactory::createWorldMapTable);
    if (! s_worldMapTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadWorldMapTable() FAILED!");
        return false;
    }

    assert(WORLDMAP_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadEquipTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadEquipTable()");

    static std::unique_ptr<datatable::EquipTable> s_equipTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.equip_table_url");

    s_equipTable = loadTable<datatable::EquipTable>(tableUrl,
        datatable::DataTableFactory::createEquipTable);
    if (! s_equipTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadEquipTable() FAILED!");
        return false;
    }

    assert(EQUIP_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadReprocessTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadReprocessTable()");

    static std::unique_ptr<datatable::ReprocessTable> s_reprocessTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.reprocess_table_url");

    s_reprocessTable = loadTable<datatable::ReprocessTable>(tableUrl,
        datatable::DataTableFactory::createReprocessTable);
    if (! s_reprocessTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadReprocessTable() FAILED!");
        return false;
    }

    assert(REPROCESS_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadPlayerActiveSkillTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadPlayerActiveSkillTable()");

    static std::unique_ptr<datatable::PlayerActiveSkillTable> s_playerActiveSkillTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.player_active_skill_table_url");

    s_playerActiveSkillTable = loadTable<datatable::PlayerActiveSkillTable>(tableUrl,
        datatable::DataTableFactory::createPlayerActiveSkillTableForServer);
    if (! s_playerActiveSkillTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadPlayerActiveSkillTable() FAILED!");
        return false;
    }

    assert(PLAYER_ACTIVE_SKILL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadItemActiveSkillTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadItemActiveSkillTable()");

    static std::unique_ptr<datatable::ItemActiveSkillTable> s_itemActiveSkillTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.item_active_skill_table_url");

    s_itemActiveSkillTable = loadTable<datatable::ItemActiveSkillTable>(tableUrl,
        datatable::DataTableFactory::createItemActiveSkillTableForServer);
    if (! s_itemActiveSkillTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadItemActiveSkillTable() FAILED!");
        return false;
    }

    assert(ITEM_ACTIVE_SKILL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadSOActiveSkillTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadSOActiveSkillTable()");

    static std::unique_ptr<datatable::SOActiveSkillTable> s_soSkillTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.so_active_skill_table_url");

    s_soSkillTable = loadTable<datatable::SOActiveSkillTable>(tableUrl,
        datatable::DataTableFactory::createSOActiveSkillTableForServer);
    if (! s_soSkillTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadSOActiveSkillTable() FAILED!");
        return false;
    }

    assert(SO_ACTIVE_SKILL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadPlayerPassiveSkillTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadPlayerPassiveSkillTable()");

    static std::unique_ptr<datatable::PlayerPassiveSkillTable> s_playerPassiveSkillTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.player_passive_skill_table_url");

    s_playerPassiveSkillTable = loadTable<datatable::PlayerPassiveSkillTable>(tableUrl,
        datatable::DataTableFactory::createPlayerPassiveSkillTableForServer);
    if (! s_playerPassiveSkillTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadPlayerPassiveSkillTable() FAILED!");
        return false;
    }

    assert(PLAYER_PASSIVE_SKILL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadNpcActiveSkillTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadNpcActiveSkillTable()");

    static std::unique_ptr<datatable::NpcActiveSkillTable> s_npcSkillTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.npc_active_skill_table_url");

    s_npcSkillTable = loadTable<datatable::NpcActiveSkillTable>(tableUrl,
        datatable::DataTableFactory::createNpcActiveSkillTableForServer);
    if (! s_npcSkillTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadNpcActiveSkillTable() FAILED!");
        return false;
    }

    assert(NPC_ACTIVE_SKILL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadSkillEffectTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadSkillEffectTable()");

    static std::unique_ptr<datatable::SkillEffectTable> s_skillEffectTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.skill_effect_table_url");

    s_skillEffectTable = loadTable<datatable::SkillEffectTable>(tableUrl,
        datatable::DataTableFactory::createSkillEffectTable);
    if (! s_skillEffectTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadSkillEffectTable() FAILED!");
        return false;
    }

    assert(SKILL_EFFECT_TABLE != nullptr);
    return true;
}

bool DataTableLoader::loadNpcTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadNpcTable()");

    static std::unique_ptr<datatable::NpcTable> s_npcTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.npc_table_url");

    s_npcTable = loadTable<datatable::NpcTable>(tableUrl,
        datatable::DataTableFactory::createNpcTable);
    if (! s_npcTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadNpcTable() FAILED!");
        return false;
    }

    assert(NPC_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadElementTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadElementTable()");

    static std::unique_ptr<datatable::ElementTable> s_elementTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.element_table_url");

    s_elementTable = loadTable<datatable::ElementTable>(tableUrl,
        datatable::DataTableFactory::createElementTable);
    if (! s_elementTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadElementTable() FAILED!");
        return false;
    }

    assert(ELEMENT_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadFragmentTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadFragmentTable()");

    static std::unique_ptr<datatable::FragmentTable> s_fragmentTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.fragment_table_url");
    s_fragmentTable = loadTable<datatable::FragmentTable>(tableUrl,
        datatable::DataTableFactory::createFragmentTable);
    if (! s_fragmentTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadFragmentTable() FAILED!");
        return false;
    }

    assert(FRAGMENT_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadItemDropTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadItemDropTable()");

    static std::unique_ptr<datatable::ItemDropTable> s_itemDropTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.item_drop_table_url");

    s_itemDropTable = loadTable<datatable::ItemDropTable>(tableUrl,
        datatable::DataTableFactory::createItemDropTable);
    if (! s_itemDropTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadItemDropTable() FAILED!");
        return false;
    }

    assert(ITEM_DROP_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadWorldDropTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadWorldDropTable()");

    static std::unique_ptr<datatable::WorldDropTable> s_worldDropTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.world_drop_table_url");

    s_worldDropTable = loadTable<datatable::WorldDropTable>(tableUrl,
        datatable::DataTableFactory::createWorldDropTable);
    if (! s_worldDropTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadWorldDropTable() FAILED!");
        return false;
    }

    assert(WORLD_DROP_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadWorldDropSuffixTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadWorldDropSuffixTable()");

    static std::unique_ptr<datatable::WorldDropSuffixTable> s_worldDropSuffixTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.world_drop_suffix_table_url");

    s_worldDropSuffixTable = loadTable<datatable::WorldDropSuffixTable>(tableUrl,
        datatable::DataTableFactory::createWorldDropSuffixTable);
    if (! s_worldDropSuffixTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadWorldDropSuffixTable() FAILED!");
        return false;
    }

    assert(WORLD_DROP_SUFFIX_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadGemTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadGemTable()");

    static std::unique_ptr<datatable::GemTable> s_gemTable;
    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.gem_table_url");
    s_gemTable = loadTable<datatable::GemTable>(tableUrl,
        datatable::DataTableFactory::createGemTable);
    if (! s_gemTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadGemTable() FAILED!");
        return false;
    }

    assert(GEM_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadRecipeTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadRecipeTable()");

    static std::unique_ptr<datatable::RecipeTable> s_recipeTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.recipe_table_url");
    s_recipeTable = loadTable<datatable::RecipeTable>(tableUrl,
        datatable::DataTableFactory::createRecipeTable);
    if (! s_recipeTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadRecipeTable() FAILED!");
        return false;
    }

    assert(RECIPE_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadExpTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadExpTable()");

    static std::unique_ptr<datatable::ExpTable> s_expTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.exp_table_url");
    s_expTable = loadTable<datatable::ExpTable>(tableUrl,
        datatable::DataTableFactory::createExpTable);
    if (! s_expTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadExpTable() FAILED!");
        return false;
    }

    assert(EXP_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadSelectProductionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadSelectProductionTable()");

    static std::unique_ptr<datatable::SelectRecipeProductionTable> s_selectProductionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.select_recipe_production_table_url");
    s_selectProductionTable = loadTable<datatable::SelectRecipeProductionTable>(tableUrl,
        datatable::DataTableFactory::createSelectRecipeProductionTable);
    if (! s_selectProductionTable.get()) {
         SNE_LOG_ERROR("DataTableLoader::loadSelectProductionTable() FAILED!");
        return false;
    }

    assert(SELECT_RECIPE_PRODUCTION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadRandomDungeonTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadRandomDungeonTable()");

    static std::unique_ptr<datatable::RandomDungeonTable> s_randomDungeonTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.random_dungeon_table_url");
    s_randomDungeonTable = loadTable<datatable::RandomDungeonTable>(tableUrl,
        datatable::DataTableFactory::createRandomDungeonTable);
    if (! s_randomDungeonTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadRandomDungeonTable() FAILED!");
        return false;
    }

    assert(RANDOM_DUNGEON_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestTable()");

    static std::unique_ptr<datatable::QuestTable> s_questTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_table_url");
    s_questTable = loadTable<datatable::QuestTable>(tableUrl,
        datatable::DataTableFactory::createQuestTable);
    if (! s_questTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestTable() FAILED!");
        return false;
    }

    assert(QUEST_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestItemTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestItemTable()");

    static std::unique_ptr<datatable::QuestItemTable> s_questItemTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_item_table_url");
    s_questItemTable = loadTable<datatable::QuestItemTable>(tableUrl,
        datatable::DataTableFactory::createQuestItemTable);
    if (! s_questItemTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestItemTable() FAILED!");
        return false;
    }

    assert(QUEST_ITEM_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestKillMissionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestKillMissionTable()");

    static std::unique_ptr<datatable::QuestKillMissionTable> s_questKillMissionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_kill_mission_table_url");
    s_questKillMissionTable = loadTable<datatable::QuestKillMissionTable>(tableUrl,
        datatable::DataTableFactory::createQuestKillMissionTable);
    if (! s_questKillMissionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestKillMissionTable() FAILED!");
        return false;
    }

    assert(QUEST_KILL_MISSION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestActivationMissionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestActivationMissionTable()");

    static std::unique_ptr<datatable::QuestActivationMissionTable> s_questActivationMissionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_activation_mission_table_url");
    s_questActivationMissionTable = loadTable<datatable::QuestActivationMissionTable>(tableUrl,
        datatable::DataTableFactory::createQuestActivationMissionTable);
    if (! s_questActivationMissionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestActivationMissionTable() FAILED!");
        return false;
    }

    assert(QUEST_ACTIVATION_MISSION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestObtainMissionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestObtainMissionTable()");

    static std::unique_ptr<datatable::QuestObtainMissionTable> s_questObtainMissionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_obtain_mission_table_url");
    s_questObtainMissionTable = loadTable<datatable::QuestObtainMissionTable>(tableUrl,
        datatable::DataTableFactory::createQuestObtainMissionTable);
    if (! s_questObtainMissionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestObtainMissionTable() FAILED!");
        return false;
    }

    assert(QUEST_OBTAIN_MISSION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestProbeMissionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestProbeMissionTable()");

    static std::unique_ptr<datatable::QuestProbeMissionTable> s_questProbeMissionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_probe_mission_table_url");
    s_questProbeMissionTable = loadTable<datatable::QuestProbeMissionTable>(tableUrl,
        datatable::DataTableFactory::createQuestProbeMissionTable);
    if (! s_questProbeMissionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestProbeMissionTable() FAILED!");
        return false;
    }

    assert(QUEST_PROBE_MISSION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestTransportMissionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestTransportMissionTable()");

    static std::unique_ptr<datatable::QuestTransportMissionTable> s_questTransportMissionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_transport_mission_table_url");
    s_questTransportMissionTable = loadTable<datatable::QuestTransportMissionTable>(tableUrl,
        datatable::DataTableFactory::createQuestTransportMissionTable);
    if (! s_questTransportMissionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestTransportMissionTable() FAILED!");
        return false;
    }

    assert(QUEST_TRANSPORT_MISSION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadQuestContentsMissionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadQuestContentsMissionTable()");

    static std::unique_ptr<datatable::QuestContentsMissionTable> s_questContentstMissionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.quest_contents_mission_table_url");
    s_questContentstMissionTable = loadTable<datatable::QuestContentsMissionTable>(tableUrl,
        datatable::DataTableFactory::createQuestContentsMissionTable);
    if (! s_questContentstMissionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadQuestContentsMissionTable() FAILED!");
        return false;
    }

    assert(QUEST_CONTENTS_MISSION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadHarvestTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadHarvestTable()");

    static std::unique_ptr<datatable::HarvestTable> s_harvestTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.harvest_table_url");
    s_harvestTable = loadTable<datatable::HarvestTable>(tableUrl,
        datatable::DataTableFactory::createHarvestTable);
    if (! s_harvestTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadHarvestTable() FAILED!");
        return false;
    }

    assert(HARVEST_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadTreasureTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadTreasureTable()");

    static std::unique_ptr<datatable::TreasureTable> s_treasureTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.treasure_table_url");
    s_treasureTable = loadTable<datatable::TreasureTable>(tableUrl,
        datatable::DataTableFactory::createTreasureTable);
    if (! s_treasureTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadTreasureTable() FAILED!");
        return false;
    }

    assert(TREASURE_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadNpcSellTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadNpcSellTable()");

    static std::unique_ptr<datatable::NpcSellTable> s_npcSellTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.npc_sell_table_url");
    s_npcSellTable = loadTable<datatable::NpcSellTable>(tableUrl,
        datatable::DataTableFactory::createNpcSellTable);
    if (! s_npcSellTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadNpcSellTable() FAILED!");
        return false;
    }

    assert(NPC_SELL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadNpcBuyTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadNpcBuyTable()");

    static std::unique_ptr<datatable::NpcBuyTable> s_npcBuyTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.npc_buy_table_url");
    s_npcBuyTable = loadTable<datatable::NpcBuyTable>(tableUrl,
        datatable::DataTableFactory::createNpcBuyTable);
    if (! s_npcBuyTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadNpcBuyTable() FAILED!");
        return false;
    }

    assert(NPC_BUY_TABLE != nullptr);
    return true;
}



bool DataTableLoader::loadAnchorTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadAnchorTable()");

    static std::unique_ptr<datatable::AnchorTable> s_anchorTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.anchor_table_url");
    s_anchorTable = loadTable<datatable::AnchorTable>(tableUrl,
        datatable::DataTableFactory::createAnchorTable);
    if (! s_anchorTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadAnchorTable() FAILED!");
        return false;
    }

    assert(ANCHOR_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadArenaTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadArenaTable()");

    static std::unique_ptr<datatable::ArenaTable> s_arenaTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.arena_table_url");
    s_arenaTable = loadTable<datatable::ArenaTable>(tableUrl,
        datatable::DataTableFactory::createArenaTable);
    if (! s_arenaTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadArenaTable() FAILED!");
        return false;
    }

    assert(ARENA_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadAccessoryTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadAccessoryTable()");

    static std::unique_ptr<datatable::AccessoryTable> s_accessoryTable;
    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.accessory_table_url");
    s_accessoryTable = loadTable<datatable::AccessoryTable>(tableUrl,
        datatable::DataTableFactory::createAccessoryTable);
    if (! s_accessoryTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadAccessoryTable() FAILED!");
        SNE_LOG_ERROR("DataTableLoader::loadAccessoryTable() FAILED!");
        return false;
    }

    assert(ACCESSORY_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadBuildingTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadBuildingTable()");

    static std::unique_ptr<datatable::BuildingTable> s_buildingTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.building_table_url");
    s_buildingTable = loadTable<datatable::BuildingTable>(tableUrl,
        datatable::DataTableFactory::createBuildingTable);
    if (! s_buildingTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadBuildingTable() FAILED!");
        return false;
    }

    assert(BUILDING_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadResourcesProductionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadResourcesProductionTable()");

    static std::unique_ptr<datatable::ResourcesProductionTable> s_resourcesProductionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.resources_production_table_url");
    s_resourcesProductionTable = loadTable<datatable::ResourcesProductionTable>(tableUrl,
        datatable::DataTableFactory::createResourcesProductionTable);
    if (! s_resourcesProductionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadResourcesProductionTable() FAILED!");
        return false;
    }

    assert(RESOURCES_PRODUCTION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadFactionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadFactionTable()");

    static std::unique_ptr<datatable::FactionTable> s_factionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.faction_table_url");
    s_factionTable = loadTable<datatable::FactionTable>(tableUrl,
        datatable::DataTableFactory::createFactionTable);
    if (! s_factionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadFactionTable() FAILED!");
        return false;
    }

    assert(FACTION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadEventTriggerTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadEventTriggerTable()");

    static std::unique_ptr<datatable::EventTriggerTable> s_eventAiTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.event_trigger_table_url");
    s_eventAiTable = loadTable<datatable::EventTriggerTable>(tableUrl,
        datatable::DataTableFactory::createEventTriggerTable);
    if (! s_eventAiTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadEventTriggerTable() FAILED!");
        return false;
    }

    assert(EVT_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadDeviceTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadDeviceTable()");

    static std::unique_ptr<datatable::DeviceTable> s_deviceTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.device_table_url");
    s_deviceTable = loadTable<datatable::DeviceTable>(tableUrl,
    datatable::DataTableFactory::createDeviceTable);
    if (! s_deviceTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadDeviceTable() FAILED!");
        return false;
    }

    assert(DEVICE_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadGliderTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadGliderTable()");

    static std::unique_ptr<datatable::GliderTable> s_gliderTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.glider_table_url");
    s_gliderTable = loadTable<datatable::GliderTable>(tableUrl,
        datatable::DataTableFactory::createGliderTable);
    if (! s_gliderTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadGliderTable() FAILED!");
        return false;
    }

    assert(GLIDER_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadFunctionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadFunctionTable()");

    static std::unique_ptr<datatable::FunctionTable> s_functionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.function_table_url");
    s_functionTable = loadTable<datatable::FunctionTable>(tableUrl,
        datatable::DataTableFactory::createFunctionTable);
    if (! s_functionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadFunctionTable() FAILED!");
        return false;
    }

    assert(FUNCTION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadVehicleTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadVehicleTable()");

    static std::unique_ptr<datatable::VehicleTable> s_vehicleTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.vehicle_table_url");
    s_vehicleTable = loadTable<datatable::VehicleTable>(tableUrl,
        datatable::DataTableFactory::createVehicleTable);
    if (! s_vehicleTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadVehicleTable() FAILED!");
        return false;
    }

    assert(VEHICLE_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadHarnessTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadHarnessTable()");

    static std::unique_ptr<datatable::HarnessTable> s_harnessTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.harness_table_url");
    s_harnessTable = loadTable<datatable::HarnessTable>(tableUrl,
        datatable::DataTableFactory::createHarnessTable);
    if (! s_harnessTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadHarnessTable() FAILED!");
        return false;
    }

    assert(HARNESS_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadActionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadActionTable()");

    static std::unique_ptr<datatable::ActionTable> s_actionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.action_table_url");
    s_actionTable = loadTable<datatable::ActionTable>(tableUrl,
        datatable::DataTableFactory::createActionTable);
    if (! s_actionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadActionTable() FAILED!");
        return false;
    }

    assert(ACTION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadNpcFormationTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadNpcFormationTable()");

    static std::unique_ptr<datatable::NpcFormationTable> s_npcFormationTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.npc_formation_table_url");
    s_npcFormationTable = loadTable<datatable::NpcFormationTable>(tableUrl,
        datatable::DataTableFactory::createNpcFormationTable);
    if (! s_npcFormationTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadNpcFormationTable() FAILED!");
        return false;
    }

    assert(NPC_FORMATION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadNpcTalkingTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadNpcTalkingTable()");

    static std::unique_ptr<datatable::NpcTalkingTable> s_npcTalkingTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.npc_talking_table_url");
    s_npcTalkingTable = loadTable<datatable::NpcTalkingTable>(tableUrl,
        datatable::DataTableFactory::createNpcTalkingTable);
    if (! s_npcTalkingTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadNpcTalkingTable() FAILED!");
        return false;
    }

    assert(NPC_TALKING_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadBuildingGuardTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadBuildingGuardTable()");

    static std::unique_ptr<datatable::BuildingGuardTable> s_buildingGuard;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.building_guard_table_url");
    s_buildingGuard = loadTable<datatable::BuildingGuardTable>(tableUrl,
        datatable::DataTableFactory::createBuildingGuardTable);
    if (! s_buildingGuard.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadBuildingGuardTable() FAILED!");
        return false;
    }

    assert(BUILDING_GUARD_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadBuildingGuardSellTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadBuildingGuardSellTable()");

    static std::unique_ptr<datatable::BuildingGuardSellTable> s_buildingGuardSell;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.building_guard_sell_table_url");
    s_buildingGuardSell = loadTable<datatable::BuildingGuardSellTable>(tableUrl,
        datatable::DataTableFactory::createBuildingGuardSellTable);
    if (! s_buildingGuardSell.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadBuildingGuardSellTable() FAILED!");
        return false;
    }

    assert(BUILDING_GUARD_SELL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadWorldEventTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadWorldEventTable()");

    static std::unique_ptr<datatable::WorldEventTable> s_worldEvent;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.world_event_table_url");
    s_worldEvent = loadTable<datatable::WorldEventTable>(tableUrl,
        datatable::DataTableFactory::createWorldEventTable);
    if (! s_worldEvent.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadWorldEventTable() FAILED!");
        return false;
    }

    assert(WORLD_EVENT_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadWorldEventMissionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadWorldEventMissionTable()");

    static std::unique_ptr<datatable::WorldEventMissionTable> s_worldEventMission;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.world_event_mission_table_url");
    s_worldEventMission = loadTable<datatable::WorldEventMissionTable>(tableUrl,
        datatable::DataTableFactory::createWorldEventMissionTable);
    if (! s_worldEventMission.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadWorldEventMissionTable() FAILED!");
        return false;
    }

    assert(WORLD_EVENT_MISSION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadWorldEventInvaderSpawnTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadWorldEventInvaderSpawnTable()");

    static std::unique_ptr<datatable::WorldEventInvaderSpawnTable> s_worldEventInvaderSpawn;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.world_event_invader_spawn_table_url");
    s_worldEventInvaderSpawn = loadTable<datatable::WorldEventInvaderSpawnTable>(tableUrl,
        datatable::DataTableFactory::createWorldEventInvaderSpawnTable);
    if (! s_worldEventInvaderSpawn.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadWorldEventInvaderSpawnTable() FAILED!");
        return false;
    }

    assert(WORLD_EVENT_INVADER_SPAWN_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadWorldEventMissionSpawnTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadWorldEventMissionSpawnTable()");

    static std::unique_ptr<datatable::WorldEventMissionSpawnTable> s_worldEventMissionSpawn;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.world_event_mission_spawn_table_url");
    s_worldEventMissionSpawn = loadTable<datatable::WorldEventMissionSpawnTable>(tableUrl,
        datatable::DataTableFactory::createWorldEventMissionSpawnTable);
    if (! s_worldEventMissionSpawn.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadWorldEventMissionSpawnTable() FAILED!");
        return false;
    }

    assert(WORLD_EVENT_MISSION_SPAWN_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadItemOptionTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadItemOptionTable()");

    static std::unique_ptr<datatable::ItemOptionTable> s_itemOptionTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.item_option_table_url");
    s_itemOptionTable = loadTable<datatable::ItemOptionTable>(tableUrl,
        datatable::DataTableFactory::creatItemOptionTable);                                     
    if (! s_itemOptionTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadItemOptionTable() FAILED!");
        return false;
    }

    assert(ITEM_OPTION_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadItemSuffixTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadItemSuffixTable()");

    static std::unique_ptr<datatable::ItemSuffixTable> s_itemSuffixTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.item_suffix_table_url");
    s_itemSuffixTable = loadTable<datatable::ItemSuffixTable>(tableUrl,
        datatable::DataTableFactory::createItemSuffixTable);
    if (! s_itemSuffixTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadItemSuffixTable() FAILED!");
        return false;
    }

    assert(ITEM_SUFFIX_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadCharacterDefaultItemTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadCharacterDefaultItemTable()");

    static std::unique_ptr<datatable::CharacterDefaultItemTable> s_characterDefaultItemTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.character_default_item_table_url");
    s_characterDefaultItemTable = loadTable<datatable::CharacterDefaultItemTable>(tableUrl,
        datatable::DataTableFactory::createCharacterDefaultItemTable);
    if (! s_characterDefaultItemTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadCharacterDefaultItemTable() FAILED!");
        return false;
    }

    assert(CHARACTER_DEFAULT_ITEM_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadCharacterDefaultSkillTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadCharacterDefaultSkillTable()");

    static std::unique_ptr<datatable::CharacterDefaultSkillTable> s_characterDefaultSkillTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.character_default_skill_table_url");
    s_characterDefaultSkillTable = loadTable<datatable::CharacterDefaultSkillTable>(tableUrl,
        datatable::DataTableFactory::createCharacterDefaultSkillTable);
    if (! s_characterDefaultSkillTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadCharacterDefaultSkillTable() FAILED!");
        return false;
    }

    assert(CHARACTER_DEFAULT_SKILL_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadAchievementTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadAchievementTable()");

    static std::unique_ptr<datatable::AchievementTable> s_achievementTable;

    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.achievement_table_url");
    s_achievementTable = loadTable<datatable::AchievementTable>(tableUrl,
        datatable::DataTableFactory::createAchievementTable);
    if (! s_achievementTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadAchievementTable() FAILED!");
        return false;
    }

    assert(ACHIEVEMENT_TABLE != nullptr);
    return true;
}


bool DataTableLoader::loadGuildLevelTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadGuildLevelTable()");

    static std::unique_ptr<datatable::GuildLevelTable> s_guildLevel;
    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.guild_level_table_url");
    s_guildLevel = loadTable<datatable::GuildLevelTable>(tableUrl,
        datatable::DataTableFactory::createGuildLevelTable);
    if (! s_guildLevel.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadGuildLevelTable() FAILED!");
        return false;
    }

    assert(GUILD_LEVEL_TABLE != nullptr);
    return true;
}

bool DataTableLoader::loadGuildSkillTable()
{
    sne::server::Profiler profiler("DataTableLoader::loadGuildSkillTable()");

    static std::unique_ptr<datatable::GuildSkillTable> s_guildSkillTable;
    const std::string tableUrl =
        SNE_PROPERTIES::getProperty<std::string>("data_table.guild_skill_table_url");
    s_guildSkillTable = loadTable<datatable::GuildSkillTable>(tableUrl,
        datatable::DataTableFactory::createGuildSkillTable);
    if (! s_guildSkillTable.get()) {
        SNE_LOG_ERROR("DataTableLoader::loadGuildSkillTable() FAILED!");
        return false;
    }

    assert(GUILD_SKILL_TABLE != nullptr);
    return true;
}


std::unique_ptr<datatable::RegionTable>
    DataTableLoader::loadRegionTable(MapCode mapCode)
{
    if (! isValidMapCode(mapCode)) {
        return std::unique_ptr<datatable::RegionTable>();
    }

    std::string urlPrefix;
    if (getMapType(mapCode) == mtArena) {
        urlPrefix =
            SNE_PROPERTIES::getProperty<std::string>("data_table.arena_region_table_url_prefix");
    }
    else {
        urlPrefix =
            SNE_PROPERTIES::getProperty<std::string>("data_table.region_table_url_prefix");
    }

    const std::string regionUrl = urlPrefix +
        boost::lexical_cast<std::string>(mapCode) + ".xml";

    return loadTable<datatable::RegionTable>(regionUrl,
        datatable::DataTableFactory::createRegionTable);
}


std::unique_ptr<datatable::RegionCoordinates>
    DataTableLoader::loadRegionCoordinates(MapCode mapCode)
{
    const std::string urlPrefix =
        SNE_PROPERTIES::getProperty<std::string>("data_table.region_coordinates_url_prefix");

    const std::string coordinatesUrl = urlPrefix +
        boost::lexical_cast<std::string>(mapCode) + ".xml";

    return loadTable<datatable::RegionCoordinates>(coordinatesUrl,
        datatable::DataTableFactory::createRegionCoordinates);
}


std::unique_ptr<datatable::RegionSpawnTable>
    DataTableLoader::loadRegionSpawnTable(MapCode mapCode)
{
    const std::string urlPrefix =
        SNE_PROPERTIES::getProperty<std::string>("data_table.region_spawn_table_url_prefix");

    const std::string regionUrl = urlPrefix +
        boost::lexical_cast<std::string>(mapCode) + ".xml";

    return loadTable<datatable::RegionSpawnTable>(regionUrl,
        datatable::DataTableFactory::createRegionSpawnTable);
}


std::unique_ptr<datatable::PositionSpawnTable>
    DataTableLoader::loadPositionSpawnTable(MapCode mapCode)
{
    const std::string urlPrefix =
        SNE_PROPERTIES::getProperty<std::string>("data_table.position_spawn_table_url_prefix");

    const std::string regionUrl = urlPrefix +
        boost::lexical_cast<std::string>(mapCode) + ".xml";

    return loadTable<datatable::PositionSpawnTable>(regionUrl,
        datatable::DataTableFactory::createPositionSpawnTable);
}


std::unique_ptr<datatable::EntityPathTable>
    DataTableLoader::loadEntityPathTable(MapCode mapCode)
{
    const std::string urlPrefix =
        SNE_PROPERTIES::getProperty<std::string>("data_table.entity_path_table_url_prefix");

    const std::string regionUrl = urlPrefix +
        boost::lexical_cast<std::string>(mapCode) + ".xml";

    return loadTable<datatable::EntityPathTable>(regionUrl,
        datatable::DataTableFactory::createEntityPathTable);
}


}} // namespace gideon { namespace serverbase {
