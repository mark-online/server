#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/WorldInfo.h>

namespace gideon { namespace datatable {
class RegionTable;
class RegionCoordinates;
class RegionSpawnTable;
class PositionSpawnTable;
class EntityPathTable;
}} // namespace gideon { namespace datatable {

namespace gideon { namespace serverbase {

/**
 * @class DataTableLoader
 */
struct GIDEON_SERVER_API DataTableLoader
{
    static bool loadPropertyTable();

    static bool loadCharacterStatusTable();

    static bool loadWorldMapTable();

    static bool loadEquipTable();

    static bool loadReprocessTable();

    static bool loadPlayerActiveSkillTable();

    static bool loadItemActiveSkillTable();

    static bool loadPlayerPassiveSkillTable();

    static bool loadNpcActiveSkillTable();

    static bool loadSOActiveSkillTable();

    static bool loadSkillEffectTable();

    static bool loadNpcTable();

    static bool loadElementTable();

    static bool loadFragmentTable();

    static bool loadItemDropTable();

    static bool loadWorldDropTable();

    static bool loadWorldDropSuffixTable();

    static bool loadGemTable();

    static bool loadRecipeTable();

    static bool loadExpTable();

    static bool loadSelectProductionTable();

    static bool loadRandomDungeonTable();

	static bool loadQuestTable();
    static bool loadQuestItemTable();
	static bool loadQuestKillMissionTable();
    static bool loadQuestActivationMissionTable();
    static bool loadQuestObtainMissionTable();
    static bool loadQuestProbeMissionTable();
    static bool loadQuestTransportMissionTable();
	static bool loadQuestContentsMissionTable();

	static bool loadHarvestTable();

	static bool loadTreasureTable();

    static bool loadNpcSellTable();

	static bool loadNpcBuyTable();

    static bool loadAnchorTable();

	static bool loadArenaTable();

    static bool loadAccessoryTable();

    static bool loadBuildingTable();

    static bool loadResourcesProductionTable();

    static bool loadFactionTable();

    static bool loadEventTriggerTable();

    static bool loadDeviceTable();

	static bool loadGliderTable();

    static bool loadFunctionTable();

	static bool loadVehicleTable();

	static bool loadHarnessTable();

    static bool loadActionTable();

    static bool loadNpcFormationTable();

    static bool loadNpcTalkingTable();

    static bool loadBuildingGuardTable();

    static bool loadBuildingGuardSellTable();

    static bool loadWorldEventTable();
    
    static bool loadWorldEventMissionTable();
    
    static bool loadWorldEventInvaderSpawnTable();
    
    static bool loadWorldEventMissionSpawnTable();

    static bool loadItemOptionTable();

    static bool loadItemSuffixTable();

    static bool loadCharacterDefaultItemTable();

    static bool loadCharacterDefaultSkillTable();

    static bool loadAchievementTable();

    static bool loadGuildLevelTable();

    static bool loadGuildSkillTable();

    static std::unique_ptr<datatable::RegionTable>
        loadRegionTable(MapCode mapCode);

    static std::unique_ptr<datatable::RegionCoordinates>
        loadRegionCoordinates(MapCode mapCode);

    static std::unique_ptr<datatable::RegionSpawnTable>
        loadRegionSpawnTable(MapCode mapCode);

    static std::unique_ptr<datatable::PositionSpawnTable>
        loadPositionSpawnTable(MapCode mapCode);

    static std::unique_ptr<datatable::EntityPathTable>
        loadEntityPathTable(MapCode mapCode);
};

}} // namespace gideon { namespace serverbase {
