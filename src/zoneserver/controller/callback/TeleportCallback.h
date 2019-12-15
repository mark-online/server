#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <gideon/cs/shared/data/UserId.h>

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class TeleportCallback
 */
class TeleportCallback
{
public:
    virtual ~TeleportCallback() {}

public:
	virtual void completed(ObjectId bindRecallId) = 0;

	virtual void recallReadied() = 0;

	virtual void recallRequested(const Nickname& nickname, const WorldPosition& worldPosition) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace gc {
