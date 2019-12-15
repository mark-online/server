#include "ZoneServerPCH.h"
#include "PlayerCharacterTitleController.h"
#include "../EntityController.h"
#include "../../model/gameobject/Entity.h"
#include "../../model/gameobject/EntityEvent.h"
#include "../../model/gameobject/ability/CharacterTitleable.h"
#include "../../model/gameobject/ability/Knowable.h"
#include "../../model/gameobject/ability/Rewardable.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <sne/database/DatabaseManager.h>
#include <sne/sgp/session/extension/RpcingExtension.h>
#include <sne/server/utility/Profiler.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/memory/MemoryPoolMixin.h>

typedef sne::database::Guard<gideon::serverbase::ProxyGameDatabase> DatabaseGuard;

namespace gideon { namespace zoneserver { namespace gc {

namespace {

/**
 * @class CharacterTitleSelectedEvent
 */
class CharacterTitleSelectedEvent : public go::EntityEvent,
    public sne::core::ThreadSafeMemoryPoolMixin<CharacterTitleSelectedEvent>
{
public:
    CharacterTitleSelectedEvent(ObjectId playerId, CharacterTitleCode titleCode) :
        playerId_(playerId),
        titleCode_(titleCode) {}

private:
    virtual void call(go::Entity& entity) {
        CharacterTitleCallback* callback =
            entity.getController().queryCharacterTitleCallback();
        if (callback != nullptr) {
            callback->characterTitleSelected(playerId_, titleCode_);
        }
    }

private:
    ObjectId playerId_;
    CharacterTitleCode titleCode_;
};


}

IMPLEMENT_SRPC_EVENT_DISPATCHER(PlayerCharacterTitleController);

PlayerCharacterTitleController::PlayerCharacterTitleController(go::Entity* owner) :
    Controller(owner),
    accountId_(owner->getAccountId()),
    characterId_(owner->getObjectId())
{
}


void PlayerCharacterTitleController::registerRpc(sne::sgp::RpcingExtension& extension)
{
    extension.registerRpcForwarder(*this);
    extension.registerRpcReceiver(*this);
}


void PlayerCharacterTitleController::unregisterRpc(sne::sgp::RpcingExtension& extension)
{
    extension.unregisterRpcForwarder(*this);
    extension.unregisterRpcReceiver(*this);
}


void PlayerCharacterTitleController::characterTitleAdded(CharacterTitleCode titleCode)
{
    const ErrorCode errorCode = getOwner().queryRewardable()->addCharacterTitle(titleCode);
    if (isSucceeded(errorCode)) {
        evCharacterTitleAdded(titleCode);
    }
}


void PlayerCharacterTitleController::characterTitleSelected(ObjectId playerId, CharacterTitleCode titleCode)
{
    evCharacterTitleSelected(playerId, titleCode);
}

// = sne::srpc::RpcForwarder overriding

void PlayerCharacterTitleController::onForwarding(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerCharacterTitleController::onForwarding(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}

// = sne::srpc::RpcReceiver overriding

void PlayerCharacterTitleController::onReceiving(const sne::srpc::RRpcId& rpcId)
{
    SNE_LOG_DEBUG("PlayerCharacterTitleController::onReceiving(%u,%s)",
        rpcId.get(), rpcId.getMethodName());
}


RECEIVE_SRPC_METHOD_0(PlayerCharacterTitleController, queryCharacterTitles)
{
    DatabaseGuard db(SNE_DATABASE_MANAGER);
    db->asyncQueryCharacterTitles(accountId_, characterId_);
}


RECEIVE_SRPC_METHOD_1(PlayerCharacterTitleController, selectCharacterTitle,
    CharacterTitleCode, selectTitleCode)
{
    sne::server::Profiler profiler(__FUNCTION__);

    go::Entity& owner = getOwner();
    if (! owner.isValid()) {
        return;
    }

    const ErrorCode errorCode = owner.queryCharacterTitleable()->selectCharacterTitle(selectTitleCode);
    if (isSucceeded(errorCode)) {
        auto event = std::make_shared<CharacterTitleSelectedEvent>(owner.getObjectId(), selectTitleCode);
        owner.queryKnowable()->broadcast(event, true);
    }
    onSelectCharacterTitle(errorCode, selectTitleCode);
}


FORWARD_SRPC_METHOD_2(PlayerCharacterTitleController, onQueryCharacterTitles,
    ErrorCode, errorCode, CharacterTitleCodeSet, titleCodeSet);


FORWARD_SRPC_METHOD_2(PlayerCharacterTitleController, onSelectCharacterTitle,
    ErrorCode, errorCode, CharacterTitleCode, selectTitleCode);


FORWARD_SRPC_METHOD_1(PlayerCharacterTitleController, evCharacterTitleAdded,
    CharacterTitleCode, titleCode);


FORWARD_SRPC_METHOD_2(PlayerCharacterTitleController, evCharacterTitleSelected,
    ObjectId, playerId, CharacterTitleCode, titleCode);

}}}  // namespace gideon { namespace zoneserver { namespace gc {
