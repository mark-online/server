#include "BotPCH.h"
#include "Config.h"
#include "Bot.h"
#include <sne/base/utility/LogManager.h>
#include <iostream>
#include <ctime>

namespace gideon { namespace bot {

#pragma warning (disable: 4702)

namespace
{

sne::base::LogManager  logManager;


/**
 * @class BotRunner
 */
class BotRunner
{
    typedef std::shared_ptr<Bot> BotPtr;
    typedef std::vector<BotPtr> Bots;
public:
    BotRunner(const Config& config) :
        config_(config) {
        initBots();
    }

    void run() {
        Bots::iterator posBots = bots_.begin();
        const Bots::iterator endBots = bots_.end();
        for (;;) {
            BotPtr character = *posBots;
            character->tick();

            if (++posBots == endBots) {
                posBots = bots_.begin();
#ifdef _WIN32
                ::Sleep(1);
#endif
            }
        }

        stop();
    }
private:
    void initBots() {
        std::wstring id;
        for (size_t i = 0; i < config_.getNumberOfClients(); ++i) {
            id = sne::core::formatString(L"test%u", (config_.getStartAccountId() + i));
            bots_.emplace_back(std::make_shared<Bot>(config_, id, "123456"));
        }
    }

    void stop() {
        Bots::iterator posBots = bots_.begin();
        const Bots::iterator endBots = bots_.end();
        for (; posBots != endBots; ++posBots) {
            BotPtr character(*posBots);
            character->disconnect();
        }
    }

private:
    const Config& config_;
    Bots bots_;
};


void initLogManager(const Config& config)
{
    if (config.isVerbose()) {
        logManager.verbose();
    }
    else {
        logManager.noDebug();

        fclose(stdout);
        fclose(stderr);
    }
}

} // namespace

}} // namespace gideon { namespace bot {

int main(int argc, char* argv[])
{
    srand(static_cast<unsigned int>(time(0)));

    // for Korean
    //std::locale::global(std::locale("Korean_korea"));
    //std::wcout.imbue(std::locale("kor"));

    gideon::bot::Config config;
    if (! config.parseArgs(argc, argv)) {
        config.printUsage();
        return EXIT_FAILURE;
    }

    gideon::bot::initLogManager(config);

    gideon::bot::BotRunner runner(config);
    runner.run();

    return EXIT_SUCCESS;
}
