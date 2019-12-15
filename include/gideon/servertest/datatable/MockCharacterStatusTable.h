#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/CharacterStatusTable.h>

namespace gideon { namespace servertest {

/**
* @class MockCharacterStatusTable
*/
class GIDEON_SERVER_API MockCharacterStatusTable : public gideon::datatable::CharacterStatusTable
{
public:
    MockCharacterStatusTable() {}

    virtual const gdt::character_status_t* getCharacterStatus(
        CharacterClass cc, CreatureLevel cl, LevelStep ls) const;

private:
    virtual const CharacterClassStatusMap& getCharacterClassStatusMap() const {
        return charClassStatusMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }
    
private:
    CharacterClassStatusMap charClassStatusMap_;
};

}} // namespace gideon { namespace servertest {
