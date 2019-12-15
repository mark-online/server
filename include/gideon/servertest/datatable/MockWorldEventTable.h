#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/WorldEventTable.h>

namespace gideon { namespace servertest {

/**
* @class MockWorldEventTable
*/
class GIDEON_SERVER_API MockWorldEventTable : public gideon::datatable::WorldEventTable
{
public:
    MockWorldEventTable() {}

    virtual const gdt::world_event_t* getWorldEventTemplate(WorldEventCode code) const;

private:
    virtual const WorldEventMap& getWorldEventMap() const {
        return worldMapMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    WorldEventMap worldMapMap_;
};

}} // namespace gideon { namespace servertest {
