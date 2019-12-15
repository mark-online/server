#include "ServerBasePCH.h"
#include "SqlServerAccountDatabase.h"
#include <sne/database/ado/AdoDatabaseUtils.h>
#include <sne/server/config/ConfigReader.h>
#include <sne/server/utility/Profiler.h>
#include <sne/security/hash/AuthlogicSha512Hash.h>
#include <sne/base/utility/Assert.h>

namespace gideon { namespace serverbase {

SqlServerAccountDatabase::SqlServerAccountDatabase() :
    getPropertiesCommand_(getAdoDatabase()),
    getServerSpecCommand_(getAdoDatabase()),
    getShardsCommand_(getAdoDatabase()),
    getPasswordCommand_(getAdoDatabase()),
    getAccountInfoCommand_(getAdoDatabase()),
    getCharacterCountsCommand_(getAdoDatabase()),
    updateLoginAtCommand_(getAdoDatabase()),
    updateCharacterCountCommand_(getAdoDatabase()),
    logLogoutCommand_(getAdoDatabase()),
    getBansCommand_(getAdoDatabase()),
    addBanCommand_(getAdoDatabase()),
    removeBanCommand_(getAdoDatabase())
{
}


bool SqlServerAccountDatabase::open(sne::server::ConfigReader& configReader)
{
    const std::string connectionString =
        configReader.getString("database", "connection-string");
    const uint8_t connectionTimeout = static_cast<uint8_t>(
        configReader.getNumeric<size_t>("database", "connection-timeout"));
    if (! connection_.open(connectionString, connectionTimeout)) {
        SNE_LOG_ERROR(__FUNCTION__ " Failed(%s, %d)",
            connectionString.c_str(), connectionTimeout);
        return false;
    }
    return true;
}


void SqlServerAccountDatabase::close()
{
    connection_.close();
}



void SqlServerAccountDatabase::beginTransaction()
{
    try {
        getAdoDatabase().execute("BEGIN TRANSACTION");
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
    }
}


void SqlServerAccountDatabase::commitTransaction()
{
    try {
        getAdoDatabase().execute("COMMIT");
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
    }
}


void SqlServerAccountDatabase::rollbackTransaction()
{
    try {
        getAdoDatabase().execute("ROLLBACK");
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
    }
}


bool SqlServerAccountDatabase::getProperties(sne::server::Properties& properties)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getPropertiesCommand_.execute();

        std::string key;
        std::string value;
        sne::database::AdoRecordSet& rs = getPropertiesCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            rs.getFieldValue(0, key);
            rs.getFieldValue(1, value);

            properties.emplace(key, value);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::getServerSpec(sne::server::ServerSpec& spec,
    const std::string& serverName, const std::string& suffix)
{
    sne::server::Profiler profiler(__FUNCTION__);

    std::string name = serverName;
    if (! suffix.empty()) {
        name += "." + suffix;
    }

    try {
        getServerSpecCommand_.paramName_.setValue(name);

        getServerSpecCommand_.execute();

        std::string address;
        getServerSpecCommand_.paramListeningAddress_.getValue(address);
        spec.listeningAddresses_.push_back(address);
        getServerSpecCommand_.paramListeningPort_.getValue(
            spec.listeningPort_);
        getServerSpecCommand_.paramMonitorPort_.getValue(
            spec.monitorPort_);
        getServerSpecCommand_.paramWorkerThreadCount_.getValue(
            spec.workerThreadCount_);
        getServerSpecCommand_.paramSessionPoolSize_.getValue(
            spec.sessionPoolSize_);
        getServerSpecCommand_.paramMaxUserCount_.getValue(
            spec.maxUserCount_);
        getServerSpecCommand_.paramPacketCipher_.getValue(
            spec.packetCipher_);
        getServerSpecCommand_.paramCipherKeyTimeLimit_.getValue(
            spec.cipherKeyTimeLimit_);
        getServerSpecCommand_.paramAuthenticationTimeout_.getValue(
            spec.authenticationTimeout_);
        getServerSpecCommand_.paramHeartbeatTimeout_.getValue(
            spec.heartbeatTimeout_);
        getServerSpecCommand_.paramMaxBytesPerSecond_.getValue(
            spec.sessionCapacity_.maxBytesPerSecond_);
        getServerSpecCommand_.paramSecondsForThrottling_.getValue(
            spec.sessionCapacity_.secondsForThrottling_);
        getServerSpecCommand_.paramMaxPendablePacketCount_.getValue(
            spec.sessionCapacity_.maxPendablePacketCount_);
        getServerSpecCommand_.paramCertificate_.getValue(
            spec.certificate_);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    spec.name_ = name;
    return true;
}


bool SqlServerAccountDatabase::getShardInfos(FullShardInfoMap& shardMap)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getShardsCommand_.execute();

        FullShardInfo shardInfo;
        sne::database::AdoRecordSet& rs = getShardsCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            ShardId shardId = invalidShardId;

            rs.getFieldValue(0, shardId);
            rs.getFieldValue(1, shardInfo.name_.ref());

            shardInfo.shardId_ = shardId;
            shardMap.emplace(shardId, shardInfo);
        }

        rs.nextRecordset();
        ZoneServerInfo zoneInfo;

