#include "ZoneServerPCH.h"
#include "ShardService.h"
#include "../../ZoneService.h"
#include "../../s2s/ZoneLoginServerProxy.h"
#include "../../s2s/ZoneCommunityServerProxy.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../item/ItemIdGenerator.h"
#include <gideon/serverbase/database/ProxyGameDatabase.h>
#include <gideon/serverbase/database/proxy/RequestFuture.h>
#include <gideon/3d/3d.h>
#include <gideon/cs/datatable/CharacterDefaultItemTable.h>
#include <sne/database/DatabaseManager.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class GetFullUserInfoFutureObserverForLoginServer
 */
class GetFullUserInfoFutureObserverForLoginServer : public sne::base::FutureObserver,
    public sne::core::ThreadSafeMemoryPoolMixin<GetFullUserInfoFutureObserverForLoginServer>
{
public:
    GetFullUserInfoFutureObserverForLoginServer() {}

private:
    virtual void update(const sne::base::Future& future) {
        const serverbase::GetFullUserInfoRequestFuture& requestFuture =
            static_cast<const serverbase::GetFullUserInfoRequestFuture&>(future);
        ZONE_SERVICE->getLoginServerProxy().z2l_onGetFullUserInfo(ecOk,
            requestFuture.userInfo_);
    }

    virtual void deleteFutureObserver() {
        delete this;
    }
};


/**
 * @class CreateCharacterFutureObserverForLoginServer
 */
class CreateCharacterFutureObserverForLoginServer : public sne::base::FutureObserver,
    public sne::core::ThreadSafeMemoryPoolMixin<CreateCharacterFutureObserverForLoginServer>
{
public:
    CreateCharacterFutureObserverForLoginServer(AccountId accountId) :
        accountId_(accountId) {}

private:
    virtual void update(const sne::base::Future& future) {
        const serverbase::CreateCharacterRequestFuture& requestFuture =
            static_cast<const serverbase::CreateCharacterRequestFuture&>(future);

        ZONE_SERVICE->getLoginServerProxy().z2l_onCreateCharacter(
            requestFuture.errorCode_, accountId_, requestFuture.characterInfo_);
    }

    virtual void deleteFutureObserver() {
        delete this;
    }

private:
    AccountId accountId_;
};


/**
 * @class DeleteCharacterFutureObserverForLoginServer
 */
class DeleteCharacterFutureObserverForLoginServer : public sne::base::FutureObserver,
    public sne::core::ThreadSafeMemoryPoolMixin<DeleteCharacterFutureObserverForLoginServer>
{
public:
    DeleteCharacterFutureObserverForLoginServer(AccountId accountId) :
        accountId_(accountId) {}

private:
    virtual void update(const sne::base::Future& future) {
        const serverbase::DeleteCharacterRequestFuture& requestFuture =
            static_cast<const serverbase::DeleteCharacterRequestFuture&>(future);

        ZONE_SERVICE->getLoginServerProxy().z2l_onDeleteCharacter(ecOk, accountId_,
            requestFuture.characterId_);
		// 현재는 케릭터 삭제시 길드에만 영향이 있다
		// 추후에 변경될 코드일수 있음
		if (isValidGuildId(requestFuture.guildId_)) {
			ZONE_SERVICE->getCommunityServerProxy().z2m_deleteCharacter(requestFuture.characterId_,
				requestFuture.guildId_);
		}
    }

    virtual void deleteFutureObserver() {
        delete this;
    }

private:
    AccountId accountId_;
};


/**
 * @class CheckDuplicateNicknameFutureObserverForLoginServer
 */
class CheckDuplicateNicknameFutureObserverForLoginServer : public sne::base::FutureObserver,
    public sne::core::ThreadSafeMemoryPoolMixin<CheckDuplicateNicknameFutureObserverForLoginServer>
{
public:
    CheckDuplicateNicknameFutureObserverForLoginServer(AccountId accountId,
        const Nickname& nickname) :
        accountId_(accountId),
        nickname_(nickname) {}

private:
    virtual void update(const sne::base::Future& future) {
        const serverbase::CheckDuplicateNicknameRequestFuture& requestFuture =
            static_cast<const serverbase::CheckDuplicateNicknameRequestFuture&>(future);

        ZONE_SERVICE->getLoginServerProxy().z2l_onCheckDuplicateNickname(requestFuture.errorCode_, accountId_,
            requestFuture.nickname_);
    }

    virtual void deleteFutureObserver() {
        delete this;
    }

private:
    AccountId accountId_;
    Nickname nickname_;
};


