#pragma once

#include "DataCodes.h"
#include <gideon/Common.h>
#include <gideon/cs/datatable/EventTriggerTable.h>

namespace gideon { namespace servertest {

/**
* @class MockEventTriggerTable
*/
class MockEventTriggerTable : public datatable::EventTriggerTable
{
public:
    MockEventTriggerTable() {}

    virtual const EventTriggerList* getEventTriggerList(DataCode /*code*/) const {
        return nullptr;
    }

private:
    virtual const EventTriggerListMap& getEventTriggerListMap() const {
        return eventAiListMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    EventTriggerListMap eventAiListMap_;
};

}} // namespace gideon { namespace servertest {