        for (; ! rs.isEof(); rs.moveNext()) {
            ShardId shardId = invalidShardId;

            rs.getFieldValue(0, shardId);
            const FullShardInfoMap::iterator pos = shardMap.find(shardId);
            if (pos == shardMap.end()) {
                assert(false && "Invalid Info");
                return false;
            }

            FullShardInfo& info = (*pos).second;
            ZoneId zoneId = invalidZoneId;
            rs.getFieldValue(1, zoneId);
            rs.getFieldValue(2, zoneInfo.mapCode_);
            rs.getFieldValue(3, zoneInfo.isFirstZone_);
            rs.getFieldValue(4, zoneInfo.isEnabled_);
            rs.getFieldValue(5, zoneInfo.zoneServerIp_);
            rs.getFieldValue(6, zoneInfo.zoneServerPort_);
            info.zoneServerInfoMap_.emplace(zoneId, zoneInfo);
        }

    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::authenticate(AccountId& accountId,
    const UserId& userId, const UserPassword& password)
{
    sne::server::Profiler profiler(__FUNCTION__);

    accountId = invalidAccountId;

    std::string encryptedPassword;
    std::string passwordSalt;

    try {
        getPasswordCommand_.paramUserId_.setValue(userId);

        getPasswordCommand_.execute();

        getPasswordCommand_.paramAccountId_.getValue(accountId);
        getPasswordCommand_.paramEncryptedPassword_.getValue(encryptedPassword);
        getPasswordCommand_.paramPasswordSalt_.getValue(passwordSalt);
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    sne::security::AuthlogicSha512Hash hasher;
    return hasher.digest(password, passwordSalt) == encryptedPassword;
}


bool SqlServerAccountDatabase::getAccountInfo(AccountInfo& accountInfo, AccountId accountId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    accountInfo.accountId_ = accountId;

    try {
        getAccountInfoCommand_.paramAccountId_.setValue(accountId);

        getAccountInfoCommand_.execute();
        getAccountInfoCommand_.paramUserId_.getValue(accountInfo.userId_);
        getAccountInfoCommand_.paramGrade_.getValue(accountInfo.accountGrade_);
        getAccountInfoCommand_.paramShardId_.getValue(accountInfo.lastAccessShardId_);
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }
    return true;
}


bool SqlServerAccountDatabase::getCharacterCounts(CharacterCountPerShardMap& characterCounts,
    AccountId accountId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getCharacterCountsCommand_.paramAccountId_.setValue(accountId);

        getCharacterCountsCommand_.execute();

        sne::database::AdoRecordSet& rs = getCharacterCountsCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            ShardId shardId = invalidShardId;
            uint8_t counts = 0;

            rs.getFieldValue(0, shardId);
            rs.getFieldValue(1, counts);

            characterCounts.emplace(shardId, counts);
        }
    }
    catch (const sne::database::AdoException& e) {
        sne::database::logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::updateLoginAt(AccountId accountId)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateLoginAtCommand_.paramAccountId_.setValue(accountId);

        updateLoginAtCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::updateCharacterCount(AccountId accountId, ShardId shardId,
    uint8_t characterCount)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        updateCharacterCountCommand_.paramAccountId_.setValue(accountId);
        updateCharacterCountCommand_.paramShardId_.setValue(shardId);
        updateCharacterCountCommand_.paramCharacterCount_.setValue(characterCount);

        updateCharacterCountCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::logLogout(AccountId accountId,
    ShardId shardId, uint32_t sessionSeconds)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        logLogoutCommand_.paramAccountId_.setValue(accountId);
        logLogoutCommand_.paramShardId_.setValue(shardId);
        logLogoutCommand_.paramSessionSeconds_.setValue(sessionSeconds);

        logLogoutCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::getBans(BanInfos& accountBanInfos, BanInfos& characterBanInfos, BanInfos& ipBanInfos)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        getBansCommand_.execute();

        sne::database::AdoRecordSet& rs = getBansCommand_.adoRecordSet_;
        for (; ! rs.isEof(); rs.moveNext()) {
            BanInfo banInfo;
            rs.getFieldValue(0, banInfo.banMode_);
            rs.getFieldValue(1, banInfo.accountId_);
            rs.getFieldValue(2, banInfo.characterId_);
            rs.getFieldValue(3, banInfo.ip_);
            rs.getFieldValue(4, banInfo.banExpireTime_);

            if (banInfo.banMode_ == bmAccount) {
                accountBanInfos.push_back(banInfo);
            }
            else if (banInfo.banMode_ == bmCharacter) {
                characterBanInfos.push_back(banInfo);
            }
            else if (banInfo.banMode_ == bmIpAddress) {
                ipBanInfos.push_back(banInfo);
            }
        }
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::addBan(const BanInfo& banInfo)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        addBanCommand_.paramBanMode_.setValue(banInfo.banMode_);
        addBanCommand_.paramAccountId_.setValue(banInfo.accountId_);
        addBanCommand_.paramCharacterId_.setValue(banInfo.characterId_);
        addBanCommand_.paramIpAddress_.setValue(banInfo.ip_);
        addBanCommand_.paramBeginAt_.setValue(getTime());
        addBanCommand_.paramEndAt_.setValue(banInfo.banExpireTime_);
        addBanCommand_.paramReason_.setValue(banInfo.reason_);
        addBanCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}


bool SqlServerAccountDatabase::removeBan(BanMode banMode, AccountId accountId, ObjectId characterId, const std::string& ipAddress)
{
    sne::server::Profiler profiler(__FUNCTION__);

    try {
        removeBanCommand_.paramBanMode_.setValue(banMode);
        removeBanCommand_.paramAccountId_.setValue(accountId);
        removeBanCommand_.paramCharacterId_.setValue(characterId);
        removeBanCommand_.paramIpAddress_.setValue(ipAddress);

        removeBanCommand_.execute();
    }
    catch (const sne::database::AdoException& e) {
        logDatabaseError(getAdoDatabase(), e);
        return false;
    }

    return true;
}

}} // namespace gideon { namespace serverbase {
