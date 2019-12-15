#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/RandomDungeonTable.h>

namespace gideon { namespace servertest {

const MapCode firstRandomDungeonMapCode = 184745985;

/**
* @class MockRandomDungeonTable
*/
class GIDEON_SERVER_API MockRandomDungeonTable : public gideon::datatable::RandomDungeonTable
{
public:
    MockRandomDungeonTable() {
        fillRandomDungeons();
    }
    virtual ~MockRandomDungeonTable() {
        destroyRandomDungeons();
    }

private:
    virtual const datatable::RandomDungeonTemplate* getRandomDungeonTemplate(MapCode mapCode) const;

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillRandomDungeons() {
        fillNormalRandomDungeon();
    }

    void fillNormalRandomDungeon();

    void destroyRandomDungeons();

private:
    RandomDungeonMap randomDungeonMap_;
};

}} // namespace gideon { namespace servertest {
