#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/AccountId.h>
#include <string>

namespace gideon { namespace bot {

/**
 * @class Config
 * Configuration.
 */
class Config
{
public:
    enum {
        heartbeatInterval = 3 * 1000, // milli-seconds
    };

public:
    Config();

    bool parseArgs(int argc, char* argv[]);

    void printUsage();

    AccountId getStartAccountId() const {
        return startAccountId_;
    }

    size_t getNumberOfClients() const {
        return numberOfClients_;
    }

    const std::string& getServerAddress() const {
        return serverAddress_;
    }

    uint16_t getServerPort() const {
        return serverPort_;
    }

    bool shouldRetry() const {
        return shouldRetry_;
    }

    size_t getLogoutInterval() const {
        return 30;
    }

    bool isVerbose() const {
        return verbose_;
    }

private:
    AccountId startAccountId_;
    size_t numberOfClients_;
    std::string serverAddress_;
    uint16_t serverPort_;
    bool shouldRetry_;
    bool verbose_;
};

}} // namespace gideon { namespace bot {
