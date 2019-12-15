#pragma once

#include "SqlServerCommonDatabaseCommands.h"

namespace gideon { namespace serverbase {

/**
 * @struct GetShardInfoCommand
 */
struct GetShardInfoCommand : RecordSetCommand
{
    sne::database::AdoIntegerParameter paramShardId_;

    GetShardInfoCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_shard"),
        paramShardId_(adoCommand_) {}
};


/**
 * @struct GetZoneInfoCommand
 */
struct GetZoneInfoCommand : Command
{
    sne::database::AdoIntegerParameter paramId_;
    CodeParameter paramGlobalWorldMapCode_;
    sne::database::AdoIntegerParameter paramShardId_;
    sne::database::AdoTinyIntParameter paramIsFirstZone_;
    sne::database::AdoVarCharParameter paramName_;

    GetZoneInfoCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_zone"),
        paramId_(adoCommand_, ADODB::adParamOutput),
        paramGlobalWorldMapCode_(adoCommand_, ADODB::adParamOutput),
        paramShardId_(adoCommand_, ADODB::adParamOutput),
        paramIsFirstZone_(adoCommand_, ADODB::adParamOutput),
        paramName_(adoCommand_, maxServerNameLength) {}
};


/**
 * @struct GetZoneInfoListCommand
 */
struct GetZoneListCommand : RecordSetCommand
{
    sne::database::AdoIntegerParameter paramShardId_;

    GetZoneListCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_zone_list"),
        paramShardId_(adoCommand_) {}
};


/**
 * @struct GetWorldTimeCommand
 */
struct GetWorldTimeCommand : Command
{
    sne::database::AdoSmallIntParameter paramWorldTime_;
    sne::database::AdoIntegerParameter paramZoneId_;

    GetWorldTimeCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_world_time"),
        paramWorldTime_(adoCommand_, ADODB::adParamOutput),
        paramZoneId_(adoCommand_) {}
};


/**
 * @struct UpdateWorldTimeCommand
 */
struct UpdateWorldTimeCommand : Command
{
    sne::database::AdoIntegerParameter paramZoneId_;
    sne::database::AdoSmallIntParameter paramWorldTime_;

    UpdateWorldTimeCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_world_time"),
        paramZoneId_(adoCommand_),
        paramWorldTime_(adoCommand_) {}
};


/**
 * @struct GetMaxInventoryIdCommand
 */
struct GetMaxInventoryIdCommand : Command
{
    IdParameter paramItemId_;
    IdParameter paramMinId_;
    IdParameter paramMaxId_;

    GetMaxInventoryIdCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_max_inventory_id"),
        paramItemId_(adoCommand_, ADODB::adParamOutput),
        paramMinId_(adoCommand_), 
        paramMaxId_(adoCommand_) {}
};


/**
* @struct GetRecentLoginUserListCommand
 */
struct GetRecentLoginUserListCommand : RecordSetCommand
{
    sne::database::AdoSmallIntParameter paramMaxCount_;

    GetRecentLoginUserListCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_recent_login_user_list"),
        paramMaxCount_(adoCommand_) {}
};


/**
 * @struct GetUserInfoCommand
 */
struct GetFullUserInfoCommand : RecordSetCommand
{
    IdParameter paramAccountId_;

    GetFullUserInfoCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_user_info"),
        paramAccountId_(adoCommand_) {}
};


/**
 * @struct CreateCharacterCommand
 */
struct CreateCharacterCommand : Command
{
    sne::database::AdoIntegerParameter paramErrorCode_;
    IdParameter paramCharacterId_;
    IdParameter paramAccountId_;
    sne::database::AdoVarWCharParameter paramNickname_;
    sne::database::AdoUnsignedTinyIntParameter paramEquipType_;
    sne::database::AdoUnsignedTinyIntParameter paramCharacterClass_;
    sne::database::AdoUnsignedTinyIntParameter paramSex_;
	sne::database::AdoUnsignedTinyIntParameter paramHair_;
	sne::database::AdoUnsignedTinyIntParameter paramFace_;
	sne::database::AdoUnsignedTinyIntParameter paramEyes_;
    CodeParameter paramHelmet_;
    CodeParameter paramShoulder_;
    CodeParameter paramArmor_;
    CodeParameter paramWaist_;
    CodeParameter paramTrousers_;
    CodeParameter paramGaiters_;
    CodeParameter paramGloves_;
    CodeParameter paramShoes_;
    CodeParameter paramRightHand_;
    CodeParameter paramLeftHand_;
    CodeParameter paramBothHands_;
    IdParameter paramHelmetId_;
    IdParameter paramShoulderId_;
    IdParameter paramArmorId_;
    IdParameter paramWaistId_;
    IdParameter paramTrousersId_;
    IdParameter paramGaitersId_;
    IdParameter paramGlovesId_;
    IdParameter paramShoesId_;
    IdParameter paramRightHandId_;
    IdParameter paramLeftHandId_;
    IdParameter paramBothHandsId_;
    sne::database::AdoIntegerParameter paramZoneId_;
    sne::database::AdoFloat32Parameter paramPosX_;
    sne::database::AdoFloat32Parameter paramPosY_;
    sne::database::AdoFloat32Parameter paramPosZ_;
    sne::database::AdoUnsignedTinyIntParameter paramPosHeading_;
    sne::database::AdoUnsignedTinyIntParameter paramInventoryCount_;

    CreateCharacterCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.create_character"),
        paramErrorCode_(adoCommand_, ADODB::adParamReturnValue),
        paramCharacterId_(adoCommand_, ADODB::adParamOutput),
        paramAccountId_(adoCommand_),
        paramNickname_(adoCommand_, maxNicknameLength),
        paramEquipType_(adoCommand_),
        paramCharacterClass_(adoCommand_),
        paramSex_(adoCommand_),
		paramHair_(adoCommand_),
		paramFace_(adoCommand_),
		paramEyes_(adoCommand_),
        paramHelmet_(adoCommand_),
        paramShoulder_(adoCommand_),
        paramArmor_(adoCommand_),
        paramWaist_(adoCommand_),
        paramTrousers_(adoCommand_),
        paramGaiters_(adoCommand_),
        paramGloves_(adoCommand_),
        paramShoes_(adoCommand_),
        paramRightHand_(adoCommand_),
        paramLeftHand_(adoCommand_),
        paramBothHands_(adoCommand_),    
        paramHelmetId_(adoCommand_),
        paramShoulderId_(adoCommand_),
        paramArmorId_(adoCommand_),
        paramWaistId_(adoCommand_),
        paramTrousersId_(adoCommand_),
        paramGaitersId_(adoCommand_),
        paramGlovesId_(adoCommand_),
        paramShoesId_(adoCommand_),
        paramRightHandId_(adoCommand_),
        paramLeftHandId_(adoCommand_),
        paramBothHandsId_(adoCommand_),
        paramZoneId_(adoCommand_),
        paramPosX_(adoCommand_),
        paramPosY_(adoCommand_),
        paramPosZ_(adoCommand_),
        paramPosHeading_(adoCommand_),
        paramInventoryCount_(adoCommand_) {}
};


/**
 * @struct DeleteCharacterCommand
 */
struct DeleteCharacterCommand : Command
{
    sne::database::AdoIntegerParameter paramErrorCode_;
	IdParameter paramGuildId_;
    IdParameter paramCharacterId_;

    DeleteCharacterCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.delete_character"),
        paramErrorCode_(adoCommand_, ADODB::adParamReturnValue),
        paramGuildId_(adoCommand_, ADODB::adParamOutput),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct CheckDuplicateNicknameCommand
 */
struct CheckDuplicateNicknameCommand : Command
{
    sne::database::AdoUnsignedTinyIntParameter paramExistNickname_;
    sne::database::AdoVarWCharParameter paramNickname_;

    CheckDuplicateNicknameCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.check_duplicate_nickname"),
        paramExistNickname_(adoCommand_, ADODB::adParamReturnValue),
        paramNickname_(adoCommand_, maxNicknameLength) {}
};



/**
 * @struct SaveCharacterStatsCommand
 */
struct SaveCharacterStatsCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    sne::database::AdoUnsignedTinyIntParameter paramLevel_;
    sne::database::AdoUnsignedTinyIntParameter paramLevelStep_;
    sne::database::AdoIntegerParameter paramExp_;
    sne::database::AdoUnsignedTinyIntParameter paramCharacterState_;
    sne::database::AdoSmallIntParameter paramSp_;
    sne::database::AdoIntegerParameter paramHp_;
    sne::database::AdoIntegerParameter paramMp_;
    sne::database::AdoFloat32Parameter paramPosX_;
    sne::database::AdoFloat32Parameter paramPosY_;
    sne::database::AdoFloat32Parameter paramPosZ_;
    sne::database::AdoUnsignedTinyIntParameter paramPosHeading_;
    sne::database::AdoIntegerParameter paramZoneId_;
	sne::database::AdoBigIntParameter paramGameMoney_;
	sne::database::AdoBigIntParameter paramBankGameMoney_;
    sne::database::AdoSmallIntParameter paramChaotic_;
    sne::database::AdoUnsignedTinyIntParameter paramIsActionBarLock_;
    sne::database::AdoUnsignedTinyIntParameter paramCharacterClass_;
    sne::database::AdoIntegerParameter paramArenaPoint_;
    sne::database::AdoIntegerParameter paramEventCoin_;
    sne::database::AdoIntegerParameter paramForgeCoin_;

    SaveCharacterStatsCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.save_character_stats"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramLevel_(adoCommand_),
        paramLevelStep_(adoCommand_),
        paramExp_(adoCommand_),
        paramCharacterState_(adoCommand_),
        paramSp_(adoCommand_),
        paramHp_(adoCommand_),
        paramMp_(adoCommand_),
        paramPosX_(adoCommand_),
        paramPosY_(adoCommand_),
        paramPosZ_(adoCommand_),
        paramPosHeading_(adoCommand_),
        paramZoneId_(adoCommand_),
		paramGameMoney_(adoCommand_),
		paramBankGameMoney_(adoCommand_),
        paramChaotic_(adoCommand_),
        paramIsActionBarLock_(adoCommand_),
        paramCharacterClass_(adoCommand_),
        paramArenaPoint_(adoCommand_),
        paramEventCoin_(adoCommand_),
		paramForgeCoin_(adoCommand_){}
};


/**
 * @struct LoadCharacterPropertiesCommand
 */
struct LoadCharacterPropertiesCommand : Command
{
    enum { maxTextLength = 8000 };

    sne::database::AdoVarCharParameter paramConfig_;
    sne::database::AdoVarCharParameter paramPrefs_;
    IdParameter paramCharacterId_;

    LoadCharacterPropertiesCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.load_character_properties"),
        paramConfig_(adoCommand_, maxTextLength, ADODB::adParamOutput),
        paramPrefs_(adoCommand_, maxTextLength, ADODB::adParamOutput),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct SaveCharacterPropertiesCommand
 */
struct SaveCharacterPropertiesCommand : Command
{
    enum { maxTextLength = 8000 };

    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    sne::database::AdoVarCharParameter paramConfig_;
    sne::database::AdoVarCharParameter paramPrefs_;

    SaveCharacterPropertiesCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.save_character_properties"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramConfig_(adoCommand_, maxTextLength),
        paramPrefs_(adoCommand_, maxTextLength) {}
};


/**
 * @struct GetCharacterInfoCommand
 */
struct GetCharacterInfoCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;

    GetCharacterInfoCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_character_info"),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct UpdateCharacterPointsCommand
 */
struct UpdateCharacterPointsCommand : Command
{
    IdParameter paramCharacterId_;
    sne::database::AdoIntegerParameter hp_;
    sne::database::AdoIntegerParameter mp_;

    UpdateCharacterPointsCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_character_points"),
        paramCharacterId_(adoCommand_),
        hp_(adoCommand_),
        mp_(adoCommand_) {}
};


/**
 * @struct UpdateCharacterPointsCommand
 */
struct SaveSelectCharacterTitleCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramTitleCode_;
    
    SaveSelectCharacterTitleCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.save_select_character_title"),
        paramCharacterId_(adoCommand_),
        paramTitleCode_(adoCommand_) {}
};


/**
 * @struct MoveItemCommand
 */
struct MoveItemCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;

    MoveItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.move_inventory_item"),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_) {}
};


/**
 * @struct SwitchItemCommand
 */
struct SwitchItemCommand : Command
{
    IdParameter paramItemId1_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    IdParameter paramItemId2_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId2_;

    SwitchItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.switch_inventory_item"),
        paramItemId1_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemId2_(adoCommand_),
        paramSlotId2_(adoCommand_) {}
};


/**
 * @struct AddItemCommand
 */
struct AddItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;
	sne::database::AdoUnsignedTinyIntParameter paramInvenType_;
    sne::database::AdoBigIntParameter expireAt_;

    AddItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_inventory_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemCount_(adoCommand_),
		paramInvenType_(adoCommand_),
        expireAt_(adoCommand_) {}
};


/**
 * @struct AddEquipItemCommand
 */
struct AddEquipItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;
    sne::database::AdoBigIntParameter expireAt_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoUnsignedIntParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoUnsignedIntParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoUnsignedIntParameter paramOptionValue3_;
    sne::database::AdoUnsignedTinyIntParameter paramSocketCount_;
	sne::database::AdoUnsignedTinyIntParameter paramInvenType_;

    AddEquipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_inventory_equip_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramItemCount_(adoCommand_),
        expireAt_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_),
        paramSocketCount_(adoCommand_),
		paramInvenType_(adoCommand_) {}
};


/**
 * @struct AddEquipSocketOptionCommand
 */
struct AddEquipSocketOptionCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;

    sne::database::AdoUnsignedTinyIntParameter paramSlotId_;
    CodeParameter paramGemCode_;

    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoUnsignedIntParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoUnsignedIntParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoUnsignedIntParameter paramOptionValue3_;


    AddEquipSocketOptionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_equip_socket_option"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramGemCode_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};


/**
 * @struct RemoveEquipSocketOptionCommand
 */
struct RemoveEquipSocketOptionCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramSlotId_;
    
    RemoveEquipSocketOptionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_equip_socket_option"),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_) {}
};


/**
 * @struct AddAccessoryItemCommand
 */
struct AddAccessoryItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;
    sne::database::AdoBigIntParameter expireAt_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoUnsignedIntParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoUnsignedIntParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoUnsignedIntParameter paramOptionValue3_;
    sne::database::AdoUnsignedTinyIntParameter paramInvenType_;

    AddAccessoryItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_inventory_accessory_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemCount_(adoCommand_),
        expireAt_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_),
		paramInvenType_(adoCommand_) {}
};



/**
 * @struct ChangeEquipItemInfo
 */
struct ChangeEquipItemInfoCommand : Command
{
    IdParameter paramItemId_;
    CodeParameter paramNewItemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramSocketCount_;

    ChangeEquipItemInfoCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.change_equip_item_info"),
        paramItemId_(adoCommand_),
        paramNewItemCode_(adoCommand_),
        paramSocketCount_(adoCommand_) {}
};

/**
 * @struct UpdateEnchantEquipItemCommand
 */
struct UpdateEnchantEquipItemCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    CodeParameter paramSkillCode_;

    UpdateEnchantEquipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_equip_item_enchant_skill"),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramSkillCode_(adoCommand_) {}
};


/**
 * @struct UpdateEquipUpgradeRateCommand
 */
struct UpdateEquipUpgradeRateCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramUpgradeRate_;

    UpdateEquipUpgradeRateCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_equip_upgrade_rate"),
        paramItemId_(adoCommand_),
        paramUpgradeRate_(adoCommand_) {}
};


/**
 * @struct RemoveItemCommand
 */
struct RemoveItemCommand : Command
{
    IdParameter paramItemId_;
    
    RemoveItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_inventory_item"),
        paramItemId_(adoCommand_) {}
};


/**
 * @struct UpdateItemCountCommand
 */
struct UpdateItemCountCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;

    UpdateItemCountCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_inventory_item_count"),
        paramItemId_(adoCommand_),
        paramItemCount_(adoCommand_) {}
};


/**
 * @struct AddQuestItemCommand
 */
struct AddQuestItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    CodeParameter paramQuestItemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramUsableCount_;
	sne::database::AdoUnsignedTinyIntParameter paramStackCount_;
	CodeParameter paramQuestCode_;

    AddQuestItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_quest_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramQuestItemCode_(adoCommand_),
        paramUsableCount_(adoCommand_),
		paramStackCount_(adoCommand_),
		paramQuestCode_(adoCommand_) {}
};


/**
 * @struct RemoveQuestItemCommand
 */
struct RemoveQuestItemCommand : Command
{
    IdParameter paramItemId_;
    
    RemoveQuestItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_quest_item"),
        paramItemId_(adoCommand_) {}
};


/**
 * @struct UpdateQuestItemUsableCountCommand
 */
struct UpdateQuestItemUsableCountCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramUsableCount_;

    UpdateQuestItemUsableCountCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_quest_item_usable_count"),
        paramItemId_(adoCommand_),
        paramUsableCount_(adoCommand_) {}
};


/**
 * @struct UpdateQuestItemCountCommand
 */
struct UpdateQuestItemCountCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramStackCount_;

    UpdateQuestItemCountCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_quest_item_count"),
        paramItemId_(adoCommand_),
        paramStackCount_(adoCommand_) {}
};


/**
 * @struct EquipItemCommand
 */
struct EquipItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramEquipPart_;

    EquipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.equip_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramEquipPart_(adoCommand_) {}
};


/**
 * @struct UnequipItemCommand
 */
struct UnequipItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramEquipPart_;

    UnequipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.unequip_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramEquipPart_(adoCommand_) {}
};


/**
 * @struct ReplaceInventoryWithEquipItemCommand
 */
struct ReplaceInventoryWithEquipItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramInvenItemId_;
    IdParameter paramEquipItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramEquipPart_;
    sne::database::AdoUnsignedTinyIntParameter paramUnequipPart_;

    ReplaceInventoryWithEquipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.relpace_inventory_item_with_equip_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramInvenItemId_(adoCommand_),
        paramEquipItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramEquipPart_(adoCommand_),
        paramUnequipPart_(adoCommand_) {}
};



