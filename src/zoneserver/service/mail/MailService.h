#pragma once

#include "../../zoneserver_export.h"
#include <gideon/serverbase/database/callback/GameMailCallback.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/MailInfo.h>
#include <sne/core/utility/Singleton.h>

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} // namespace go


/**
 * @class MailService
 *
 * 메일 서비스
 */
class ZoneServer_Export MailService : public boost::noncopyable,
    public serverbase::GameMailCallback
{
    SNE_DECLARE_SINGLETON(MailService);
    typedef std::mutex LockType;
    typedef sne::core::HashMap<ObjectId, MailInfos> PlayerMailInfos;
    
public:
    MailService() {}
	~MailService() {}

    bool initialize();

    void releaseSyncMail(ObjectId playerId);

    ErrorCode sendMail(go::Entity& sender, const SendMailInfo& mailInfo);
    void sendMailFromAuction(MailType mailType, DataCode npcCode, ObjectId receiverId,
        const InventoryInfo& inventoryInfo, GameMoney gameMoney);
    void sendMailFromAchievement(ObjectId receiverId, DataCode achievementCode, const InventoryInfo& inventoryInfo);

    ErrorCode syncMail(ObjectId playerId);
    ErrorCode queryMailTitle(MailTitleInfo& mailInfo, ObjectId playerId, MailId mailId);
    ErrorCode queryMailContent(MailContentInfo& mailInfo, ObjectId playerId, MailId mailId);
    ErrorCode aquireMailItems(go::Entity& onwer, MailId mailId);
    ErrorCode deleteMail(ObjectId playerId, MailId mailId);
    
private:
    // = GameMailCallback overriding
    virtual void onSyncMail(ObjectId playerId, const MailInfos& mails);
    virtual void onReceive(ObjectId playerId, const MailInfo& mail);
    virtual void onNotReadMail(ObjectId playerId);

private:
    void queryMails_i(MailTitleInfos& mailInfos, ObjectId playerId);
    
    bool isWaitingPlayer(ObjectId playerId) const;
    bool isSyncPlayer(ObjectId playerId) const;
    
    MailInfos* getMailInfos(ObjectId playId);
    const MailInfos* getMailInfos(ObjectId playId) const;


    MailInfo& getMailInfo(ObjectId playId, MailId mailId);
    const MailInfo& getMailInfo(ObjectId playId, MailId mailId) const;

private:
    mutable LockType lock_;
    ObjectIdSet waitingPlayers_;
    PlayerMailInfos mails_;
};

}} // namespace gideon { namespace zoneserver {

#define MAIL_SERVICE gideon::zoneserver::MailService::instance()
