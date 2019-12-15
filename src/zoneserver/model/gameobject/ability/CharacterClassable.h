#pragma once

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class CharacterClassable
 * 설치물을 생성 있다
 */
class CharacterClassable
{
public:
    virtual ~CharacterClassable() {}
	
public:
	virtual CharacterClass getCharacterClass() const = 0;

    virtual void changeCharacterClass(CharacterClass characterClass) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
