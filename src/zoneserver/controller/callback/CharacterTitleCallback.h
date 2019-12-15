#pragma once

#include <gideon/cs/shared/data/CharacterTitleInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class CharacterTitleCallback
 */
class CharacterTitleCallback
{
public:
    virtual ~CharacterTitleCallback() {}

public:
    virtual void characterTitleAdded(CharacterTitleCode titleCode) = 0;

    virtual void characterTitleSelected(ObjectId playerId, CharacterTitleCode titleCode) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