/**
 * @struct AccessoryItemCommand
 */
struct EquipAccessoryItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramAccessoryPart_;

    EquipAccessoryItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.equip_accessory_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramAccessoryPart_(adoCommand_) {}
};


/**
 * @struct UnequipAccessoryItemCommand
 */
struct UnequipAccessoryItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramAccessoryPart_;

    UnequipAccessoryItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.unequip_accessory_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramAccessoryPart_(adoCommand_) {}
};


/**
 * @struct ReplaceInventoryWithAccessoryItemCommand
 */
struct ReplaceInventoryWithAccessoryItemCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramInvenItemId_;
    IdParameter paramAccessoryItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramAccessoryPart_;
    sne::database::AdoUnsignedTinyIntParameter paramUnequipPart_;

    ReplaceInventoryWithAccessoryItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.relpace_inventory_item_with_accessory_item"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramInvenItemId_(adoCommand_),
        paramAccessoryItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramAccessoryPart_(adoCommand_),
        paramUnequipPart_(adoCommand_) {}
};



/**
 * @struct ChangeCharacterStateCommand
 */
struct ChangeCharacterStateCommand  : Command
{
    IdParameter paramCharacterId_;
    sne::database::AdoUnsignedTinyIntParameter paramCharacterState_;

    ChangeCharacterStateCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.change_character_state"),
        paramCharacterId_(adoCommand_),
        paramCharacterState_(adoCommand_) {}
};


/**
 * @struct SaveActionBarCommand
 */
struct SaveActionBarCommand : Command
{
    IdParameter paramCharacterId_;
    sne::database::AdoUnsignedTinyIntParameter paramActionBarIndex_;
    sne::database::AdoUnsignedTinyIntParameter paramActionBarPosition_;
    CodeParameter paramCode_;

    SaveActionBarCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.save_action_bar"),
        paramCharacterId_(adoCommand_),
        paramActionBarIndex_(adoCommand_),
        paramActionBarPosition_(adoCommand_),
        paramCode_(adoCommand_) {}
};


/**
 * @struct LearnSkillCommand
 */
struct LearnSkillCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    CodeParameter paramCurrentSkillCode_;
    CodeParameter paramLearnSkillCode_;

    LearnSkillCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.learn_skill"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramCurrentSkillCode_(adoCommand_),
        paramLearnSkillCode_(adoCommand_) {}
};


/**
 * @struct RemoveSkillCommand
 */
struct RemoveSkillCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramSkillCode_;

    RemoveSkillCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_skill"),
        paramCharacterId_(adoCommand_),
        paramSkillCode_(adoCommand_) {}
};


/**
 * @struct RemoveAllSkillCommand
 */
struct RemoveAllSkillCommand : Command
{
    IdParameter paramCharacterId_;

    RemoveAllSkillCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_all_skill"),
        paramCharacterId_(adoCommand_){}
};


/**
 * @struct GetQuestsCommand
 */
struct GetQuestsCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;

    GetQuestsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_quests"),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct GetRepeatQuestsCommand
 */
struct GetRepeatQuestsCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;

    GetRepeatQuestsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_repeat_quests"),
        paramCharacterId_(adoCommand_) {}
};

/**
 * @struct AcceptedQuestCommand
 */
struct AcceptedQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;

    AcceptedQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.accept_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_) {}
};

/**
 * @struct AcceptRepeatQuestCommand
 */
struct AcceptRepeatQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;
    sne::database::AdoDateParameter paramAcceptTime_;

    AcceptRepeatQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.accept_repeat_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_),
        paramAcceptTime_(adoCommand_) {}
};

/**
 * @struct CancelQuestCommand
 */
struct CancelQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;

    CancelQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.cancel_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_) {}
};

/**
 * @struct CancelRepeatQuestCommand
 */
struct CancelRepeatQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;

    CancelRepeatQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.cancel_repeat_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_) {}
};


/**
 * @struct CompleteQuestCommand
 */
struct CompleteQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;

    CompleteQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.complete_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_) {}
};


/**
 * @struct CompleteRepeatQuestCommand
 */
struct CompleteRepeatQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;
    sne::database::AdoDateParameter paramAcceptTime_;

    CompleteRepeatQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.complete_repeat_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_),
        paramAcceptTime_(adoCommand_) {}
};


/**
 * @struct UpdateCompleteRepeatQuestCommand
 */
struct UpdateCompleteRepeatQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;
    sne::database::AdoDateParameter paramAcceptTime_;

    UpdateCompleteRepeatQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_complete_repeat_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_),
        paramAcceptTime_(adoCommand_) {}
};


/**
 * @struct InsertQuestMissionCommand
 */
struct InsertQuestMissionCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;
	CodeParameter paramQuestMissionCode_;
	CodeParameter paramQuestGoalCode_;
	sne::database::AdoUnsignedIntParameter paramQuestGoalValue_;

    InsertQuestMissionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.insert_quest_mission"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_),
		paramQuestMissionCode_(adoCommand_),
		paramQuestGoalCode_(adoCommand_),
		paramQuestGoalValue_(adoCommand_) {}
};


/**
 * @struct InsertRepeatQuestMissionCommand
 */
struct InsertRepeatQuestMissionCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;
	CodeParameter paramQuestMissionCode_;
	CodeParameter paramQuestGoalCode_;
	sne::database::AdoUnsignedIntParameter paramQuestGoalValue_;

    InsertRepeatQuestMissionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.insert_repeat_quest_mission"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_),
		paramQuestMissionCode_(adoCommand_),
		paramQuestGoalCode_(adoCommand_),
		paramQuestGoalValue_(adoCommand_) {}
};


/**
 * @struct UpdateQuestMissionCommand
 */
struct UpdateQuestMissionCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;
	CodeParameter paramQuestMissionCode_;
	CodeParameter paramQuestGoalCode_;
	sne::database::AdoUnsignedIntParameter paramQuestGoalValue_;

    UpdateQuestMissionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_quest_mission"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_),
		paramQuestMissionCode_(adoCommand_),
		paramQuestGoalCode_(adoCommand_),
		paramQuestGoalValue_(adoCommand_) {}
};

/**
 * @struct UpdateRepeatQuestMissionCommand
 */
struct UpdateRepeatQuestMissionCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;
	CodeParameter paramQuestMissionCode_;
	CodeParameter paramQuestGoalCode_;
	sne::database::AdoUnsignedIntParameter paramQuestGoalValue_;

    UpdateRepeatQuestMissionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_repeat_quest_mission"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_),
		paramQuestMissionCode_(adoCommand_),
		paramQuestGoalCode_(adoCommand_),
		paramQuestGoalValue_(adoCommand_) {}
};


/**
 * @struct RemoveCompleteRepeatQuest
 */
struct RemoveCompleteRepeatQuest : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_;

    RemoveCompleteRepeatQuest(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_complete_repeat_quest"),
        paramCharacterId_(adoCommand_),
        paramQuestCode_(adoCommand_) {}
};


/**
 * @struct RemoveCompleteQuestCommand
 */
struct RemoveCompleteQuestCommand : Command
{
    IdParameter paramCharacterId_;
    CodeParameter paramQuestCode_; 

    RemoveCompleteQuestCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_complete_quest"),
        paramCharacterId_(adoCommand_),
		paramQuestCode_(adoCommand_) {}
};


/**
 * @struct CreateGuildCommand
 */
struct CreateGuildCommand : Command
{
    IdParameter paramId_;
    sne::database::AdoVarWCharParameter paramGuildName_;
    CodeParameter paramGuildMarkCode_; 

    CreateGuildCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.create_guild"),
        paramId_(adoCommand_),
		paramGuildName_(adoCommand_, maxGuildNameLength),
        paramGuildMarkCode_(adoCommand_) {}
};


/**
 * @struct AddGuildMemberCommand
 */
struct AddGuildMemberCommand : Command
{
    IdParameter paramCharacterId_;
    IdParameter paramGuildId_;
    sne::database::AdoUnsignedTinyIntParameter paramPosition_;
    sne::database::AdoTinyIntParameter paramRankId_;
    sne::database::AdoIntegerParameter paramContributiveness_;

    AddGuildMemberCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_member"),
        paramCharacterId_(adoCommand_),
		paramGuildId_(adoCommand_),
        paramPosition_(adoCommand_),
        paramRankId_(adoCommand_),
        paramContributiveness_(adoCommand_) {}
};


/**
 * @struct RemoveGuildMemberCommand
 */
struct RemoveGuildMemberCommand : Command
{
    IdParameter paramCharacterId_;

    RemoveGuildMemberCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_guild_member"),
        paramCharacterId_(adoCommand_) {}
};

/**
 * @struct RemoveGuildCommand
 */
struct RemoveGuildCommand : Command
{
    IdParameter paramGuildId_;

    RemoveGuildCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_guild"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct AddGuildRelationshipCommand
 */
struct AddGuildRelationshipCommand : Command
{
    IdParameter paramOwnerGuildId_;
    IdParameter paramTargetGuildId_;
    sne::database::AdoTinyIntParameter paramRelationshipType_;
    sne::database::AdoBigIntParameter paramRelationshipAt_;

    AddGuildRelationshipCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_relationship"),
        paramOwnerGuildId_(adoCommand_),
        paramTargetGuildId_(adoCommand_),
        paramRelationshipType_(adoCommand_),
        paramRelationshipAt_(adoCommand_)
    {}

};


/**
 * @struct RemoveGuildCommand
 */
struct RemoveGuildRelationshipCommand : Command
{
    IdParameter paramOwnerGuildId_;
    IdParameter paramTargetGuildId_;

    RemoveGuildRelationshipCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_guild_relationship"),
        paramOwnerGuildId_(adoCommand_),
        paramTargetGuildId_(adoCommand_) {}
};


/**
 * @struct ChangeGuildMemberPositionCommand
 */
struct ChangeGuildMemberPositionCommand : Command
{
    IdParameter paramCharacterId_;
    sne::database::AdoUnsignedTinyIntParameter paramPosition_;

    ChangeGuildMemberPositionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.change_guild_member_position"),
        paramCharacterId_(adoCommand_),
        paramPosition_(adoCommand_) {}
};


/**
 * @struct AddGuildApplicantCommand
 */
struct AddGuildApplicantCommand : Command
{
    IdParameter paramCharacterId_;
    IdParameter paramGuildId_;

    AddGuildApplicantCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_applicant"),
        paramCharacterId_(adoCommand_),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct RemoveGuildApplicantCommand
 */
struct RemoveGuildApplicantCommand : Command
{
    IdParameter paramCharacterId_;

    RemoveGuildApplicantCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_guild_applicant"),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct ModifyGuildIntroductionCommand
 */
struct ModifyGuildIntroductionCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoVarWCharParameter paramIntroduction_;
    
    ModifyGuildIntroductionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.modify_guild_introduction"),
        paramGuildId_(adoCommand_),
        paramIntroduction_(adoCommand_, maxGuildIntroduction) {}
};


/**
 * @struct ModifyGuildNoticeCommand
 */
struct ModifyGuildNoticeCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoVarWCharParameter paramNotice_;

    ModifyGuildNoticeCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.modify_guild_notice"),
        paramGuildId_(adoCommand_),
        paramNotice_(adoCommand_, maxGuildNotice) {}
};


/**
 * @struct UpdateGuildGameMoneyCommand
 */
struct UpdateGuildGameMoneyCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoBigIntParameter paramGameMoney_;

    UpdateGuildGameMoneyCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_game_money"),
        paramGuildId_(adoCommand_),
        paramGameMoney_(adoCommand_) {}
};


/**
 * @struct UpdateGuildMemberWithdrawCommand
 */
struct UpdateGuildMemberWithdrawCommand : Command
{
    IdParameter paramPlayerId_;
    sne::database::AdoBigIntParameter paramWithdraw_;
    sne::database::AdoBigIntParameter paramResetWithdrawTime_;

    UpdateGuildMemberWithdrawCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_member_withdraw"),
        paramPlayerId_(adoCommand_),
        paramWithdraw_(adoCommand_),
        paramResetWithdrawTime_(adoCommand_){}
};


/**
 * @struct UpdateGuildVaultNameCommand
 */
struct UpdateGuildVaultNameCommand : Command
{
    sne::database::AdoUnsignedTinyIntParameter paramId_;
    IdParameter paramGuildId_;
    sne::database::AdoVarWCharParameter paramName_;

    UpdateGuildVaultNameCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_vault_name"),
        paramId_(adoCommand_),
        paramGuildId_(adoCommand_),
        paramName_(adoCommand_, maxGuildVaultNameLength) {}
};


/**
 * @struct AddGuildVaultCommand
 */
struct AddGuildVaultCommand : Command
{
    sne::database::AdoUnsignedTinyIntParameter paramId_;
    IdParameter paramGuildId_;
    sne::database::AdoVarWCharParameter paramName_;

    AddGuildVaultCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_valult"),
        paramId_(adoCommand_),
        paramGuildId_(adoCommand_),
        paramName_(adoCommand_, maxGuildVaultNameLength) {}
};


/**
 * @struct GetGuildInfos
 */
struct GetGuildInfosCommand : RecordSetCommand
{
    GetGuildInfosCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_infos") {}
};


/**
 * @struct GetGuildRanksCommand
 */
struct GetGuildRanksCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildRanksCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_ranks"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetGuildBankRightsCommand
 */
struct GetGuildBankRightsCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildBankRightsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_bank_rights"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetGuildSkillsCommand
 */
struct GetGuildSkillsCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildSkillsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_skills"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetGuildMemberInfosCommand
 */
struct GetGuildMemberInfosCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildMemberInfosCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_member_infos"),
        paramGuildId_(adoCommand_) {}
};

/**
 * @struct GetGuildRelationshipCommand
 */
struct GetGuildRelationshipsCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildRelationshipsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_relationships"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GuildApplicantsCommand
 */
struct GuildApplicantsCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GuildApplicantsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_applicants"),
        paramGuildId_(adoCommand_) {}
};

/**
 * @struct GetGuildInventoryCommand
 */
struct GetGuildInventoryCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildInventoryCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_inventory"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetGuildVaultsCommand
 */
struct GetGuildVaultsCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildVaultsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_vaults"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetGuildEventLpgCommand
 */
struct GetGuildEventLogCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildEventLogCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_event_log"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetGuildBankEventLogCommand
 */
struct GetGuildBankEventLogCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildBankEventLogCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_bank_event_log"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetGuildGameMoneyEventLogCommand
 */
struct GetGuildGameMoneyEventLogCommand : RecordSetCommand
{
    IdParameter paramGuildId_;

    GetGuildGameMoneyEventLogCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_guild_game_money_event_log"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct AddGuildItemCommand
 */
struct AddGuildItemCommand : Command
{
    IdParameter paramItemId_;
    IdParameter paramGuildId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramVaultId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;

    AddGuildItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_inventory_item"),
        paramItemId_(adoCommand_),
        paramGuildId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramVaultId_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemCount_(adoCommand_) {}
};


/**
 * @struct AddGuildEquipItemCommand
 */
struct AddGuildEquipItemCommand : Command
{
    IdParameter paramGuildId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;;
    sne::database::AdoUnsignedTinyIntParameter paramVaultId_;

    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;
    sne::database::AdoUnsignedTinyIntParameter paramSocketCount_;


    AddGuildEquipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_inventory_equip_item"),
        paramGuildId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramItemCount_(adoCommand_),
        paramVaultId_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_),
        paramSocketCount_(adoCommand_) {}
};


/**
 * @struct AddGuildEquipSocketOptionCommand
 */
struct AddGuildEquipSocketOptionCommand : Command
{
    IdParameter paramGuildId_;
    IdParameter paramItemId_;

    sne::database::AdoUnsignedTinyIntParameter paramSlotId_;
    CodeParameter paramGemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;


    AddGuildEquipSocketOptionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_equip_socket_option"),
        paramGuildId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramGemCode_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};


/**
 * @struct AddGuildAccessoryItemCommand
 */
struct AddGuildAccessoryItemCommand : Command
{
    IdParameter paramGuildId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;;
    sne::database::AdoUnsignedTinyIntParameter paramVaultId_;

    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;


    AddGuildAccessoryItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_inventory_accessory_item"),
        paramGuildId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramItemCount_(adoCommand_),
        paramVaultId_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};


/**
 * @struct RemoveGuildItemCommand
 */
struct RemoveGuildItemCommand : Command
{
    IdParameter paramItemId_;
    
    RemoveGuildItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_guild_inventory_item"),
        paramItemId_(adoCommand_) {}
};


/**
 * @struct UpdateGuildItemCountCommand
 */
struct UpdateGuildItemCountCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;

    UpdateGuildItemCountCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_inventory_item_count"),
        paramItemId_(adoCommand_),
        paramItemCount_(adoCommand_) {}
};


/**
 * @struct MoveGuildItemCommand
 */
struct MoveGuildItemCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramVaultId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;

    MoveGuildItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.move_guild_inventory_item"),
        paramItemId_(adoCommand_),
        paramVaultId_(adoCommand_),
        paramSlotId_(adoCommand_) {}
};


/**
 * @struct SwitchGuildItemCommand
 */
struct SwitchGuildItemCommand : Command
{
    IdParameter paramItemId1_;
    sne::database::AdoUnsignedTinyIntParameter paramVaultId1_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    IdParameter paramItemId2_;
    sne::database::AdoUnsignedTinyIntParameter paramVaultId2_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId2_;

    SwitchGuildItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.switch_guild_inventory_item"),
        paramItemId1_(adoCommand_),
        paramVaultId1_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemId2_(adoCommand_),
        paramVaultId2_(adoCommand_),
        paramSlotId2_(adoCommand_) {}
};


/**
 * @struct GetMaxGuildIdCommand
 */
struct GetMaxGuildIdCommand : Command
{
    IdParameter paramItemId_;

    GetMaxGuildIdCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_max_guild_id"),
        paramItemId_(adoCommand_, ADODB::adParamOutput) {}
};


/**
 * @struct GetPlayerIdCommand
 */
