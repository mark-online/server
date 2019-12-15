#pragma once
#include <gideon/cs/shared/data/CharacterTitleInfo.h>

namespace gideon { namespace zoneserver { namespace go {

/**
* @class CharacterTitleable
 * 칭호를 가질수 있다
 */
class CharacterTitleable
{
public:
    virtual ~CharacterTitleable() {}
	
public:
    virtual void initializeCharacterTitle(ErrorCode errorCode, const CharacterTitleCodeSet& titleCodeSet) = 0;
	virtual ErrorCode selectCharacterTitle(CharacterTitleCode titleCode) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
