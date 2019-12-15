#include "LoginServerPCH.h"
#include "LoginServerApp.h"
#include "LoginService.h"
#include "s2s/LoginServerSideProxyFactory.h"
#include "c2s/LoginClientSessionFactory.h"
#include "user/LoginUserManager.h"
#include <sne/server/session/ServerSideSessionManager.h>
#include <gideon/serverbase/database/AccountDatabase.h>
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include <gideon/cs/shared/data/ServerInfo.h>
#include <sne/server/app/Factory.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/database/proxy/server/BaseDatabaseProxyServerApp.h>
#include <sne/server/session/SessionFactoryHelper.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/io_context/IoContextTask.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace loginserver {

namespace
{

/**
 * @class SessionFactoryHelperForServers
 * S2S 세션 팩토리 도우미
 */
class SessionFactoryHelperForServers : public sne::server::SessionFactoryHelper
{
public:
    std::unique_ptr<sne::base::SessionFactory> createSessionFactory(
        const sne::server::ServerSpec& serverSpec,
        sne::base::IoContextTask& ioContextTask) override {
        return std::make_unique<LoginServerSideProxyFactory>(serverSpec,
            ioContextTask);
    }
};


/**
 * @class GideonLoginService
 */
class GideonLoginService : public LoginService
{
public:
    GideonLoginService(const CommunityServerInfo& communityServerInfo) :
        LoginService(communityServerInfo),
        ioService_(nullptr),
        c2sSpec_(nullptr),
        s2sSpec_(nullptr) {}

    bool initialize(const sne::server::ServerSpec& c2sSpec,
        const sne::server::ServerSpec& s2sSpec,
        sne::base::IoContextTask& ioContextTask) {
        c2sSpec_ = &c2sSpec;
        s2sSpec_ = &s2sSpec;
        ioService_ = &ioContextTask;

        if (! LoginService::initialize()) {
            return false;
        }

        return true;
    }

private:
    const sne::server::S2sCertificate& getS2sCertificate() const override {
        return s2sSpec_->certificate_;
    }

    size_t getMaxUserCount() const override {
        return c2sSpec_->maxUserCount_;
    }

private:
    sne::base::IoContextTask* ioService_;
    const sne::server::ServerSpec* c2sSpec_;
    const sne::server::ServerSpec* s2sSpec_;
};

} // namespace

// = LoginServerApp

#pragma warning (disable: 4355)

LoginServerApp::LoginServerApp()
{
}


LoginServerApp::~LoginServerApp()
{
}


bool LoginServerApp::initLoginService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    SNE_ASSERT(getServerSpec().isValid());

    const CommunityServerInfo communityServerInfo(c2mSpec_.listeningAddresses_[0],
        c2mSpec_.listeningPort_);

    loginService_ = std::make_unique<GideonLoginService>(communityServerInfo);

    auto& theLoginService = static_cast<GideonLoginService&>(*loginService_);

    if (! theLoginService.initialize(c2sSpec_, s2sSpec_,
        getIoContextTask(getS2sIoServiceName()))) {
        return false;
    }

    return true;
}


void LoginServerApp::finishLoginService()
{
    if (loginService_.get() != nullptr) {
        loginService_->finalize();
    }
}


bool LoginServerApp::initDatabaseManager()
{
    sne::server::Profiler profiler(__FUNCTION__);

    sne::base::IoContextTask* nullIoContextTask = nullptr;
    serverbase::GideonDatabaseFactory dbPoolFactory(*nullIoContextTask,
        getServiceConfig().getServiceName());

    databaseManager_ = sne::database::DatabaseManagerFactory::create(
        getServiceConfig().getConfigReader(), dbPoolFactory);
    if (! databaseManager_) {
        return false;
    }

    sne::database::DatabaseManager::instance(databaseManager_.get());
    return true;
}


bool LoginServerApp::loadProperties()
{
    sne::server::Profiler profiler(__FUNCTION__);

    sne::server::Properties properties;
    {
        sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

        if (! db->getProperties(properties)) {
            return false;
        }
    }

    SNE_PROPERTIES::initialize(properties);
    return true;
}