struct GetPlayerIdCommand : Command
{
    IdParameter paramPlayerId_;
    sne::database::AdoVarWCharParameter paramNickname_;

    GetPlayerIdCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_character_id"),
        paramPlayerId_(adoCommand_, ADODB::adParamReturnValue),
        paramNickname_(adoCommand_, maxNicknameLength) {}
};

/**
 * @struct GetMailsCommand
 */
struct GetMailsCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;

    GetMailsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_mails"),
        paramCharacterId_(adoCommand_) {}
};



/**
 * @struct InsertMailCommand
 */
struct InsertMailCommand : Command
{
    IdParameter paramMailId_;
    IdParameter paramCharacterId_;
    sne::database::AdoVarWCharParameter paramNickname_;
    IdParameter paramSendId_;
    CodeParameter paramDataCode_;
    sne::database::AdoVarWCharParameter paramTitle_;
    sne::database::AdoVarWCharParameter paramBody_;
    sne::database::AdoDateParameter sentAt_;
    sne::database::AdoBigIntParameter paramGameMoney_;
    sne::database::AdoTinyIntParameter paramMailType_;

    InsertMailCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.insert_mail"),
        paramMailId_(adoCommand_, ADODB::adParamOutput),
        paramCharacterId_(adoCommand_),
        paramNickname_(adoCommand_, maxNicknameLength),
        paramSendId_(adoCommand_),
        paramDataCode_(adoCommand_),
        paramTitle_(adoCommand_, maxMailTitle),
        paramBody_(adoCommand_, maxMailBody),
        sentAt_(adoCommand_),
        paramGameMoney_(adoCommand_),
        paramMailType_(adoCommand_)
    {
    }
};


/**
 * @struct AddItemCommand
 */
struct AddMailItemCommand : Command
{
    IdParameter paramMailId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;

    AddMailItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_mail_item"),
        paramMailId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramItemCount_(adoCommand_) {}
};


/**
 * @struct AddMailEquipItemCommand
 */
struct AddMailEquipItemCommand : Command
{
    IdParameter paramMailId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;

    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;

    sne::database::AdoUnsignedTinyIntParameter paramSocketCount_;


    AddMailEquipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_mail_equip_item"),
        paramMailId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramItemCount_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_),
        paramSocketCount_(adoCommand_)  {}
};




/**
 * @struct AddMailEquipSocketOptionCommand
 */
struct AddMailEquipSocketOptionCommand : Command
{
    IdParameter paramMailId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramSlotId_;
    CodeParameter paramGemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;

    AddMailEquipSocketOptionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_mail_equip_socket_option"),
        paramMailId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramGemCode_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};


/**
 * @struct AddMailAccessoryItemCommand
 */
struct AddMailAccessoryItemCommand : Command
{
    IdParameter paramMailId_;
    IdParameter paramCharacterId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;

    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;

    AddMailAccessoryItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_mail_accessory_item"),
        paramMailId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramItemCount_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};

/**
 * @struct DeleteMailItemCommand
 */
struct DeleteMailItemCommand : Command
{
    IdParameter paramMailId_;

    DeleteMailItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.delete_mail_item"),
        paramMailId_(adoCommand_)
    {
    }
};

/**
 * @struct DeleteMailCommand
 */
struct DeleteMailCommand : Command
{
    IdParameter paramMailId_;

    DeleteMailCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.delete_mail"),
        paramMailId_(adoCommand_)
    {
    }
};


/**
 * @struct ReadMailCommand
 */
struct ReadMailCommand : Command
{
    IdParameter paramMailId_;

    ReadMailCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.read_mail"),
        paramMailId_(adoCommand_)
    {
    }
};


/**
 * @struct HasNotReadMailCommand
 */
struct HasNotReadMailCommand : Command
{
    sne::database::AdoUnsignedTinyIntParameter paramHasNotReadMail_;
    IdParameter paramCharacterId_;

    HasNotReadMailCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.has_not_read_mail"),
        paramHasNotReadMail_(adoCommand_, ADODB::adParamReturnValue),
        paramCharacterId_(adoCommand_)
    {
    }
};


/**
 * @struct GetAuctionsCommand
 */
struct GetAuctionsCommand : RecordSetCommand
{
    IdParameter paramZoneId_;

    GetAuctionsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_auctions"),
        paramZoneId_(adoCommand_) {}
};


/**
 * @struct InsertAuctionCommand
 */
struct InsertAuctionCommand : Command
{
    IdParameter paramAuctionId_;
    IdParameter paramZoneId_;
    CodeParameter paramNpcCode_;
    IdParameter paramSellerId_;    
    sne::database::AdoBigIntParameter paramBidMoney_;
    sne::database::AdoBigIntParameter paramBuyoutMoney_;
    sne::database::AdoBigIntParameter paramDepositFee_;
    sne::database::AdoDateParameter expireAt_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoTinyIntParameter paramItemCount_;

    InsertAuctionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.insert_auction"),
        paramAuctionId_(adoCommand_),
        paramZoneId_(adoCommand_),
        paramNpcCode_(adoCommand_),
        paramSellerId_(adoCommand_),
        paramBidMoney_(adoCommand_),
        paramBuyoutMoney_(adoCommand_),
        paramDepositFee_(adoCommand_),
        expireAt_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramItemCount_(adoCommand_)
    {
    }
};


/**
 * @struct AddAuctionEquipInfoCommand
 */
struct AddAuctionEquipInfoCommand : Command
{
    IdParameter paramAuctionId_;
    IdParameter paramZoneId_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;

    sne::database::AdoUnsignedTinyIntParameter paramSocketCount_;


    AddAuctionEquipInfoCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_auction_equip_info"),
        paramAuctionId_(adoCommand_),
        paramZoneId_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_),
        paramSocketCount_(adoCommand_)  {}
};


/**
 * @struct AddAuctionEquipSocketOptionCommand
 */
struct AddAuctionEquipSocketOptionCommand : Command
{
    IdParameter paramAuctionId_;
    IdParameter paramZoneId_;
    sne::database::AdoUnsignedTinyIntParameter paramSlotId_;
    CodeParameter paramGemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;
    

    AddAuctionEquipSocketOptionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_auction_equip_socket_option"),
        paramAuctionId_(adoCommand_),
        paramZoneId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramGemCode_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};



/**
 * @struct AddAuctionAccessoryInfoCommand
 */
struct AddAuctionAccessoryInfoCommand : Command
{
    IdParameter paramAuctionId_;
    IdParameter paramZoneId_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;

    AddAuctionAccessoryInfoCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_auction_accessory_info"),
        paramAuctionId_(adoCommand_),
        paramZoneId_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};


/**
 * @struct UpdateBidCommand
 */
struct UpdateBidCommand : Command
{
    IdParameter paramAuctionId_;
    IdParameter paramZoneId_;
    IdParameter paramBuyerId_;
    sne::database::AdoBigIntParameter paramBidMoney_;

    UpdateBidCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_bid"),
        paramAuctionId_(adoCommand_),
        paramZoneId_(adoCommand_),
        paramBuyerId_(adoCommand_),
        paramBidMoney_(adoCommand_) {}
};


/**
 * @struct DeleteAuctionCommand
 */
struct DeleteAuctionCommand : Command
{
    IdParameter paramAuctionId_;
    IdParameter paramZoneId_;

    DeleteAuctionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.delete_auction"),
        paramAuctionId_(adoCommand_),
        paramZoneId_(adoCommand_) {}
};

/**
 * @struct GetPlayerArenaRecordsCommand
 */
struct GetPlayerArenaRecordsCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;

    GetPlayerArenaRecordsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_character_arena_records"),
        paramCharacterId_(adoCommand_) {}
};

/**
 * @struct UpdateArenaRecordCommand
 */
struct UpdateArenaRecordCommand : Command
{
    IdParameter paramCharacterId_;   
    sne::database::AdoUnsignedTinyIntParameter paramMode_;
    sne::database::AdoUnsignedIntParameter paramScore_;
    sne::database::AdoUnsignedIntParameter paramWin_;
    sne::database::AdoUnsignedIntParameter paramLose_;
    sne::database::AdoUnsignedIntParameter paramDraw_;

    UpdateArenaRecordCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_arena_record"),
        paramCharacterId_(adoCommand_),
        paramMode_(adoCommand_),
        paramScore_(adoCommand_),
        paramWin_(adoCommand_),
        paramLose_(adoCommand_),
        paramDraw_(adoCommand_) {}
};


/**
 * @struct UpdateArenaPointCommand
 */
struct UpdateArenaPointCommand : Command
{
    IdParameter paramCharacterId_;
    sne::database::AdoUnsignedIntParameter paramArenaPoint_;

    UpdateArenaPointCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_arena_point"),
        paramCharacterId_(adoCommand_),
        paramArenaPoint_(adoCommand_) {}
};



/**
 * @struct UpdateDeserterExpireTimeCommand
 */
struct UpdateDeserterExpireTimeCommand : Command
{
    IdParameter paramCharacterId_;
    sne::database::AdoBigIntParameter paramDeserterExpireTime_;

    UpdateDeserterExpireTimeCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_deserter_expired_at"),
        paramCharacterId_(adoCommand_),
        paramDeserterExpireTime_(adoCommand_) {}
};


