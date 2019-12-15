#pragma once

#include <gideon/cs/shared/data/EmotionInfo.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class EmotionCallback
 */
class EmotionCallback
{
public:
    virtual ~EmotionCallback() {}

public:
    virtual void emotionNotified(ObjectId playerId, const std::string& emotion) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
