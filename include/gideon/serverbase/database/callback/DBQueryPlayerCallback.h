#pragma once

#include <gideon/cs/shared/data/AchievementInfo.h>
#include <gideon/cs/shared/data/CharacterTitleInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace serverbase {


/***
 * @class DBQueryPlayerCallback
 ***/
class DBQueryPlayerCallback
{
public:
    virtual ~DBQueryPlayerCallback() {}

    virtual void registPlayer(zoneserver::go::Entity& player) = 0;
    virtual void unregistPlayer(ObjectId playerId) = 0;

    virtual void onQueryAchievements(ErrorCode errorCode, ObjectId playerId, const ProcessAchievementInfoMap& processInfoMap,
        const CompleteAchievementInfoMap& completeInfoMap) = 0;
    virtual void onQueryCharacterTitles(ErrorCode errorCode, ObjectId playerId, const CharacterTitleCodeSet& titleCodeSet) = 0;
};


}} // namespace gideon { namespace serverbase {