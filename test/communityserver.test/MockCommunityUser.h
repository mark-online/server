#pragma once

#include "CommunityServer/user/detail/CommunityUserImpl.h"
#include <sne/test/CallCounter.h>

using namespace sne;
using namespace gideon;

/**
 * @class MockCommunityUser
 *
 * 테스트 용 mock CommunityUser
 */
class MockCommunityUser :
    public communityserver::CommunityUserImpl,
    public sne::test::CallCounter
{
public:
    MockCommunityUser() :
        lastErrorCode_(ecWhatDidYouTest),
        lastPartyAnswer_(false) {}

public:
    // = rpc::WorldMapChannelRpc overriding
    OVERRIDE_SRPC_METHOD_1(evWorldMapChannelEntered,
        MapCode, worldMapCode);
    OVERRIDE_SRPC_METHOD_1(evWorldMapChannelLeft,
        MapCode, worldMapCode);
    OVERRIDE_SRPC_METHOD_3(evWorldMapSaid,
        ObjectId, playerId, Nickname, nickname, ChatMessage, message);

    OVERRIDE_SRPC_METHOD_2(onInviteParty,
        ErrorCode, errorCode, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_1(onRespondPartyInvitation,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onLeaveParty,
        ErrorCode, errorCode);

    OVERRIDE_SRPC_METHOD_1(evPartyInvited,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(evPartyInvitationReponded,
        Nickname, nickname, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_1(evPartyMeberAdded,
        PartyMemberInfo, memberInfo);
    OVERRIDE_SRPC_METHOD_2(evPartyMemberInfos,
        PartyMemberInfos, memberInfos, PartyType, partyType);
    OVERRIDE_SRPC_METHOD_1(evPartyMemberLeft,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evPartyMemberKicked,
        ObjectId, playerId);
    OVERRIDE_SRPC_METHOD_1(evPartyMasterChanged,
        ObjectId, masterId);

    OVERRIDE_SRPC_METHOD_1(evShardNoticed,
        ChatMessage, message);

    OVERRIDE_SRPC_METHOD_7(evGuildInfo,
        BaseGuildInfo, guildInfo, GuildMemberInfoMap, guildMembers,
        GuildRelationshipInfoMap, relaytionMap,
        GuildIntroduction, introduction, GuildNotice, guildNotice,
        GuildRankInfos, rankInfos, GuildLevelInfo, guildLevelInfo);
    OVERRIDE_SRPC_METHOD_2(onInviteGuild,
        ErrorCode, errorCode, Nickname, nickname);
    OVERRIDE_SRPC_METHOD_1(onCreateGuild,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onRespondGuildInvitation,
        ErrorCode, errorCode, GuildId, guildId);
    OVERRIDE_SRPC_METHOD_1(onLeaveGuild,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onAddGuildRelationship,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_1(onRemoveGuildRelationship,
        ErrorCode, errorCode);
    OVERRIDE_SRPC_METHOD_2(onSearchGuildInfo,
        ErrorCode, errorCode, SearchGuildInfos, searchGuildInfos);

    OVERRIDE_SRPC_METHOD_1(evGuildInvited,
        Nickname, nickname);
    OVERRIDE_SRPC_METHOD_2(evGuildInvitationResponded,
        Nickname, nickname, bool, isAnswer);
    OVERRIDE_SRPC_METHOD_3(evGuildMemberSaid,
        ObjectId, playerId, Nickname, nickname, ChatMessage, message);
    OVERRIDE_SRPC_METHOD_1(evGuildMemberLeft,
        ObjectId, playerId);
public:
    ErrorCode lastErrorCode_;
    bool lastPartyAnswer_;
    bool lastGuildAnswer_;
    PartyMemberInfos lastMemberInfos_;
    PartyMemberInfo lastMemberInfo_;
    ObjectId leftGuildMemberId_;
    SearchGuildInfos lastSearchGuildInfos_;
};
