#pragma once

#include <sne/database/Connection.h>

namespace gideon { namespace servertest {

/**
* @class MockConnection
*/
class MockConnection : public sne::database::Connection
{
private:
    virtual bool open(const std::string& /*connectionString*/,
        uint8_t /*connectionTimeout*/) {
        return true;
    }

    virtual void close() {}

    virtual bool isOpened() {
        return true;
    }

    virtual std::string getLastError() const {
        return "";
    }

    virtual unsigned int getLastResult() const {
        return 0;
    }
};

}} // namespace gideon { namespace servertest {