/**
 * @struct CreateBuildingCommand
 */
struct CreateBuildingCommand  : Command
{
    IdParameter paramBuildingId_;
    CodeParameter paramBuildingCode_;    
    sne::database::AdoFloat32Parameter paramPosX_;
    sne::database::AdoFloat32Parameter paramPosY_;
    sne::database::AdoFloat32Parameter paramPosZ_;
    sne::database::AdoUnsignedTinyIntParameter paramPosHeading_;
    sne::database::AdoUnsignedIntParameter paramCurrentHp_;
    sne::database::AdoUnsignedTinyIntParameter paramState_;
    sne::database::AdoBigIntParameter paramBuildTime_;
    sne::database::AdoBigIntParameter paramExpireTime_;
    sne::database::AdoUnsignedTinyIntParameter paramOwnerType_;
    IdParameter paramOwnerCharacterId_;
    IdParameter paramOwnerGuildId_;
    sne::database::AdoBigIntParameter paramCreateTime_;

    CreateBuildingCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.create_building"),
        paramBuildingId_(adoCommand_),
        paramBuildingCode_(adoCommand_),
        paramPosX_(adoCommand_),
        paramPosY_(adoCommand_),
        paramPosZ_(adoCommand_),
        paramPosHeading_(adoCommand_),
        paramCurrentHp_(adoCommand_),
        paramState_(adoCommand_),
        paramBuildTime_(adoCommand_),
        paramExpireTime_(adoCommand_),
        paramOwnerType_(adoCommand_),
        paramOwnerCharacterId_(adoCommand_),
        paramOwnerGuildId_(adoCommand_),
        paramCreateTime_(adoCommand_)
    {}
};


/**
 * @struct GetBuildings
 */
struct GetBuildingsCommand : RecordSetCommand
{
    IdParameter paramZoneMaxBuildingId_;
    IdParameter paramMinId_;
    IdParameter paramMaxId_;

    GetBuildingsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_buildings"),
        paramZoneMaxBuildingId_(adoCommand_, ADODB::adParamOutput),
        paramMinId_(adoCommand_),
        paramMaxId_(adoCommand_) {}
};


/**
 * @struct RemoveBuildingCommand
 */
struct RemoveBuildingCommand : Command
{
    IdParameter paramBuildingId_;
    
    RemoveBuildingCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_building"),
        paramBuildingId_(adoCommand_) {}
};


/**
 * @struct UpdateBuildingStateCommand
 */
struct UpdateBuildingStateCommand: Command
{
    IdParameter paramBuildingId_;
    sne::database::AdoUnsignedTinyIntParameter paramState_;
    sne::database::AdoBigIntParameter paramBuildTime_;
    sne::database::AdoBigIntParameter paramExpireTime_;
    sne::database::AdoUnsignedIntParameter paramCurrentHp_;

    UpdateBuildingStateCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_building_state"),
        paramBuildingId_(adoCommand_),
        paramState_(adoCommand_),
        paramBuildTime_(adoCommand_),
        paramExpireTime_(adoCommand_),
        paramCurrentHp_(adoCommand_) {}
};


/**
 * @struct UpdateBuildingOwnerCommand
 */
struct UpdateBuildingOwnerCommand: Command
{
    IdParameter paramBuildingId_;
    sne::database::AdoUnsignedTinyIntParameter paramOwnerType_;
    IdParameter paramPlayerId_;
    IdParameter paramGuildId_;

    UpdateBuildingOwnerCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_building_owner"),
        paramBuildingId_(adoCommand_),
        paramOwnerType_(adoCommand_),
        paramPlayerId_(adoCommand_),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct MoveBuildingItemCommand
 */
struct MoveBuildingItemCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;

    MoveBuildingItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.move_building_inventory_item"),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_) {}
};


/**
 * @struct SwitchBuildingItemCommand
 */
struct SwitchBuildingItemCommand : Command
{
    IdParameter paramItemId1_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    IdParameter paramItemId2_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId2_;

    SwitchBuildingItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.switch_building_inventory_item"),
        paramItemId1_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemId2_(adoCommand_),
        paramSlotId2_(adoCommand_) {}
};


/**
 * @struct AddBuildingItemCommand
 */
struct AddBuildingItemCommand : Command
{
    IdParameter paramBuildingId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;
	sne::database::AdoUnsignedTinyIntParameter paramInvenType_;
    sne::database::AdoBigIntParameter expireAt_;

    AddBuildingItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_building_inventory_item"),
        paramBuildingId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemCount_(adoCommand_),
		paramInvenType_(adoCommand_),
        expireAt_(adoCommand_) {}
};


/**
 * @struct AddBuildingEquipItemCommand
 */
struct AddBuildingEquipItemCommand : Command
{
    IdParameter paramBuildingId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId1_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;
    sne::database::AdoBigIntParameter expireAt_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoUnsignedIntParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoUnsignedIntParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoUnsignedIntParameter paramOptionValue3_;
    sne::database::AdoUnsignedTinyIntParameter paramSocketCount_;
	sne::database::AdoUnsignedTinyIntParameter paramInvenType_;

    AddBuildingEquipItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_building_inventory_equip_item"),
        paramBuildingId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId1_(adoCommand_),
        paramItemCount_(adoCommand_),
        expireAt_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_),
        paramSocketCount_(adoCommand_),
		paramInvenType_(adoCommand_) {}
};

/**
 * @struct AddBuildingEquipSocketOptionCommand
 */
struct AddBuildingEquipSocketOptionCommand : Command
{
    IdParameter paramBuildingId_;
    IdParameter paramItemId_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    CodeParameter paramGemCode_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;


    AddBuildingEquipSocketOptionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_building_equip_socket_option"),
        paramBuildingId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramGemCode_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_) {}
};


/**
 * @struct AddBuildingAccessoryItemCommand
 */
struct AddBuildingAccessoryItemCommand : Command
{
    IdParameter paramBuildingId_;
    IdParameter paramItemId_;
    CodeParameter paramItemCode_;
    sne::database::AdoUnsignedSmallIntParameter paramSlotId_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;
    sne::database::AdoBigIntParameter expireAt_;

    sne::database::AdoUnsignedTinyIntParameter paramOptionScript1_;
    sne::database::AdoIntegerParameter paramOptionValue1_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript2_;
    sne::database::AdoIntegerParameter paramOptionValue2_;
    sne::database::AdoUnsignedTinyIntParameter paramOptionScript3_;
    sne::database::AdoIntegerParameter paramOptionValue3_;
    sne::database::AdoUnsignedTinyIntParameter paramInvenType_;

    AddBuildingAccessoryItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_building_inventory_accessory_item"),
        paramBuildingId_(adoCommand_),
        paramItemId_(adoCommand_),
        paramItemCode_(adoCommand_),
        paramSlotId_(adoCommand_),
        paramItemCount_(adoCommand_),
        expireAt_(adoCommand_),
        paramOptionScript1_(adoCommand_),
        paramOptionValue1_(adoCommand_),
        paramOptionScript2_(adoCommand_),
        paramOptionValue2_(adoCommand_),
        paramOptionScript3_(adoCommand_),
        paramOptionValue3_(adoCommand_),
        paramInvenType_(adoCommand_){}
};

/**
 * @struct RemoveBuildingItemCommand
 */
struct RemoveBuildingItemCommand : Command
{
    IdParameter paramItemId_;
    
    RemoveBuildingItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_building_inventory_item"),
        paramItemId_(adoCommand_) {}
};


/**
 * @struct RemoveSelectRecipeProductionCommand
 */
struct AddSelectRecipeProductionCommand : Command
{
    IdParameter paramBuildingId_;
    CodeParameter paramCode_;
    sne::database::AdoUnsignedTinyIntParameter paramCount_;
    sne::database::AdoBigIntParameter paramCompleteTime_;

    AddSelectRecipeProductionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_select_recipe_production"),
        paramBuildingId_(adoCommand_),
        paramCode_(adoCommand_), 
        paramCount_(adoCommand_),
        paramCompleteTime_(adoCommand_)
        {}
};


/**
 * @struct RemoveSelectRecipeProductionCommand
 */
struct RemoveSelectRecipeProductionCommand : Command
{
    IdParameter paramBuildingId_;
    CodeParameter paramCode_;

    RemoveSelectRecipeProductionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_select_recipe_production"),
        paramBuildingId_(adoCommand_),
        paramCode_(adoCommand_) {}
};


/**
 * @struct UpdateSelectRecipeProductionCommand
 */
struct UpdateSelectRecipeProductionCommand : Command
{
    IdParameter paramBuildingId_;
    CodeParameter paramCode_;
    sne::database::AdoUnsignedTinyIntParameter paramCount_;
    sne::database::AdoBigIntParameter paramCompleteTime_;

    UpdateSelectRecipeProductionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_select_recipe_production"),
        paramBuildingId_(adoCommand_),
        paramCode_(adoCommand_), 
        paramCount_(adoCommand_),
        paramCompleteTime_(adoCommand_)
        {}
};



/**
 * @struct UpdateBuildingItemCountCommand
 */
