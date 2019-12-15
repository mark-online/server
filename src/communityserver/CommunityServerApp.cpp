#include "CommunityServerPCH.h"
#include "CommunityServerApp.h"
#include "CommunityService.h"
#include "s2s/CommunityLoginServerProxy.h"
#include "s2s/CommunityServerSideProxyFactory.h"
#include "c2s/CommunityClientSessionFactory.h"
#include "user/detail/CommunityUserImpl.h"
#include "user/CommunityUserAllocator.h"
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/serverbase/datatable/DataTableLoader.h>
#include <gideon/server/data/ServerAddress.h>
#include <sne/server/s2s/ClientSideProxyFactory.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/server/session/ServerSideSessionManager.h>
#include <sne/server/app/Factory.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace communityserver {

namespace {


/**
 * @class CommunityUserAllocatorImpl
 */
class CommunityUserAllocatorImpl : public CommunityUserAllocator
{
public:
    CommunityUser* malloc() {
        return new CommunityUserImpl;
    }

    void free(CommunityUser* user) {
        boost::checked_delete(user);
    }
};


/**
 * @class SessionFactoryHelperForServers
 * S2S 세션 팩토리 도우미
 */
class SessionFactoryHelperForServers : public sne::server::SessionFactoryHelper
{
public:
    virtual std::unique_ptr<sne::base::SessionFactory> createSessionFactory(
        const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask) {
        return std::unique_ptr<sne::base::SessionFactory>(
            new CommunityServerSideProxyFactory(serverSpec, ioContextTask));
    }
};


/**
 * @class GideonCommunityService
 */
class GideonCommunityService : public CommunityService
{
public:
    GideonCommunityService() :
        ioService_(nullptr),
        c2sSpec_(nullptr),
        s2sSpec_(nullptr),
        m2lSpec_(nullptr) {}

    bool initialize(const sne::server::ServerSpec& c2sSpec,
        const sne::server::ServerSpec& s2sSpec,
        const sne::server::ServerSpec& m2lSpec,
        sne::base::IoContextTask& ioContextTask) {
        c2sSpec_ = &c2sSpec;
        s2sSpec_ = &s2sSpec;
        m2lSpec_ = &m2lSpec;
        ioService_ = &ioContextTask;

        return CommunityService::initialize(c2sSpec_->name_);
    }

private:
    virtual const sne::server::S2sCertificate& getServerCertificate() const {
        return s2sSpec_->certificate_;
    }

    virtual size_t getMaxUserCount() const {
        return c2sSpec_->maxUserCount_;
    }

    virtual std::unique_ptr<CommunityUserAllocator> createCommunityUserAllocator() const {
        return std::make_unique<CommunityUserAllocatorImpl>();
    }

    virtual std::unique_ptr<CommunityLoginServerProxy> createLoginServerProxy() const {
        auto proxy =
            sne::server::ClientSideProxyFactory<CommunityLoginServerProxy>::create(
                sne::server::toServerInfo(*m2lSpec_), *ioService_);

        if (! proxy->connect()) {
            proxy.reset();
        }
        return proxy;
    }

private:
    sne::base::IoContextTask* ioService_;
    const sne::server::ServerSpec* c2sSpec_;
    const sne::server::ServerSpec* s2sSpec_;
    const sne::server::ServerSpec* m2lSpec_;
};

} // namespace

// = CommunityServerApp

CommunityServerApp::CommunityServerApp()
{
}


CommunityServerApp::~CommunityServerApp()
{
}



bool CommunityServerApp::initTaskScheduler()
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


bool CommunityServerApp::initCommunityService()
{
    sne::server::Profiler profiler(__FUNCTION__);

    SNE_ASSERT(getServerSpec().isValid());

    communityService_ = std::make_unique<GideonCommunityService>();

    GideonCommunityService& theCommunityService =
        static_cast<GideonCommunityService&>(*communityService_);

    if (! theCommunityService.initialize(getServerSpec(), s2sSpec_, m2lSpec_,
        getIoContextTask(getS2sIoServiceName()))) {
        return false;
    }

    return true;
}


void CommunityServerApp::finishCommunityService()
{
}


bool CommunityServerApp::loadServerSpec()
{
    m2lSpec_ = getServerSpec(getLoginServerName(), getS2sIoServiceName());
    if (! m2lSpec_.isValid()) {
        return false;
    }

    s2sSpec_ = getServerSpec(getServiceConfig().getServiceName(), getS2sIoServiceName());
    return s2sSpec_.isValid();
}


bool CommunityServerApp::loadDataTables()
{
    if (!serverbase::DataTableLoader::loadPropertyTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadGuildSkillTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadGuildLevelTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadAccessoryTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadGemTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadRecipeTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadElementTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadFunctionTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadFragmentTable()) {
        return false;
    }

    if (!serverbase::DataTableLoader::loadBuildingTable()) {
        return false;
    }
    return true;
}


bool CommunityServerApp::initMonitorService()
{
    registerRconQuerySource("user.count", *this);
    registerRconQuerySource("login.count", *this);
    return true;
}


bool CommunityServerApp::initSessionManagerForServers()
{
    sne::server::Profiler profiler(__FUNCTION__);

    static SessionFactoryHelperForServers sessionFactoryHelperForServers;

    sessionManagerForServers_ =
        std::make_unique<sne::server::ServerSideSessionManager>(
            "community.s2s",
            s2sSpec_, getIoContextTask(getS2sIoServiceName()),
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

bool CommunityServerApp::onInit()
{
    if (! serverbase::BaseSessionServerApp::onInit()) {
        return false;
    }

    if (! loadServerSpec()) {
        SNE_LOG_ERROR("CommunityServerApp::loadServerSpec() FAILED!");
        return false;
    }

    if (! initTaskScheduler()) {
        SNE_LOG_ERROR("ZoneServerApp::initTaskScheduler() FAILED!");
        return false;
    }

    if (! initMonitorService()) {
        SNE_LOG_ERROR("CommunityServerApp::initMonitorService() FAILED!");
        return false;
    }

    if (! addIoContextTask(getS2sIoServiceName(),
        m2lSpec_.workerThreadCount_)) {
        SNE_LOG_ERROR("CommunityServerApp::addIoContextTask() FAILED!");
        return false;
    }

    if (!loadDataTables()) {
        return false;
    }

    if (! initCommunityService()) {
        SNE_LOG_ERROR("CommunityServerApp::initCommunityService() FAILED!");
        return false;
    }

    if (! initSessionManagerForServers()) {
        SNE_LOG_ERROR("CommunityServerApp::initSessionManagerForServers() FAILED!");
        return false;
    }

    return true;
}


bool CommunityServerApp::onStart()
{
    communityService_->ready();

    if (! serverbase::BaseSessionServerApp::onStart()) {
        return false;
    }

    return true;
}


void CommunityServerApp::onFinish()
{
    serverbase::BaseSessionServerApp::onFinish();

    finishCommunityService();
}

// = sne::server::SessionFactoryHelper overriding

std::unique_ptr<sne::base::SessionFactory> CommunityServerApp::createSessionFactory(
    const sne::server::ServerSpec& serverSpec, sne::base::IoContextTask& ioContextTask)
{
    return std::make_unique<CommunityClientSessionFactory>(serverSpec, ioContextTask);
}

// = rcon::QuerySource overriding

std::string CommunityServerApp::query(const std::string& name,
    const std::string& detail) const
{
    return serverbase::BaseSessionServerApp::query(name, detail);
}

}} // namespace gideon { namespace communityserver {

DEFINE_SNE_APP_FACTORY(gideon::communityserver::CommunityServerApp)

