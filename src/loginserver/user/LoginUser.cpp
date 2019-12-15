#include "LoginServerPCH.h"
#include "LoginUser.h"
#include "../c2s/LoginClientSessionCallback.h"

namespace gideon { namespace loginserver {

void LoginUser::disconnect()
{
    if (callback_ != nullptr) {
        callback_->disconnectNow();
    }
}


void LoginUser::expelled(ExpelReason reason)
{
    if (callback_ != nullptr) {
        callback_->expelled(reason);
    }
}


ErrorCode LoginUser::loginZoneServer(sne::server::ServerId zoneServerId, ShardId shardId,
    const Certificate& certificate)
{
    if (isConnectedToZoneServer()) {
        return ecLoginAlreadyLoggedIn;
    }

    const ServerType serverType = stZoneServer;
    const ErrorCode errorCode = checkCertificate(certificate, serverType);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    loggedInZoneServer(zoneServerId, shardId);

    issueCertificate(serverType);
    return ecOk;
}


ErrorCode LoginUser::loginCommunityServer(sne::server::ServerId communityServerId,
    const Certificate& certificate)
{
    if (isConnectedToCommunityServer()) {
        return ecLoginAlreadyLoggedIn;
    }

    const ServerType serverType = stCommunityServer;
    const ErrorCode errorCode = checkCertificate(certificate, serverType);
    if (isFailed(errorCode)) {
        return errorCode;
    }

    loggedInCommunityServer(communityServerId);
    issueCertificate(serverType);
    return ecOk;
}


void LoginUser::userInfoChanged(const FullUserInfo& userInfo)
{
    characterZoneMap_.clear();

    for (const FullCharacterInfos::value_type& value : userInfo.characters_) {
        const ObjectId characterId = value.first;
        const FullCharacterInfo& characterInfo = value.second;
        characterZoneMap_.insert(
            CharacterZoneMap::value_type(characterId, characterInfo.zoneId_));
    }
}

}} // namespace gideon { namespace loginserver {
