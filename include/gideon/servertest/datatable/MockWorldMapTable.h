#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/WorldMapTable.h>

namespace gideon { namespace servertest {

const MapCode globalWorldMapCode = makeMapCode(mtGlobalWorld, 1);
const MapCode otherWorldMapCode = makeMapCode(mtGlobalWorld, 2);


/**
* @class MockWorldMapTable
*/
class GIDEON_SERVER_API MockWorldMapTable : public gideon::datatable::WorldMapTable
{
public:
    MockWorldMapTable() {
        fillMaps();
    }
    virtual ~MockWorldMapTable() {
        destroyMaps();
    }

    virtual const gdt::map_t* getMap(MapCode code) const;

private:
    virtual const WorldMapMap& getWorldMapMap() const {
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
    void fillMaps() {
        fillInitialWorldMap();
    }

    void fillInitialWorldMap();

    void destroyMaps();

private:
    WorldMapMap worldMapMap_;
};

}} // namespace gideon { namespace servertest {
