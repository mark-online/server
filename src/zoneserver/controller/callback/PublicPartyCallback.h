#pragma once

#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PublicPartyCallback
 */
class PublicPartyCallback
{
public:
    virtual ~PublicPartyCallback() {}
    
    virtual ErrorCode joinParty(go::Entity& player) = 0;
    virtual void leaveParty(go::Entity& player) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
