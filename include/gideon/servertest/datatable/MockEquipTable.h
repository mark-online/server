#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/EquipTable.h>

namespace gideon { namespace servertest {

/**
* @class MockEquipTable
*/
class GIDEON_SERVER_API MockEquipTable : public gideon::datatable::EquipTable
{
public:
    MockEquipTable() {
        fillItems();
    }
    virtual ~MockEquipTable() {
        destroyItems();
    }

    virtual const gdt::equip_t* getEquip(EquipCode code) const;

private:
    virtual const EquipMap& getEquipMap() const {
        return equipMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillItems() {
        fillOneHandSword();
        fillUpgradeOneHandSword();
        fillShield();
        fillUpgradeShield();
        fillHelmet();
        fillUpgradeHelmet();
        fillOtherHelmet();
        fillUpgradeOtherHelmet();
        fillLance();
        fillUpgradeLance();
        fillShoes();
        fillUpgradeShoes();
    }

    void fillOneHandSword();
    void fillUpgradeOneHandSword();
    void fillShield();
    void fillUpgradeShield();
    void fillHelmet();
    void fillUpgradeHelmet();
    void fillOtherHelmet();
    void fillUpgradeOtherHelmet();
    void fillLance();
    void fillUpgradeLance();
    void fillShoes();
    void fillUpgradeShoes();

    void destroyItems(); 

private:
    EquipMap equipMap_;
};

}} // namespace gideon { namespace servertest {
