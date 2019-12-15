#pragma once

#include "DataCodes.h"
#include <gideon/Common.h>
#include <gideon/cs/datatable/FactionTable.h>

namespace gideon { namespace servertest {

/**
* @class MockFactionTable
*/
class MockFactionTable : public datatable::FactionTable
{
public:
    MockFactionTable() {
        fillFactions();
    }

    virtual const datatable::FactionTemplate* getFaction(FactionCode code) const {
        const FactionMap::const_iterator pos = factionMap_.find(code);
        if (pos != factionMap_.end()) {
            return &((*pos).second);
        }
        return nullptr;
    }

private:
    virtual const FactionMap& getFactionMap() const {
        return factionMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillFactions() {
        factionMap_.emplace(playerFactionCode,
            datatable::FactionTemplate(playerFactionCode, famAllPlayers, famUnknown, famMonster));
        factionMap_.emplace(defaultMonsterFactionCode,
            datatable::FactionTemplate(defaultMonsterFactionCode, famMonster, famMonster, famMonster));
    }

private:
    FactionMap factionMap_;
};

}} // namespace gideon { namespace servertest {
