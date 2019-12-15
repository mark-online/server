#pragma once

#include <gideon/cs/shared/data/PartyInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include "../../../service/party/Party.h"


namespace gideon { namespace zoneserver { namespace go {


/**
 * @class Partyable
 * 파티를 할수 있다
 */
class Partyable
{
public:
    virtual ~Partyable() {}
	
public:
    virtual bool isPartyMember() const = 0;
    virtual bool isSameParty(PartyId partyId) const = 0;
	virtual PartyPtr getParty() = 0;
	virtual const PartyPtr getParty() const = 0;

    virtual void setParty(PartyPtr party) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