bool LoginServerApp::loadServerSpec()
{
    sne::database::Guard<serverbase::AccountDatabase> db(SNE_DATABASE_MANAGER);

    (void)db->getServerSpec(c2sSpec_, getServiceConfig().getServiceName(), "");
    if (! c2sSpec_.isValid()) {
        return false;
    }

    (void)db->getServerSpec(s2sSpec_, getServiceConfig().getServiceName(), getS2sIoServiceName());
    if (! s2sSpec_.isValid()) {
        return false;
    }

    (void)db->getServerSpec(c2mSpec_, getCommunityServerName(), "");
    if (! c2mSpec_.isValid()) {
        return false;
    }

    return true;
}


bool LoginServerApp::initTaskScheduler()
{
    // TODO: config에서 읽을 것
    const uint16_t threadCount = 2;
    auto* ioContextTask = addIoContextTask("task_scheduler", threadCount);
    if (!ioContextTask) {
        return false;
    }
    taskScheduler_ = std::make_unique<sne::base::TaskScheduler>(*ioContextTask);
    sne::base::TaskScheduler::instance(taskScheduler_.get());
    return true;
}


bool LoginServerApp::initMonitorService()
{
    registerRconQuerySource("user.count", *this);
    registerRconQuerySource("login.count", *this);
    return true;
}


bool LoginServerApp::initSessionManagerForServers()
{
    sne::server::Profiler profiler(__FUNCTION__);

    static SessionFactoryHelperForServers sessionFactoryHelperForServers;

    sessionManagerForServers_ = std::make_unique<sne::server::ServerSideSessionManager>(
        "login.s2s", s2sSpec_, getIoContextTask(getS2sIoServiceName()),
        sessionFactoryHelperForServers);
    if (! sessionManagerForServers_->initialize()) {
        return false;
    }

    if (! sessionManagerForServers_->startSessionAcceptor()) {
        return false;
    }

    return true;
}

// = serverbase::BaseSessionServerApp overriding

bool LoginServerApp::onInit()
{
    if (! initTaskScheduler()) {
        SNE_LOG_ERROR("LoginServerApp::initTaskScheduler() FAILED!");
        return false;
    }

    if (! initDatabaseManager()) {
        return false;
    }

    if (! loadServerSpec()) {
        return false;
    }

    if (! loadProperties()) {
        return false;
    }

    if (! addIoContextTask(getS2sIoServiceName(),
        s2sSpec_.workerThreadCount_)) {
        SNE_LOG_ERROR("LoginServerApp::addProactorTask() FAILED!");
        return false;
    }

    if (! initLoginService()) {
        SNE_LOG_ERROR("LoginServerApp::initLoginService() FAILED!");
        return false;
    }

    if (! initSessionManagerForServers()) {
        SNE_LOG_ERROR("LoginServerApp::initSessionManagerForServers() FAILED!");
        return false;
    }

    if (! sne::server::SessionServerApp::onInit()) {
        return false;
    }

    if (! initMonitorService()) {
        return false;
    }

    return true;
}


void LoginServerApp::onFinish()
{
    sne::server::SessionServerApp::onFinish();

    finishLoginService();
}

// = sne::server::SessionFactoryHelper overriding

std::unique_ptr<sne::base::SessionFactory> LoginServerApp::createSessionFactory(
    const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask)
{
    return std::unique_ptr<sne::base::SessionFactory>(
        new LoginClientSessionFactory(serverSpec, ioContextTask));
}

// = sne::base::SessionValidator overriding

bool LoginServerApp::isValidated() const
{
    if (! sne::server::SessionServerApp::isValidated()) {
        return false;
    }

    return ! loginService_->isServiceThrottling();
}

// = sne::server::rcon::QuerySource overriding

std::string LoginServerApp::query(const std::string& name,
    const std::string& detail) const
{
    if ("user.count" == name) {
        return std::to_string(LOGINUSER_MANAGER->getUserCount());
    }

    if ("login.count" == name) {
        return std::to_string(LOGINUSER_MANAGER->getLoginCount());
    }

    return sne::server::SessionServerApp::query(name, detail);
}

}} // namespace gideon { namespace loginserver {


DEFINE_SNE_APP_FACTORY(gideon::loginserver::LoginServerApp)