ErrorCode fillDefaultEquipItem(CreateCharacterEquipments& createCharacterEquipments,
    CharacterClass characterClass, EquipCode mainWeaponItem)
{
    if (! (ccFighter <= characterClass && characterClass <= ccRogue)) {
        return ecCharacterInvalidClass;
    }

    const datatable::CharacterDefaultItemInfo* itemInfo = 
        CHARACTER_DEFAULT_ITEM_TABLE->getCharacterDefaultItemInfo(characterClass);
    if (! itemInfo) {
        return ecServerInternalError;
    }

    if (! isValidEquipCode(mainWeaponItem)) {
        return ecCharacterInvalidMainWeapon;
    }

    if (! itemInfo->hasMainWeapon(mainWeaponItem)) {
        return ecCharacterInvalidMainWeapon;
    }

    for (const DataCode defenceItemCode : itemInfo->getDefaultDefenceItems()) {
        if (isValidEquipCode(defenceItemCode)) {
            const EquipPart part = getEquipPart(defenceItemCode);
            createCharacterEquipments[part].equipCode_ = defenceItemCode;
            createCharacterEquipments[part].equipId_ = ITEM_ID_GENERATOR->generateItemId();            
        }        
    }

    const EquipPart mainWeaponPart = getEquipPart(mainWeaponItem);
    createCharacterEquipments[mainWeaponPart].equipCode_ = mainWeaponItem;
    createCharacterEquipments[mainWeaponPart].equipId_ = ITEM_ID_GENERATOR->generateItemId();

    const EquipCode subWeaponCode = itemInfo->getDefaultSubWeaponItem();
    if (isValidEquipCode(subWeaponCode) && mainWeaponPart == epRightHand) {
        const EquipPart subWeaponPart = getEquipPart(subWeaponCode);
        createCharacterEquipments[subWeaponPart].equipCode_ = subWeaponCode;
        createCharacterEquipments[subWeaponPart].equipId_ = ITEM_ID_GENERATOR->generateItemId();
    }

    return ecOk;
}

} // namespace

// = ShardService

void ShardService::fullUserInfoRequested(AccountId accountId)
{
    {
        sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

        sne::base::Future::Ref future = db->asyncGetFullUserInfo(AccountInfo(accountId));
        if (future.get() != nullptr) {
            future->attach(new GetFullUserInfoFutureObserverForLoginServer);
            return;
        }
    }

    ZONE_SERVICE->getLoginServerProxy().z2l_onGetFullUserInfo(ecDatabaseInternalError,
        FullUserInfo(accountId));
}


void ShardService::createCharacterRequested(const CreateCharacterInfo& createCharacterInfo)
{
    CreateCharacterEquipments createCharacterEquipments;
    ErrorCode errorCode =
        fillDefaultEquipItem(createCharacterEquipments, createCharacterInfo.characterClass_, createCharacterInfo.selectMainWeaponCode_);
    if (isFailed(errorCode)) {
        // Log ??
        return;
    }

    const ZoneId zoneId = WORLD->getZoneId();
    WorldPosition position = WORLD->getGlobalWorldMap().getCharacterSpawnPosition();
    position.heading_ = getHeading(Position(), position);
    {
        sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

        sne::base::Future::Ref future = db->asyncCreateCharacter(createCharacterInfo, createCharacterEquipments, zoneId, position);
        if (future.get() != nullptr) {
            future->attach(new CreateCharacterFutureObserverForLoginServer(createCharacterInfo.accountId_));
            return;
        }
    }

    ZONE_SERVICE->getLoginServerProxy().z2l_onCreateCharacter(ecDatabaseInternalError,
        createCharacterInfo.accountId_, FullCharacterInfo());
}


void ShardService::deleteCharacterRequested(AccountId accountId, ObjectId characterId)
{
    {
        sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

        sne::base::Future::Ref future = db->asyncDeleteCharacter(accountId, characterId);;
        if (future.get() != nullptr) {
            future->attach(new DeleteCharacterFutureObserverForLoginServer(accountId));
            return;
        }
    }

    ZONE_SERVICE->getLoginServerProxy().z2l_onDeleteCharacter(ecDatabaseInternalError,
        accountId, characterId);
}


void ShardService::checkDuplicateNicknameRequested(AccountId accountId, const Nickname& nickname)
{
    {
        sne::database::Guard<serverbase::ProxyGameDatabase> db(SNE_DATABASE_MANAGER);

        sne::base::Future::Ref future = db->asyncCheckDuplicateNickname(accountId, nickname);;
        if (future.get() != nullptr) {
            future->attach(new CheckDuplicateNicknameFutureObserverForLoginServer(accountId, nickname));
            return;
        }
    }

    ZONE_SERVICE->getLoginServerProxy().z2l_onCheckDuplicateNickname(ecDatabaseInternalError,
        accountId, nickname);
}

}} // namespace gideon { namespace zoneserver {
