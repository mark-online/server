#include "BotPCH.h"
#include "Config.h"
#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4127 4701 4819)
#endif
#include <boost/lexical_cast.hpp>
#ifdef _MSC_VER
#  pragma warning (pop)
#endif
#include <iostream>

namespace gideon { namespace bot {

namespace
{

template <typename T>
inline T toInteger(const std::string& s)
{
    try {
        return boost::lexical_cast<T>(s.c_str());
    }
    catch(const boost::bad_lexical_cast &) {}
    return 0;
}


void parseAddress(std::string& ip, uint16_t& port,
    const std::string& address)
{
    ip.clear();
    port = 0;

    const size_t pos = address.find_first_of(':');
    if (pos != std::string::npos) {
        ip = address.substr(0, pos);
        port = toInteger<uint16_t>(address.substr(pos + 1));
    }
}

} // namespace

// = Config

Config::Config() :
    startAccountId_(1),
    numberOfClients_(1),
    serverPort_(0),
    shouldRetry_(false),
    verbose_(false)
{
}


bool Config::parseArgs(int argc, char* argv[])
{
    if (argc < 2) {
        return false;
    }

    bool isOk = true;
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "-?") {
            isOk = false;
            break;
        }
        
        if (arg == "-v") {
            verbose_ = true;
        }
        else if (arg == "-r") {
            shouldRetry_ = true;
        }
        else if (arg.compare(0, 3, "-s=") == 0) {
            parseAddress(serverAddress_, serverPort_, arg.substr(3));
        }
        else if (arg.compare(0, 3, "-a=") == 0) {
            startAccountId_ = toInteger<AccountId>(arg.substr(3));
        }
        else if (arg.compare(0, 3, "-c=") == 0) {
            numberOfClients_ = toInteger<uint16_t>(arg.substr(3));
        }
        else {
            std::cerr << "Unknown option '" << arg.c_str() << "', ignoring it.\n";
        }
    }

    if (serverAddress_.empty() || (serverPort_ == 0)) {
        isOk = false;
    }
    else if (numberOfClients_ <= 0) {
        isOk = false;
    }

    return isOk;
}


void Config::printUsage()
{
    std::cout << "== Gideon Bot ==\n" <<
        "USAGE: Bot [options]\n" <<
        "  -s=<address:port> server address\n" <<
        "  -a=<account-id> start AccountId(default: 1)\n" <<
        "  -c=<count> the number of clients(default: 1)\n" <<
        "  -r retry if failed(default: off)\n" <<
        "  -v verbose (default: off)\n" <<
        " ex)\n" <<
        "   Bot -s=127.0.0.1:10000 -a=1 -c=100 -v\n";
}

}} // namespace gideon { namespace bot {
