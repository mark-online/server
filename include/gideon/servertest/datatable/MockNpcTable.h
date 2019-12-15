#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace servertest {

/**
* @class MockNpcTable
*/
class GIDEON_SERVER_API MockNpcTable : public datatable::NpcTable
{
    enum {cpDefault = 100};
public:
    MockNpcTable() {
        fillMonsters();
    }
    virtual ~MockNpcTable() {
        destroyMonsters();
    }

    virtual const datatable::NpcTemplate* getNpcTemplate(NpcCode code) const;

private:
    virtual const NpcMap& getNpcMap() const {
        return monsterMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillMonsters() {
        fillNormalMonster();
        fillServantMonster();
        fillBossMonster();
        fillNpcQuest();
        fillStoreNpc();
    }

    void fillNormalMonster();
    void fillServantMonster();
    void fillBossMonster();
    void destroyMonsters();
    void fillNpcQuest();
    void fillStoreNpc();
private:
    NpcMap monsterMap_;
};

}} // namespace gideon { namespace servertest {
