#pragma once

#include "../../zoneserver_export.h"
#include "Party.h"
#include "PartyPlayerHelper.h"
#include <sne/core/utility/Singleton.h>


namespace gideon { namespace zoneserver { namespace go {
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver {


/**
 * @class PartyService
 *
 * 파티 서비스
 */
class ZoneServer_Export PartyService : public boost::noncopyable,
    public PartyPlayerHelper
{
    SNE_DECLARE_SINGLETON(PartyService);

    typedef std::mutex LockType;
    typedef sne::core::HashMap<PartyId, PartyPtr> PartyMap;
    typedef sne::core::HashMap<ObjectId, PartyPtr> PlayerPartyMap;

public:
    PartyService();
    ~PartyService();

    bool initialize();
   
public:
    // 커뮤니티 서버 호출용
    PartyPtr createParty(PartyId partyId, PartyType partyType);
    void addPartyMember(PartyId partyId, ObjectId playerId);
    void removePartyMember(ObjectId playerId);   
    void destoryParty(PartyId partyId);

public:
	PartyPtr getPartyByPartyId(PartyId partyId);
    PartyPtr getPartyByPlayerId(ObjectId playerId);

private:
    bool isExist(PartyId partyId) const;

    PartyPtr getPartyByPartyId_i(PartyId partyId);
    PartyPtr getPartyByPlayerId_i(ObjectId playerId);

private:
    virtual go::Entity* getPlayer(ObjectId playerId);
    virtual const go::Entity* getPlayer(ObjectId playerId) const;

private:
    mutable LockType lock_;

    PartyMap partyMap_;
    PlayerPartyMap playerPartyMap_;
};


}} // namespace gideon { namespace zoneserver {


#define PARTY_SERVICE gideon::zoneserver::PartyService::instance()
