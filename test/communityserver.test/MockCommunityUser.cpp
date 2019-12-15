#include "CommunityServerTestPCH.h"
#include "MockCommunityUser.h"

// = rpc::WorldMapChannelRpc overriding

DEFINE_SRPC_METHOD_1(MockCommunityUser, evWorldMapChannelEntered,
    MapCode, worldMapCode)
{
    addCallCount("evWorldMapChannelEntered");
    worldMapCode;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evWorldMapChannelLeft,
    MapCode, worldMapCode)
{
    addCallCount("evWorldMapChannelLeft");
    worldMapCode;
}


DEFINE_SRPC_METHOD_3(MockCommunityUser, evWorldMapSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message)
{
    addCallCount("evWorldMapSaid");
    playerId, nickname, message;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evShardNoticed,
    ChatMessage, message)
{
    addCallCount("evShardNoticed");
    message;
}


DEFINE_SRPC_METHOD_2(MockCommunityUser, onInviteParty,
    ErrorCode, errorCode, Nickname, nickname)
{
    addCallCount("onInviteParty");
    lastErrorCode_ = errorCode;
    nickname;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, onRespondPartyInvitation,
    ErrorCode, errorCode)
{
    addCallCount("onRespondPartyInvitation");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, onLeaveParty,
    ErrorCode, errorCode)
{
    addCallCount("onLeaveParty");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evPartyInvited,
    Nickname, nickname)
{
    addCallCount("evPartyInvited");
    nickname;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evPartyMeberAdded,
    PartyMemberInfo, memberInfo)
{
    addCallCount("evPartyMeberAdded");
    lastMemberInfo_ = memberInfo;
}


DEFINE_SRPC_METHOD_2(MockCommunityUser, evPartyMemberInfos,
    PartyMemberInfos, memberInfos, PartyType, partyType)
{
    addCallCount("evPartyMemberInfos");
    lastMemberInfos_ = memberInfos;
    partyType;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evPartyMemberLeft,
    ObjectId, playerId)
{
    addCallCount("evPartyMemberLeft");
    playerId;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evPartyMemberKicked,
    ObjectId, playerId)
{
    addCallCount("evPartyMemberKicked");
    playerId;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evPartyMasterChanged,
    ObjectId, masterId)
{
    addCallCount("evPartyMasterChanged");
    masterId;
}


DEFINE_SRPC_METHOD_2(MockCommunityUser, evPartyInvitationReponded,
    Nickname, nickname, bool, isAnswer)
{
    addCallCount("evPartyInvitationReponded");
    nickname;
    lastPartyAnswer_ = isAnswer;
}


DEFINE_SRPC_METHOD_7(MockCommunityUser, evGuildInfo,
    BaseGuildInfo, guildInfo, GuildMemberInfoMap, guildMembers,
    GuildRelationshipInfoMap, relaytionMap,
    GuildIntroduction, introduction, GuildNotice, guildNotice,
    GuildRankInfos, rankInfos, GuildLevelInfo, guildLevelInfo)
{
    addCallCount("evGuildInfo");
    guildInfo, guildMembers, relaytionMap, introduction, guildNotice, rankInfos, guildLevelInfo;
}


DEFINE_SRPC_METHOD_2(MockCommunityUser, onInviteGuild,
    ErrorCode, errorCode, Nickname, nickname)
{
    addCallCount("onInviteGuild");
    lastErrorCode_ = errorCode;
    nickname;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, onCreateGuild,
    ErrorCode, errorCode)
{
    addCallCount("onCreateGuild");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_2(MockCommunityUser, onRespondGuildInvitation,
    ErrorCode, errorCode, GuildId, guildId)
{
    addCallCount("onRespondGuildInvitation");
    lastErrorCode_ = errorCode;
    guildId;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, onLeaveGuild,
    ErrorCode, errorCode)
{
    addCallCount("onLeaveGuild");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evGuildInvited,
    Nickname, nickname)
{
    addCallCount("evGuildInvited");
    nickname;
}


DEFINE_SRPC_METHOD_2(MockCommunityUser, evGuildInvitationResponded,
    Nickname, nickname, bool, isAnswer)
{
    addCallCount("evGuildInvitationResponded");
    lastGuildAnswer_ = isAnswer;
    nickname;
}


DEFINE_SRPC_METHOD_3(MockCommunityUser, evGuildMemberSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message)
{
    addCallCount("evGuildMemberSaid");
    playerId, nickname, message;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, evGuildMemberLeft,
    ObjectId, playerId)
{
    addCallCount("evGuildMemberLeft");
    leftGuildMemberId_ = playerId;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, onAddGuildRelationship,
    ErrorCode, errorCode)
{
    addCallCount("onAddGuildRelationship");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_1(MockCommunityUser, onRemoveGuildRelationship,
    ErrorCode, errorCode)
{
    addCallCount("onRemoveGuildRelationship");
    lastErrorCode_ = errorCode;
}


DEFINE_SRPC_METHOD_2(MockCommunityUser, onSearchGuildInfo,
    ErrorCode, errorCode, SearchGuildInfos, searchGuildInfos)
{
    addCallCount("onSearchGuildInfo");
    lastErrorCode_ = errorCode;
    lastSearchGuildInfos_ = searchGuildInfos;
}