struct UpdateBuildingItemCountCommand : Command
{
    IdParameter paramItemId_;
    sne::database::AdoUnsignedTinyIntParameter paramItemCount_;

    UpdateBuildingItemCountCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_building_inventory_item_count"),
        paramItemId_(adoCommand_),
        paramItemCount_(adoCommand_) {}
};


/**
 * @struct RemoveAllBuildingItemCommand
 */
struct RemoveAllBuildingItemCommand : Command
{
    IdParameter paramBuildingId_;
    sne::database::AdoUnsignedTinyIntParameter paramInvenType_;

    RemoveAllBuildingItemCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_all_building_inventory_item"),
        paramBuildingId_(adoCommand_),
        paramInvenType_(adoCommand_) {}
};


/**
 * @struct AddBuildingGuardCommand
 */
struct AddBuildingGuardCommand : Command
{
    IdParameter paramGuardId_;
    IdParameter paramBuildingId_;
    CodeParameter paramNpcCode_;
    sne::database::AdoFloat32Parameter paramPosX_;
    sne::database::AdoFloat32Parameter paramPosY_;
    sne::database::AdoFloat32Parameter paramPosZ_;
    sne::database::AdoUnsignedTinyIntParameter paramPosHeading_;
    sne::database::AdoBigIntParameter paramExpireTime_;

    
    AddBuildingGuardCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_building_guard"),
        paramGuardId_(adoCommand_),
        paramBuildingId_(adoCommand_),
        paramNpcCode_(adoCommand_),
        paramPosX_(adoCommand_),
        paramPosY_(adoCommand_),
        paramPosZ_(adoCommand_),
        paramPosHeading_(adoCommand_),
        paramExpireTime_(adoCommand_) {}
};


/**
 * @struct RemoveBuildingGuardCommand
 */
struct RemoveBuildingGuardCommand : Command
{
    IdParameter paramGuardId_;

    RemoveBuildingGuardCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_building_guard"),
        paramGuardId_(adoCommand_) {}
};


/**
 * @struct AddBindRecallInfoCommand
 */
struct AddBindRecallInfoCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    IdParameter paramLinkId_;
    CodeParameter paramDataCode_;
    CodeParameter paramWorldMapCode_;
    sne::database::AdoFloat32Parameter paramPosX_;
    sne::database::AdoFloat32Parameter paramPosY_;
    sne::database::AdoFloat32Parameter paramPosZ_;
    
    AddBindRecallInfoCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_bind_recall_info"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramLinkId_(adoCommand_),
        paramDataCode_(adoCommand_),
        paramWorldMapCode_(adoCommand_),
        paramPosX_(adoCommand_),
        paramPosY_(adoCommand_),
        paramPosZ_(adoCommand_)
    {
    }
};


/**
 * @struct RemoveBindRecallCommand
 */
struct RemoveBindRecallCommand : Command
{
    IdParameter paramLinkId_;
    
    RemoveBindRecallCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_bind_recall_info"),
        paramLinkId_(adoCommand_) {}
};


/**
 * @struct ReleaseBeginnerProtectionCommand
 */
struct ReleaseBeginnerProtectionCommand : Command
{
    IdParameter paramCharacterId_;
    
    ReleaseBeginnerProtectionCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.release_beginner_protection"),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct AddCooldownCommand
 */
struct AddCooldownCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    CodeParameter paramDataCode_;
    sne::database::AdoBigIntParameter paramExpireTime_;

    AddCooldownCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_cooldown"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramDataCode_(adoCommand_),
        paramExpireTime_(adoCommand_) {}
};

/**
 * @struct RemoveAllCooldownCommand
 */
struct RemoveAllCooldownCommand : Command
{
    IdParameter paramCharacterId_;

    RemoveAllCooldownCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_all_cooldown"),
        paramCharacterId_(adoCommand_){}
};


/**
 * @struct AddRemainEffectCommand
 */
struct AddRemainEffectCommand : Command
{
    IdParameter paramAccountId_;
    IdParameter paramCharacterId_;
    CodeParameter paramDataCode_;

    sne::database::AdoUnsignedTinyIntParameter paramUseTargetStatusType_;
    sne::database::AdoUnsignedTinyIntParameter paramIsCaster_;

    sne::database::AdoIntegerParameter paramEffectValue1_;
    sne::database::AdoBigIntParameter paramExpireTime1_;
    sne::database::AdoIntegerParameter paramEffectValue2_;
    sne::database::AdoBigIntParameter paramExpireTime2_;

    AddRemainEffectCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_remain_effect"),
        paramAccountId_(adoCommand_),
        paramCharacterId_(adoCommand_),
        paramDataCode_(adoCommand_),
        paramUseTargetStatusType_(adoCommand_),
        paramIsCaster_(adoCommand_),
        paramEffectValue1_(adoCommand_),
        paramExpireTime1_(adoCommand_),
        paramEffectValue2_(adoCommand_),
        paramExpireTime2_(adoCommand_) {}
};


/**
 * @struct RemoveAllRemainEffectCommand
 */
struct RemoveAllRemainEffectCommand : Command
{
    IdParameter paramCharacterId_;

    RemoveAllRemainEffectCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_all_remain_effect"),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct UpdateCharacterInventoryInfoCommand
 */
struct UpdateCharacterInventoryInfoCommand : Command
{
    IdParameter paramCharacterId_;
    sne::database::AdoUnsignedTinyIntParameter paramInvenType_;
    sne::database::AdoUnsignedTinyIntParameter paramIsCashSlot_;
    sne::database::AdoUnsignedTinyIntParameter paramCount_;

    UpdateCharacterInventoryInfoCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_character_inventory_info"),
        paramCharacterId_(adoCommand_),
        paramInvenType_(adoCommand_),
        paramIsCashSlot_(adoCommand_),
        paramCount_(adoCommand_) {}
};


/**
 * @struct CreateVehicleCommand
 */
struct CreateVehicleCommand : Command
{
    IdParameter paramCharacterId_;
	IdParameter paramAccountId_;
    IdParameter paramId_;
    CodeParameter paramCode_;
	sne::database::AdoBigIntParameter paramBirthday_;
    sne::database::AdoUnsignedTinyIntParameter paramPeakAge_;

    CreateVehicleCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.create_vehicle_info"),
		paramCharacterId_(adoCommand_),
		paramAccountId_(adoCommand_),
		paramId_(adoCommand_),
		paramCode_(adoCommand_),
		paramBirthday_(adoCommand_),
		paramPeakAge_(adoCommand_) {}
};


/**
 * @struct CreateGliderCommand
 */
struct CreateGliderCommand : Command
{
    IdParameter paramCharacterId_;
	IdParameter paramAccountId_;
    IdParameter paramId_;
    CodeParameter paramCode_;
	sne::database::AdoUnsignedIntParameter paramDurability_;
    CreateGliderCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.create_glider_info"),
		paramCharacterId_(adoCommand_),
		paramAccountId_(adoCommand_),
		paramId_(adoCommand_),
		paramCode_(adoCommand_),
		paramDurability_(adoCommand_) {}
};


/**
 * @struct DeleteVehicleCommand
 */
struct DeleteVehicleCommand : Command
{
    IdParameter paramId_;

    DeleteVehicleCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.delete_vehicle"),
		paramId_(adoCommand_) {}
};


/**
 * @struct DeleteGliderCommand
 */
struct DeleteGliderCommand : Command
{
    IdParameter paramId_;

    DeleteGliderCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.delete_glider"),
		paramId_(adoCommand_) {}
};


/**
 * @struct SelectVehicleCommand
 */
struct SelectVehicleCommand : Command
{
	IdParameter paramCharacterId_;
    IdParameter paramVehicleId_;

    SelectVehicleCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.select_vehicle"),
		paramCharacterId_(adoCommand_),
		paramVehicleId_(adoCommand_){}
};


/**
 * @struct SelectGliderCommand
 */
struct SelectGliderCommand : Command
{
	IdParameter paramCharacterId_;
    IdParameter paramGliderId_;

    SelectGliderCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.select_glider"),
		paramCharacterId_(adoCommand_),
		paramGliderId_(adoCommand_){}
};



/**
 * @struct UpdateGliderDurabilityCommand
 */
struct UpdateGliderDurabilityCommand : Command
{
	IdParameter paramId_;
    sne::database::AdoUnsignedIntParameter paramDurability_;

    UpdateGliderDurabilityCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_glider_durability"),
		paramId_(adoCommand_),
		paramDurability_(adoCommand_){}
};


/**
 * @struct GetBuddiesCommand
 */
struct GetBuddiesCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;
    
    GetBuddiesCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_buddies"),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct AddBuddyCommand
 */
struct AddBuddyCommand : Command
{
    IdParameter paramCharacterId1_;
	IdParameter paramCharacterId2_;

    AddBuddyCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_buddy"),
		paramCharacterId1_(adoCommand_),
		paramCharacterId2_(adoCommand_) {}
};


/**
 * @struct RemoveBuddyCommand
 */
struct RemoveBuddyCommand : Command
{
    IdParameter paramCharacterId1_;
	IdParameter paramCharacterId2_;

    RemoveBuddyCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_buddy"),
		paramCharacterId1_(adoCommand_),
		paramCharacterId2_(adoCommand_) {}
};


/**
 * @struct AddBlockCommand
 */
struct AddBlockCommand : Command
{
    IdParameter paramOwnerId_;
	IdParameter paramCharacterId_;

    AddBlockCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_block"),
		paramOwnerId_(adoCommand_),
		paramCharacterId_(adoCommand_) {}
};


/**
 * @struct RemoveBlockCommand
 */
struct RemoveBlockCommand : Command
{
    IdParameter paramOwnerId_;
    IdParameter paramCharacterId_;

    RemoveBlockCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_block"),
            paramOwnerId_(adoCommand_),
            paramCharacterId_(adoCommand_) {}
};


/**
 * @struct AddGuildRankCommand
 */
struct AddGuildRankCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoTinyIntParameter paramRankId_;
    sne::database::AdoVarWCharParameter paramName_;
    sne::database::AdoIntegerParameter paramRights_;
    sne::database::AdoIntegerParameter paramGoldWithdrawalPerDay_;

    AddGuildRankCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.create_guild_rank"),
            paramGuildId_(adoCommand_),
            paramRankId_(adoCommand_),
            paramName_(adoCommand_, maxGuildNameLength),
            paramRights_(adoCommand_),
            paramGoldWithdrawalPerDay_(adoCommand_) {}
};


/**
 * @struct AddGuildBankRightsCommand
 */
struct AddGuildBankRightsCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoTinyIntParameter paramRankId_;
    sne::database::AdoTinyIntParameter paramVaultId_;
    sne::database::AdoIntegerParameter paramRights_;
    sne::database::AdoIntegerParameter paramGoldWithdrawalPerDay_;

    AddGuildBankRightsCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.create_guild_bank_right"),
            paramGuildId_(adoCommand_),
            paramRankId_(adoCommand_),
            paramVaultId_(adoCommand_),
            paramRights_(adoCommand_),
            paramGoldWithdrawalPerDay_(adoCommand_) {}
};

/**
 * @struct DeleteGuildRankCommand
 */
struct DeleteGuildRankCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoTinyIntParameter paramRankId_;

    DeleteGuildRankCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.delete_guild_rank"),
        paramGuildId_(adoCommand_),
        paramRankId_(adoCommand_) {}
};


/**
 * @struct SwapGuildRankCommand
 */
struct SwapGuildRankCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoTinyIntParameter paramRankId1_;
    sne::database::AdoTinyIntParameter paramRankId2_;

    SwapGuildRankCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.swap_guild_rank"),
        paramGuildId_(adoCommand_),
        paramRankId1_(adoCommand_),
        paramRankId2_(adoCommand_) {}
};


/**
 * @struct UpdateGuildRankNameCommand
 */
struct UpdateGuildRankNameCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoTinyIntParameter paramRankId_;
    sne::database::AdoVarWCharParameter paramRankName_;

    UpdateGuildRankNameCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_rank_name"),
            paramGuildId_(adoCommand_),
            paramRankId_(adoCommand_),
            paramRankName_(adoCommand_, maxGuildRankNameLength) {}
};


/**
 * @struct UpdateGuildRankRightsCommand
 */
struct UpdateGuildRankRightsCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoTinyIntParameter paramRankId_;
    sne::database::AdoIntegerParameter paramRights_;
    sne::database::AdoIntegerParameter paramGoldWithdrawalPerDay_;

    UpdateGuildRankRightsCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_rank_rights"),
            paramGuildId_(adoCommand_),
            paramRankId_(adoCommand_),
            paramRights_(adoCommand_),
            paramGoldWithdrawalPerDay_(adoCommand_) {}
};


/**
 * @struct UpdateGuildBankRightsCommand
 */
struct UpdateGuildBankRightsCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoTinyIntParameter paramRankId_;
    sne::database::AdoTinyIntParameter paramVaultId_;
    sne::database::AdoIntegerParameter paramRights_;
    sne::database::AdoIntegerParameter paramGoldWithdrawalPerDay_;

    UpdateGuildBankRightsCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_bank_rights"),
            paramGuildId_(adoCommand_),
            paramRankId_(adoCommand_),
            paramVaultId_(adoCommand_),
            paramRights_(adoCommand_),
            paramGoldWithdrawalPerDay_(adoCommand_) {}
};



/**
 * @struct UpdateGuildRankCommand
 */
struct UpdateGuildRankCommand : Command
{
    IdParameter paramPlayerId_;
    sne::database::AdoTinyIntParameter paramRankId_;

    UpdateGuildRankCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_member_rank"),
        paramPlayerId_(adoCommand_),
        paramRankId_(adoCommand_) {}
};



/**
 * @struct UpdateGuildExpCommand
 */
struct UpdateGuildExpCommand : Command
{
    IdParameter paramGuildId_;
    sne::database::AdoIntegerParameter paramExp_;
    sne::database::AdoIntegerParameter paramDayAddExp_;
    sne::database::AdoIntegerParameter paramSp_;
    sne::database::AdoBigIntParameter paramLastUpdateTime_;

    UpdateGuildExpCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_guild_exp"),
        paramGuildId_(adoCommand_),
        paramExp_(adoCommand_),
        paramDayAddExp_(adoCommand_),
        paramSp_(adoCommand_),
        paramLastUpdateTime_(adoCommand_) {}
};


/**
 * @struct AddGuildSkillCommand
 */
struct AddGuildSkillCommand : Command
{
    IdParameter paramGuildId_;
    CodeParameter paramSkillCode_;

    AddGuildSkillCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_guild_skill"),
        paramGuildId_(adoCommand_),
        paramSkillCode_(adoCommand_) {}
};


/**
 * @struct RemoveGuildSkillCommand
 */
struct RemoveGuildSkillCommand : Command
{
    IdParameter paramGuildId_;
    CodeParameter paramSkillCode_;

    RemoveGuildSkillCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_guild_skill"),
        paramGuildId_(adoCommand_),
        paramSkillCode_(adoCommand_) {}
};


/**
 * @struct RemoveAllGuildSkillsCommand
 */
struct RemoveAllGuildSkillsCommand : Command
{
    IdParameter paramGuildId_;

    RemoveAllGuildSkillsCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_all_guild_skills"),
        paramGuildId_(adoCommand_) {}
};


/**
 * @struct GetCharacterTitlesCommand
 */
struct GetCharacterTitlesCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;
    
    GetCharacterTitlesCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_character_titles"),
        paramCharacterId_(adoCommand_) {}
};

/**
 * @struct GetCompleteAchievementsCommand
 */
struct GetCompleteAchievementsCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;
    
    GetCompleteAchievementsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_complete_achievements"),
        paramCharacterId_(adoCommand_) {}
};

/**
 * @struct GetProcessAchievementsCommand
 */
struct GetProcessAchievementsCommand : RecordSetCommand
{
    IdParameter paramCharacterId_;
    
    GetProcessAchievementsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_process_achievements"),
        paramCharacterId_(adoCommand_) {}
};


/**
 * @struct AddCharacterTitleCommand
 */
struct AddCharacterTitleCommand : Command
{
    IdParameter paramPlayerId_;
    CodeParameter paramTitleCode_;

    AddCharacterTitleCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_character_title"),
        paramPlayerId_(adoCommand_),
        paramTitleCode_(adoCommand_) {}
};


/**
 * @struct AddProcessAchievementCommand
 */
struct AddProcessAchievementCommand : Command
{
    IdParameter paramPlayerId_;
    CodeParameter paramAchievementCode_;
    sne::database::AdoBigIntParameter paramParam1_;
    sne::database::AdoBigIntParameter paramParam2_;
    sne::database::AdoBigIntParameter paramLastUpdateTime_;

    AddProcessAchievementCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_process_achievement"),
        paramPlayerId_(adoCommand_),
        paramAchievementCode_(adoCommand_),
        paramParam1_(adoCommand_),
        paramParam2_(adoCommand_),
        paramLastUpdateTime_(adoCommand_) {}
};

/**
 * @struct UpdateProcessAchievementCommand
 */
struct UpdateProcessAchievementCommand : Command
{
    IdParameter paramPlayerId_;
    CodeParameter paramAchievementCode_;
    sne::database::AdoBigIntParameter paramParam1_;
    sne::database::AdoBigIntParameter paramParam2_;
    sne::database::AdoBigIntParameter paramLastUpdateTime_;

    UpdateProcessAchievementCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_process_achievement"),
        paramPlayerId_(adoCommand_),
        paramAchievementCode_(adoCommand_),
        paramParam1_(adoCommand_),
        paramParam2_(adoCommand_),
        paramLastUpdateTime_(adoCommand_) {}
};


/**
 * @struct CompleteAchievementCommand
 */
struct CompleteAchievementCommand : Command
{
    IdParameter paramPlayerId_;
    CodeParameter paramAchievementCode_;
    sne::database::AdoIntegerParameter paramAchievementPoint_;
    sne::database::AdoBigIntParameter paramCompleteTime_;

    CompleteAchievementCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.complete_achievement"),
        paramPlayerId_(adoCommand_),
        paramAchievementCode_(adoCommand_),
        paramAchievementPoint_(adoCommand_),
        paramCompleteTime_(adoCommand_) {}
};



}} // namespace gideon { namespace serverbase {
