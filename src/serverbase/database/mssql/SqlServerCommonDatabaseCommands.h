#pragma once

#include <gideon/cs/shared/data/ServerType.h>
#include <sne/server/common/ServerSpec.h>
#include <sne/database/ado/AdoConnection.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace serverbase {

typedef sne::database::AdoBigIntParameter IdParameter;
typedef sne::database::AdoUnsignedIntParameter CodeParameter;

/**
 * @struct Command
 */
struct Command
{
    sne::database::AdoCommand adoCommand_;

    Command(sne::database::AdoDatabase& database, const std::string& cmd) :
        adoCommand_(database, cmd) {}

    void execute() {
        adoCommand_.executeNoRecords();
    }
};


/**
 * @struct Command
 */
struct RecordSetCommand : Command
{
    sne::database::AdoRecordSet adoRecordSet_;

    RecordSetCommand(sne::database::AdoDatabase& database, const std::string& cmd) :
        Command(database, cmd),
        adoRecordSet_(database) {}

    void execute() {
        adoRecordSet_.execute(adoCommand_);
    }
};


/**
 * @struct GetPropertiesCommand
 */
struct GetPropertiesCommand : RecordSetCommand
{
    GetPropertiesCommand(sne::database::AdoDatabase& database) :
        RecordSetCommand(database, "dbo.get_properties") {}
};


/**
 * @struct GetServerSpecCommand
 */
struct GetServerSpecCommand : Command
{
    sne::database::AdoVarCharParameter paramListeningAddress_;
    sne::database::AdoIntegerParameter paramListeningPort_;
    sne::database::AdoIntegerParameter paramMonitorPort_;
    sne::database::AdoIntegerParameter paramWorkerThreadCount_;
    sne::database::AdoIntegerParameter paramSessionPoolSize_;
    sne::database::AdoIntegerParameter paramMaxUserCount_;
    sne::database::AdoVarCharParameter paramPacketCipher_;
    sne::database::AdoIntegerParameter paramCipherKeyTimeLimit_;
    sne::database::AdoIntegerParameter paramAuthenticationTimeout_;
    sne::database::AdoIntegerParameter paramHeartbeatTimeout_;
    sne::database::AdoIntegerParameter paramMaxBytesPerSecond_;
    sne::database::AdoIntegerParameter paramSecondsForThrottling_;
    sne::database::AdoIntegerParameter paramMaxPendablePacketCount_;
    sne::database::AdoVarCharParameter paramCertificate_;
    sne::database::AdoVarCharParameter paramName_;

    GetServerSpecCommand(sne::database::AdoDatabase& database) :
        Command(database, "dbo.get_server_spec"),
        paramListeningAddress_(adoCommand_, sne::server::maxListeningAddressLength,
            ADODB::adParamOutput),
        paramListeningPort_(adoCommand_, ADODB::adParamOutput),
        paramMonitorPort_(adoCommand_, ADODB::adParamOutput),
        paramWorkerThreadCount_(adoCommand_, ADODB::adParamOutput),
        paramSessionPoolSize_(adoCommand_, ADODB::adParamOutput),
        paramMaxUserCount_(adoCommand_, ADODB::adParamOutput),
        paramPacketCipher_(adoCommand_, 10, ADODB::adParamOutput),
        paramCipherKeyTimeLimit_(adoCommand_, ADODB::adParamOutput),
        paramAuthenticationTimeout_(adoCommand_, ADODB::adParamOutput),
        paramHeartbeatTimeout_(adoCommand_, ADODB::adParamOutput),
        paramMaxBytesPerSecond_(adoCommand_, ADODB::adParamOutput),
        paramSecondsForThrottling_(adoCommand_, ADODB::adParamOutput),
        paramMaxPendablePacketCount_(adoCommand_, ADODB::adParamOutput),
        paramCertificate_(adoCommand_, 10, ADODB::adParamOutput),
        paramName_(adoCommand_, maxServerNameLength) {}
};

}} // namespace gideon { namespace serverbase {
