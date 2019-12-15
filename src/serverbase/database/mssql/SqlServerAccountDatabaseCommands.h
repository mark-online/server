#pragma once

#include "SqlServerCommonDatabaseCommands.h"

namespace gideon { namespace serverbase {

typedef sne::database::AdoBigIntParameter IdParameter;
typedef sne::database::AdoUnsignedIntParameter CodeParameter;


/**
 * @struct GetZoneGroupCommand
 */
struct GetShardsCommand : RecordSetCommand
{
    GetShardsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_shards") {}
};


/**
 * @struct GetPasswordCommand
 */
struct GetPasswordCommand : Command
{
    IdParameter paramAccountId_;
    sne::database::AdoVarCharParameter paramEncryptedPassword_;
    sne::database::AdoVarCharParameter paramPasswordSalt_;
    sne::database::AdoVarWCharParameter paramUserId_;

    GetPasswordCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_password"),
        paramAccountId_(adoCommand_, ADODB::adParamOutput),
        paramEncryptedPassword_(adoCommand_, 128, ADODB::adParamOutput),
        paramPasswordSalt_(adoCommand_, 20, ADODB::adParamOutput),
        paramUserId_(adoCommand_, maxUserIdLength) {}
};


/**
 * @struct GetAccountInfoCommand
 */
struct GetAccountInfoCommand : Command
{
	sne::database::AdoVarWCharParameter paramUserId_;
	sne::database::AdoUnsignedTinyIntParameter paramGrade_;
    sne::database::AdoIntegerParameter paramShardId_;
    IdParameter paramAccountId_;

    GetAccountInfoCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_account_info"),
		paramUserId_(adoCommand_, maxUserIdLength, ADODB::adParamOutput),
		paramGrade_(adoCommand_, ADODB::adParamOutput),
        paramShardId_(adoCommand_, ADODB::adParamOutput),
        paramAccountId_(adoCommand_) {}
};


/**
* @struct GetCharacterCountsCommand
 */
struct GetCharacterCountsCommand : RecordSetCommand
{
    IdParameter paramAccountId_;

    GetCharacterCountsCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_character_counts"),
        paramAccountId_(adoCommand_) {}
};


/**
 * @struct UpdateLoginAtCommand
 */
struct UpdateLoginAtCommand : Command
{
    IdParameter paramAccountId_;

    UpdateLoginAtCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_login_at"),
        paramAccountId_(adoCommand_) {}
};


/**
 * @struct UpdateCharacterCountCommand
 */
struct UpdateCharacterCountCommand : Command
{
    IdParameter paramAccountId_;
	sne::database::AdoIntegerParameter paramShardId_;
	sne::database::AdoSmallIntParameter paramCharacterCount_;

    UpdateCharacterCountCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.update_character_count"),
        paramAccountId_(adoCommand_),
		paramShardId_(adoCommand_),
        paramCharacterCount_(adoCommand_) {}
};


/**
 * @struct LogLogoutCommand
 */
struct LogLogoutCommand : Command
{
    IdParameter paramAccountId_;
	IdParameter paramShardId_;
    sne::database::AdoIntegerParameter paramSessionSeconds_;

    LogLogoutCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.insert_logout_log"),
        paramAccountId_(adoCommand_),
		paramShardId_(adoCommand_),
        paramSessionSeconds_(adoCommand_) {}
};

/**
 * @struct GetBansCommand
 */
struct GetBansCommand : RecordSetCommand
{
    GetBansCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_bans") {}
};


/**
 * @struct AddBanCommand
 */
struct AddBanCommand : Command
{
	sne::database::AdoUnsignedTinyIntParameter paramBanMode_;
    IdParameter paramAccountId_;
	IdParameter paramCharacterId_;
    sne::database::AdoVarCharParameter paramIpAddress_;
    sne::database::AdoDateParameter paramBeginAt_;
	sne::database::AdoDateParameter paramEndAt_;
	sne::database::AdoVarWCharParameter paramReason_;

    AddBanCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.add_ban"),
		paramBanMode_(adoCommand_),
        paramAccountId_(adoCommand_),
		paramCharacterId_(adoCommand_),
        paramIpAddress_(adoCommand_, 30),
		paramBeginAt_(adoCommand_),
		paramEndAt_(adoCommand_),
        paramReason_(adoCommand_, 128) {}
};


/**
 * @struct RemoveBanCommand
 */
struct RemoveBanCommand : Command
{
	sne::database::AdoUnsignedTinyIntParameter paramBanMode_;
    IdParameter paramAccountId_;
	IdParameter paramCharacterId_;
    sne::database::AdoVarCharParameter paramIpAddress_;
    
    RemoveBanCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.remove_ban"),
		paramBanMode_(adoCommand_),
        paramAccountId_(adoCommand_),
		paramCharacterId_(adoCommand_),
        paramIpAddress_(adoCommand_, 30) {}
};

}} // namespace gideon { namespace serverbase {
