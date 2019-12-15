#include "BotPCH.h"
#include "BotCommunity.h"
#include <gideon/clientnet/ServerProxy.h>
#include <iostream>

namespace gideon { namespace bot {

IMPLEMENT_SRPC_EVENT_DISPATCHER(BotCommunity);

BotCommunity::BotCommunity(clientnet::ServerProxy& serverProxy)
{
    serverProxy.registerRpcForwarderForCommunityServer(*this);
    serverProxy.registerRpcReceiverForCommunityServer(*this);
}


void BotCommunity::initialize(const UserId& userId)
{
    userId_ = userId;
}


void BotCommunity::reset()
{
    userId_.clear();
}


// = sne::srpc::RpcForwarder overriding

void BotCommunity::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    std::wcout << L"BotCommunity::onForwarding(" << rpcId.get() << L"," <<
        rpcId.getMethodName() << L")\n";
}

// = sne::srpc::RpcReceiver overriding

void BotCommunity::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    std::wcout << L"BotCommunity::onReceiving(" << rpcId.get() << L"," <<
        rpcId.getMethodName() << L")\n";
}

// = rpc::ShardChannelRpc overriding

FORWARD_SRPC_METHOD_1(BotCommunity, sayInShard,
    ChatMessage, message);


RECEIVE_SRPC_METHOD_3(BotCommunity, evShardSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message)
{
    std::wcout << L"* " << userId_ <<
        L": Character(" << nickname <<
        L"): " << message << L").\n";
    playerId;
}

// = rpc::WorldMapChannelRpc overriding

FORWARD_SRPC_METHOD_1(BotCommunity, sayInWorldMap,
    ChatMessage, message);


RECEIVE_SRPC_METHOD_1(BotCommunity, evWorldMapChannelEntered,
    MapCode, worldMapCode)
{
    sayInWorldMap(L"hello");

    worldMapCode;
    // TODO:
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evWorldMapChannelLeft,
    MapCode, worldMapCode)
{
    worldMapCode;
    // TODO:
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evWorldMapSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message)
{
    std::wcout << L"* " << userId_ <<
        L": Character(" << nickname <<
        L"): " << message << L").\n";
    playerId;
}

// = rpc::NoticeRpc overriding

FORWARD_SRPC_METHOD_1(BotCommunity, noticeToShard,
    ChatMessage, message);


RECEIVE_SRPC_METHOD_1(BotCommunity, evShardNoticed,
    ChatMessage, message)
{
    std::wcout << L"* " << userId_ <<
        L": Notice(" << message << L").\n";
}

// = CommunityPartyRpc overriding

FORWARD_SRPC_METHOD_1(BotCommunity, inviteParty,
    Nickname, nickname);    

FORWARD_SRPC_METHOD_0(BotCommunity, leaveGuild)


FORWARD_SRPC_METHOD_2(BotCommunity, respondPartyInvitation,
    Nickname, nickname, bool, isAnswer);


FORWARD_SRPC_METHOD_0(BotCommunity, leaveParty);


FORWARD_SRPC_METHOD_1(BotCommunity, sayPartyMember,
    ChatMessage, message);


FORWARD_SRPC_METHOD_1(BotCommunity, kickPartyMember,
    ObjectId, playerId);


FORWARD_SRPC_METHOD_1(BotCommunity, setWaypoint,
    Waypoint, waypoint);


FORWARD_SRPC_METHOD_1(BotCommunity, delegatePartyMaster,
    ObjectId, newMasterId);


FORWARD_SRPC_METHOD_1(BotCommunity, responsePublicParty,
    PartyId, partyId);


FORWARD_SRPC_METHOD_1(BotCommunity, changePartyType,
    PartyType, partyType);


FORWARD_SRPC_METHOD_2(BotCommunity, moveRaidPartyGroupPosition,
    ObjectId, playerId, PartyPosition, position); 


FORWARD_SRPC_METHOD_2(BotCommunity, switchRaidPartyGroupPosition,
    ObjectId, srcId, ObjectId, descId); 


