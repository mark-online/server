#pragma once

#include <gideon/cs/shared/data/DuelInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/UserId.h>

namespace gideon { namespace zoneserver { namespace gc {

/****
 * @class FieldDuelCallback
 *
 ***/
class FieldDuelCallback
{
public:
    virtual void fieldDuelRequested(ObjectId playerId, const Nickname& nickname) = 0;
    virtual void fieldDuelResponed(ObjectId playerId, bool isAnswer) = 0;
    virtual void fieldDuelReadied(const Position& centerPosition) = 0;
    virtual void fieldDuelStarted() = 0;
    virtual void fieldDuelStopped(FieldDuelResultType type) = 0;

    // 브로드 케스팅용    
    virtual void fieldDeulResult(ObjectId winnerId, const Nickname& dueler1, 
        ObjectId loserId, const Nickname& dueler2) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {