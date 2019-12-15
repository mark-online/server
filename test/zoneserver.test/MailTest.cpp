#include "ZoneServerTestPCH.h"
#include "GameTestFixture.h"
#include "MockGameTimer.h"
#include "MockPlayerMailController.h"
#include "MockPlayerItemController.h"
#include "ZoneServer/model/gameobject/Player.h"
#include <gideon/servertest/datatable/DataCodes.h>

/**
* @class MailTest
*
* 메일 관련 테스트
*/
class MailTest : public GameTestFixture
{
};


TEST_F(MailTest, testNotSyncSendMail)
{
    SendMailInfo sendMailInfo;
    sendMailInfo.nickname_ = L"test1";
    sendMailInfo.title_ = L"";
    sendMailInfo.mailBody_ = L"";
    playerMailController1_->sendMail(sendMailInfo);

    ASSERT_EQ(ecMailNotSendSelf, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(0, playerMailController2_->getCallCount("evMailReceived"));

    sendMailInfo.nickname_ = L"test2";
    playerMailController1_->sendMail(sendMailInfo);
    ASSERT_EQ(ecMailInvalidMail, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(0, playerMailController2_->getCallCount("evMailReceived"));

    sendMailInfo.title_ = L"test2";
    playerMailController1_->sendMail(sendMailInfo);
    ASSERT_EQ(ecOk, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(1, playerMailController2_->getCallCount("evMailReceived"));

    sendMailInfo.nickname_ = L"zzztest2";
    playerMailController1_->sendMail(sendMailInfo);
    ASSERT_EQ(ecOk, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(1, playerMailController1_->getCallCount("evMailReceived"));
    ASSERT_EQ(1, playerMailController2_->getCallCount("evMailReceived"));
}


TEST_F(MailTest, testNotSyncMail)
{
    playerMailController1_->synchronizeMail();
    ASSERT_EQ(ecOk, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(1, playerMailController1_->getCallCount("evMailSynchronized"));

    playerMailController1_->synchronizeMail();
    ASSERT_EQ(ecMailAlreadySync, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(1, playerMailController1_->getCallCount("evMailSynchronized"));

}


TEST_F(MailTest, testSyncSendMail)
{
    playerMailController1_->synchronizeMail();
    playerMailController2_->synchronizeMail();
    SendMailInfo sendMailInfo;
    sendMailInfo.nickname_ = L"test1";
    sendMailInfo.title_ = L"";
    sendMailInfo.mailBody_ = L"";
    playerMailController1_->sendMail(sendMailInfo);

    ASSERT_EQ(ecMailNotSendSelf, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(0, playerMailController2_->getCallCount("evMailReceived"));

    sendMailInfo.nickname_ = L"test2";
    playerMailController1_->sendMail(sendMailInfo);
    ASSERT_EQ(ecMailInvalidMail, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(0, playerMailController2_->getCallCount("evMailReceived"));

    sendMailInfo.title_ = L"test2";
    playerMailController1_->sendMail(sendMailInfo);
    ASSERT_EQ(ecOk, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(1, playerMailController2_->getCallCount("evMailReceived"));

    sendMailInfo.nickname_ = L"zzztest2";
    playerMailController1_->sendMail(sendMailInfo);
    ASSERT_EQ(ecOk, playerMailController1_->lastErrorCode_);
    ASSERT_EQ(1, playerMailController1_->getCallCount("evMailReceived"));
    ASSERT_EQ(1, playerMailController2_->getCallCount("evMailReceived"));

}