RECEIVE_SRPC_METHOD_2(BotCommunity, onInviteParty,
    ErrorCode, errorCode, Nickname, nickname)
{
    errorCode, nickname;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onRespondPartyInvitation,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onLeaveParty,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onKickPartyMember,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onDelegatePartyMaster,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onResponsePublicParty,
    ErrorCode, errorCode, PartyId, partyId)
{
    errorCode, partyId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onChangePartyType,
    ErrorCode, errorCode, PartyType, partyType)
{
    errorCode, partyType;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, onMoveRaidPartyGroupPosition,
    ErrorCode, errorCode, ObjectId, playerId, PartyPosition, position)
{
    errorCode, playerId, position;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, onSwitchRaidPartyGroupPosition,
    ErrorCode, errorCode, ObjectId, srcId, ObjectId, descId)
{
    errorCode, srcId, descId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evPartyInvited,
    Nickname, nickname)
{
    respondPartyInvitation(nickname, true);
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evPartyInvitationReponded,
    Nickname, nickname, bool, isAnswer)
{
    nickname, isAnswer;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evPartyMeberAdded,
    PartyMemberInfo, memberInfo)
{
    memberInfo;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evPartyMemberInfos,
    PartyMemberInfos, memberInfos, PartyType, partyType)
{
    memberInfos, partyType;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evPartyMemberLeft,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evPartyMemberKicked,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evPartyMasterChanged,
    ObjectId, masterId)
{
    masterId;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evPartyMemberMapMoved,
    ObjectId, memberId, MapCode, worldMapCode, MapCode, subMapCode)
{
    memberId, worldMapCode, subMapCode;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evPartyMemberSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message)
{
    playerId, nickname, message;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evPartyTypeChanged,
    PartyType, partyType)
{
    partyType;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evRaidPartyGroupPositionMoved,
    ObjectId, playerId, PartyPosition, position)
{
    position, playerId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evRaidPartyGroupPositionSwitched,
    ObjectId, srcId, ObjectId, descId)
{
    srcId, descId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evPartyGroupPositionInitialized,
    PartyPositionMap, positionMap)
{
    positionMap;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evWaypointSet,
    ObjectId, playerId, Waypoint, waypoint)
{
    playerId, waypoint;
}


// = WhisperRpc overriding

FORWARD_SRPC_METHOD_2(BotCommunity, whisper,
    Nickname, nickname, ChatMessage, message);


RECEIVE_SRPC_METHOD_3(BotCommunity, onWhisper,
    ErrorCode, errorCode, Nickname, nickname, ChatMessage, message)
{
    errorCode, nickname, message;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evWhispered,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message)
{
    playerId, nickname, message;
}


FORWARD_SRPC_METHOD_2(BotCommunity, createGuild,
    GuildName, guildName, GuildMarkCode, guildMarkCode);


FORWARD_SRPC_METHOD_1(BotCommunity, inviteGuild,
    Nickname, nickname);

// = CommunityGuildRpc overriding

FORWARD_SRPC_METHOD_2(BotCommunity, respondGuildInvitation,
    Nickname, nickname, bool, isAnswer);


FORWARD_SRPC_METHOD_1(BotCommunity, sayGuildMember,
    ChatMessage, message);


FORWARD_SRPC_METHOD_1(BotCommunity, kickGuildMember, 
    ObjectId, playerId);


FORWARD_SRPC_METHOD_2(BotCommunity, addGuildRelationship,
    GuildId, targetGuildId, GuildRelatioshipType, type);


FORWARD_SRPC_METHOD_1(BotCommunity, removeGuildRelationship,
    GuildId, targetGuildId);


FORWARD_SRPC_METHOD_1(BotCommunity, searchGuildInfo,
    GuildName, searchGuildName);


FORWARD_SRPC_METHOD_1(BotCommunity, signupGuild,
    GuildId, guildId);


FORWARD_SRPC_METHOD_0(BotCommunity, queryGuildApplicants);


FORWARD_SRPC_METHOD_2(BotCommunity, confirmGuildApplicant,
    ObjectId, playerId, bool, isAccept);


FORWARD_SRPC_METHOD_1(BotCommunity, queryGuildMember,
    GuildId, guildId);


FORWARD_SRPC_METHOD_0(BotCommunity, queryMyGuildApplicantId);


FORWARD_SRPC_METHOD_1(BotCommunity, queryDetailSearchGuildInfo,
    GuildId, guildId);


FORWARD_SRPC_METHOD_0(BotCommunity, cancelGuildApplicant);


FORWARD_SRPC_METHOD_1(BotCommunity, modifyGuildIntroduction,
    GuildIntroduction, introduction);


FORWARD_SRPC_METHOD_1(BotCommunity, modifyGuildNotice,
    GuildNotice, notice);


FORWARD_SRPC_METHOD_0(BotCommunity, queryMyGuildInfo);


FORWARD_SRPC_METHOD_0(BotCommunity, queryMyGuildSkillInfo);


FORWARD_SRPC_METHOD_0(BotCommunity, queryMyGuildLevelInfo);


