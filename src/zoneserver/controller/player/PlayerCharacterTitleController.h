#pragma once

#include "../../zoneserver_export.h"
#include "../Controller.h"
#include "../callback/CharacterTitleCallback.h"
#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/rpc/player/CharacterTitleRpc.h>
#include <sne/srpc/RpcForwarder.h>
#include <sne/srpc/RpcReceiver.h>

namespace sne { namespace sgp {
class RpcingExtension;
}} // namespace sne { namespace sgp {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class PlayerCharacterTitleController
 * 업적 담당
 */
class ZoneServer_Export PlayerCharacterTitleController : public Controller,
    public rpc::CharacterTitleRpc,           
    public CharacterTitleCallback,
    private sne::srpc::RpcForwarder,
    private sne::srpc::RpcReceiver
{
    DECLARE_SRPC_EVENT_DISPATCHER(PlayerCharacterTitleController);
public:
    PlayerCharacterTitleController(go::Entity* owner);

public:
    virtual void initialize() {}
    virtual void finalize() {}

private:
    virtual void characterTitleAdded(CharacterTitleCode titleCode);
    virtual void characterTitleSelected(ObjectId playerId, CharacterTitleCode titleCode);

public:
    void registerRpc(sne::sgp::RpcingExtension& extension);
    void unregisterRpc(sne::sgp::RpcingExtension& extension);


private:
    // = sne::srpc::RpcForwarder overriding
    virtual void onForwarding(const sne::srpc::RRpcId& rpcId);

    // = sne::srpc::RpcReceiver overriding
    virtual void onReceiving(const sne::srpc::RRpcId& rpcId);

public:
    OVERRIDE_SRPC_METHOD_0(queryCharacterTitles);
    OVERRIDE_SRPC_METHOD_1(selectCharacterTitle,
        CharacterTitleCode, selectTitleCode);

    OVERRIDE_SRPC_METHOD_2(onQueryCharacterTitles,
        ErrorCode, errorCode, CharacterTitleCodeSet, titleCodeSet);
    OVERRIDE_SRPC_METHOD_2(onSelectCharacterTitle,
        ErrorCode, errorCode, CharacterTitleCode, selectTitleCode);

    OVERRIDE_SRPC_METHOD_1(evCharacterTitleAdded,
        CharacterTitleCode, titleCode);    
    OVERRIDE_SRPC_METHOD_2(evCharacterTitleSelected,
        ObjectId, playerId, CharacterTitleCode, titleCode);
private:
    AccountId accountId_;
    ObjectId characterId_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
