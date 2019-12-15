#pragma once

#include <gideon/Common.h>
#include <sne/server/app/SessionServerApp.h>
#include <sne/server/common/ServerSpec.h>

namespace sne { namespace database {
class Database;
class DatabaseManager;
}} // namespace sne { namespace database {

namespace gideon { namespace serverbase {

/**
 * @class BaseSessionServerApp
 *
 * Gideon 전용 세션 서버 베이스 클래스
 */
class GIDEON_SERVER_API BaseSessionServerApp :
    public sne::server::SessionServerApp
{
public:
    BaseSessionServerApp();
    virtual ~BaseSessionServerApp();

protected:
    virtual const sne::server::ServerSpec& getServerSpec() const {
        return c2sSpec_;
    }

    // = sne::server::SessionServerApp overriding
    virtual bool onInit();

protected:
    //const sne::server::ServerSpec& getS2sServerSpec() const {
    //    return s2sSpec_;
    //}

    sne::server::ServerSpec getServerSpec(const std::string& serverName,
        const std::string& suffix) const;

private:
    bool initDatabaseManager();
    bool loadServerSpec();
    bool loadProperties();

private:
    std::unique_ptr<sne::database::DatabaseManager> databaseManager_;

    sne::server::ServerSpec c2sSpec_;
    //sne::server::ServerSpec s2sSpec_;
};

}} // namespace gideon { namespace serverbase {
