#pragma once

#include <gideon/cs/shared/data/GraveStoneInfo.h>


namespace gideon { namespace zoneserver { namespace go {
class GraveStone;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class GraveStoneCallback
 */
class GraveStoneCallback
{
public:
    virtual ~GraveStoneCallback() {}

public:
	virtual void destoryed(go::GraveStone& graveStone) = 0;
    // 자기 자신 무덤이 없어짐을 알린다
    virtual void graveStoneRemoved() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