FORWARD_SRPC_METHOD_2(BotCommunity, queryGuildEventLogs, 
    uint8_t, beginLogIndex, uint8_t, logCount);


FORWARD_SRPC_METHOD_1(BotCommunity, addGuildRank, 
    GuildRankName, rankName);


FORWARD_SRPC_METHOD_1(BotCommunity, deleteGuildRank, 
    GuildRankId, rankId);


FORWARD_SRPC_METHOD_2(BotCommunity, swapGuildRank,
    GuildRankId, rankId1, GuildRankId, rankId2);


FORWARD_SRPC_METHOD_2(BotCommunity, setGuildRankName, 
    GuildRankId, rankId, GuildRankName, rankName);


FORWARD_SRPC_METHOD_3(BotCommunity, updateGuildRankRights, 
    GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay);


FORWARD_SRPC_METHOD_4(BotCommunity, updateGuildBankRights,
    GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay);


FORWARD_SRPC_METHOD_2(BotCommunity, updateGuildRank,
    ObjectId, memberId, GuildRankId, rankId);


RECEIVE_SRPC_METHOD_2(BotCommunity, onInviteGuild,
    ErrorCode, errorCode, Nickname, nickname)
{
    errorCode, nickname;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onLeaveGuild,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onCreateGuild,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onRespondGuildInvitation,
    ErrorCode, errorCode, GuildId, guildId)
{
    errorCode, guildId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onKickGuildMember, 
    ErrorCode, errorCode, ObjectId, playerId)
{
    errorCode, playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onAddGuildRelationship,
    ErrorCode, errorCode)
{
    errorCode; 
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onRemoveGuildRelationship,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onSearchGuildInfo,
    ErrorCode, errorCode, SearchGuildInfos, searchGuildInfos)
{
    errorCode, searchGuildInfos;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onSignupGuild,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onGuildApplicants,
    ErrorCode, errorCode, GuildApplicantInfoMap, guildApplicantInfoMap)
{
    errorCode, guildApplicantInfoMap;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onConfirmGuildApplicant,
    ErrorCode, errorCode, ObjectId, playerId)
{
    errorCode, playerId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onGuildMemberInfos,
    ErrorCode, errorCode, GuildMemberInfoMap, guildMemberInfoMap)
{
    errorCode, guildMemberInfoMap;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onMyGuildApplicantId,
    GuildId, guildId)
{
    guildId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onDetailSearchGuildInfo,
    ErrorCode, errorCode, DetailSearchGuildInfo, info)
{
    errorCode, info;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onCancelGuildApplicant,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onModifyGuildIntroduction,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onModifyGuildNotice,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_5(BotCommunity, onGuildEventLogs, 
    ErrorCode, errorCode, uint8_t, pageIndex,
    uint8_t, pageCount, uint8_t, totalLogCount, GuildEventLogInfoList, logList)
{
    errorCode, pageIndex, pageCount, totalLogCount, logList;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onAddGuildRank, 
    ErrorCode, errorCode, GuildRankInfo, rankInfo)
{
    errorCode, rankInfo;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onDeleteGuildRank,
    ErrorCode, errorCode, GuildRankId, rankId)
{
    errorCode, rankId;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, onSwapGuildRank, 
    ErrorCode, errorCode, GuildRankId, rankId1, GuildRankId, rankId2)
{
    errorCode, rankId1, rankId2;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, onSetGuildRankName, 
    ErrorCode, errorCode, GuildRankId, rankId, GuildRankName, rankName)
{
    errorCode, rankId, rankName;
}


RECEIVE_SRPC_METHOD_4(BotCommunity, onUpdateGuildRankRights, 
    ErrorCode, errorCode, GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay)
{
    errorCode, rankId, rights, goldWithdrawalPerDay;
}


RECEIVE_SRPC_METHOD_5(BotCommunity, onUpdateGuildBankRights, 
    ErrorCode, errorCode, GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay)
{
    errorCode, rankId, vaultId, rights, itemWithdrawalPerDay;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, onUpdateGuildRank, 
    ErrorCode, errorCode, ObjectId, memberId, GuildRankId, rankId)
{
    errorCode, memberId, rankId;
}


RECEIVE_SRPC_METHOD_7(BotCommunity, evGuildInfo,
    BaseGuildInfo, guildInfo, GuildMemberInfoMap, guildMembers,
    GuildRelationshipInfoMap, relaytionMap,
    GuildIntroduction, introduction, GuildNotice, guildNotice,
    GuildRankInfos, rankInfos, GuildLevelInfo, guildLevelInfo)
{
    guildInfo, guildMembers, relaytionMap, introduction, guildNotice, rankInfos, guildLevelInfo;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildSkillInfos,
    GuildId, guildId, SkillCodes, guildSkills)
{
    guildId, guildSkills;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildLevelInfo,
    GuildId, guildId, GuildLevelInfo, guildLevelInfo)
{
    guildId, guildLevelInfo;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildInvited,
    Nickname, nickname)
{
    nickname;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildInvitationResponded,
    Nickname, nickname, bool, isAnswer)
{
    nickname, isAnswer;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evGuildMemberSaid,
    ObjectId, playerId, Nickname, nickname, ChatMessage, message)
{
    playerId, nickname, message;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildMemberJoined,
    GuildMemberInfo, guildMemberInfo)
{
    guildMemberInfo;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildMemberLeft,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildMemberKicked,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildMemberOnline,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildMemberOffline,
    ObjectId, playerId)
{
    playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildRelationshipAdded,
    GuildRelationshipInfo, relationshipInfo)
{
    relationshipInfo;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildRelationshipRemoved,
    GuildId, targetGuildId)
{
    targetGuildId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildMemberPositionChanged,
	ObjectId, playerId, GuildMemberPosition, position)
{
	playerId, position;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildIntroductionModified,
    GuildIntroduction, introduction)
{
    introduction;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildNoticeModified,
    GuildNotice, notice)
{
    notice;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildSignUpResponded,
    GuildName, guildName, bool, isAccept)
{
    guildName, isAccept;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildMasterChanged,
    ObjectId, masterId)
{
    masterId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildRankAdded,
    GuildRankInfo, rankInfo)
{
    rankInfo;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildRankDeleted,
    GuildRankId, rankId)
{
    rankId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildRankSwapped,
    GuildRankId, rankId1, GuildRankId, rankId2)
{
    rankId1, rankId2;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildRankNameUpdated,
    GuildRankId, rankId, GuildRankName, rankName)
{
    rankId, rankName;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evGuildRankRightsUpdated,
    GuildRankId, rankId, uint32_t, rights, uint32_t, goldWithdrawalPerDay)
{
    rankId, rights, goldWithdrawalPerDay;
}


RECEIVE_SRPC_METHOD_4(BotCommunity, evGuildBankRightsUpdated,
    GuildRankId, rankId, VaultId, vaultId, uint32_t, rights, uint32_t, itemWithdrawalPerDay)
{
    rankId, vaultId, rights, itemWithdrawalPerDay;
}



RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildMemberRankUpdated,
    ObjectId, memberId, GuildRankId, rankId)
{
    memberId, rankId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildMemberLevelUpdated,
    ObjectId, memberId, CreatureLevel, level)
{
    memberId, level;
}

// = CommunityGuildInventoryRpc overriding

FORWARD_SRPC_METHOD_0(BotCommunity, queryGuildBankInfo);


FORWARD_SRPC_METHOD_1(BotCommunity, openGuildInventory,
    VaultId, vaultId);


FORWARD_SRPC_METHOD_0(BotCommunity, closeGuildInventory);


FORWARD_SRPC_METHOD_3(BotCommunity, moveGuildInventoryItem,
    VaultId, vaultId, ObjectId, itemId, SlotId, slotId);


FORWARD_SRPC_METHOD_3(BotCommunity, switchGuildInventoryItem,
    VaultId, vaultId, ObjectId, itemId1, ObjectId, itemId2);


FORWARD_SRPC_METHOD_2(BotCommunity, queryGuildGameMoneyEventLogs, 
    uint8_t, beginLogIndex, uint8_t, logCount);


FORWARD_SRPC_METHOD_2(BotCommunity, queryGuildBankEventLogs, 
    uint8_t, beginLogIndex, uint8_t, logCount);


FORWARD_SRPC_METHOD_2(BotCommunity, updateGuildVaultName,
    VaultId, vaultId, VaultName, name);


RECEIVE_SRPC_METHOD_3(BotCommunity, onGuildBankInfo,
    ErrorCode, errorCode, BaseVaultInfos, infos, GameMoney, gameMoney)
{
    errorCode, infos, gameMoney;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, onOpenGuildInventory,
    ErrorCode, errorCode, VaultId, vaultId, InventoryInfo, inventoryInfo)
{
    errorCode, vaultId, inventoryInfo;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onMoveGuildInventoryItem,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onSwitchGuildInventoryItem,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_5(BotCommunity, onGuildGameMoneyEventLogs, 
    ErrorCode, errorCode, uint8_t, pageIndex,
    uint8_t, pageCount, uint8_t, totalLogCount, GuildGameMoneyEventLogInfoList, logList)
{
    errorCode, pageIndex, pageCount, totalLogCount, logList;
}


RECEIVE_SRPC_METHOD_5(BotCommunity, onGuildBankEventLogs, 
    ErrorCode, errorCode, uint8_t, pageIndex, 
    uint8_t, pageCount, uint8_t, totalLogCount, GuildBankEventLogInfoList, bankLogList)
{
    errorCode, pageIndex, pageCount, totalLogCount, bankLogList;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onUpdateGuildVaultName,
    ErrorCode, errorCode)
{
    errorCode;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildVaultAdded,
    BaseVaultInfo, vaultInfo)
{
    vaultInfo;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evGuildInventoryItemCountUpdated,
    VaultId, vaultId, ObjectId, itemId, uint8_t, ItemCount)
{
    vaultId, itemId, ItemCount;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildInventoryItemAdded,
    VaultId, vaultId, ItemInfo, itemInfo)
{
    vaultId, itemInfo;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildInventoryItemRemoved,
    VaultId, vaultId, ObjectId, itemId)
{
    vaultId, itemId;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evGuildInventoryItemMoved,
    VaultId, vaultId, ObjectId, itemId, SlotId, slotId)
{
    vaultId, itemId, slotId;
}


RECEIVE_SRPC_METHOD_3(BotCommunity, evGuildInventoryItemSwitched,
    VaultId, vaultId, ObjectId, itemId1, ObjectId, itemId2)
{
    vaultId, itemId1, itemId2;
}

RECEIVE_SRPC_METHOD_1(BotCommunity, evGuildGameMoneyUpdated,
    GameMoney, currentGameMoney)
{
    currentGameMoney;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evGuildVaultNameUpdated,
    VaultId, vaultId, VaultName, name)
{
    vaultId, name;
}

// = CommunityDominionRpc overriding

FORWARD_SRPC_METHOD_0(BotCommunity, queryWorldDominion);


FORWARD_SRPC_METHOD_1(BotCommunity, queryZoneDominion,
	MapCode, worldMapCode);


RECEIVE_SRPC_METHOD_1(BotCommunity, onWorldDominion,
	DominionWorldInfoMap, infoMap)
{
	infoMap;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, onZoneDominion,
	BuildingDominionInfos, infos)
{
	infos;
}

// = CommunityMessengerRpc overriding

FORWARD_SRPC_METHOD_1(BotCommunity, requestBuddy,
	Nickname, nickname);


FORWARD_SRPC_METHOD_2(BotCommunity, repsonseBuddy,
	Nickname, requester, bool, isAnswer);


FORWARD_SRPC_METHOD_1(BotCommunity, removeBuddy,
	ObjectId, playerId);


FORWARD_SRPC_METHOD_1(BotCommunity, addBlock,
    Nickname, nickname);

FORWARD_SRPC_METHOD_1(BotCommunity, removeBlock,
    ObjectId, playerId);


RECEIVE_SRPC_METHOD_2(BotCommunity, onRequestBuddy,
	ErrorCode, errorCode, Nickname, nickname)
{
	errorCode, nickname;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onRepsonseBuddy,
	ErrorCode, errorCode, BuddyInfo, buddyInfo)
{
	errorCode, buddyInfo;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evBuddyInfos,
	BuddyInfos, buddyInfos, BlockInfos, blockInfos)
{
	buddyInfos, blockInfos;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onRemoveBuddy,
	ErrorCode, errorCode, ObjectId, playerId)
{
	errorCode, playerId;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onAddBlock,
    ErrorCode, errorCode, BlockInfo, blockInfo)
{
    errorCode, blockInfo;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, onRemoveBlock,
    ErrorCode, errorCode, ObjectId, playerId)
{
    errorCode, playerId;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evBuddyRequested,
	Nickname, nickname)
{
	nickname;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evBuddyAdded,
	BuddyInfo, buddy)
{
	buddy;
}


RECEIVE_SRPC_METHOD_2(BotCommunity, evBuddyOnlineStateChanged,
	ObjectId, playerId, bool, isOnline)
{
	playerId, isOnline;
}


RECEIVE_SRPC_METHOD_1(BotCommunity, evBuddyRemoved,
	ObjectId, playerId)
{
	playerId;
}
}} // namespace gideon { namespace bot {
