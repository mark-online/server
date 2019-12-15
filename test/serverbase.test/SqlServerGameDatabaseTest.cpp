#include "ServerBaseTestPCH.h"
#include <gideon/server/data/CreateCharacterInfo.h>
#include <gideon/servertest/datatable/MockEquipTable.h>
#include <gideon/servertest/datatable/DataCodes.h>
#include <gideon/serverbase/database/GameDatabase.h>
#include <gideon/serverbase/database/GideonDatabaseFactory.h>
#include <sne/database/DatabaseManagerFactory.h>
#include <sne/database/DatabaseManager.h>
#include <sne/database/ado/AdoDatabase.h>
#include <sne/server/config/ConfigReader.h>
#include <sne/core/utility/Win32Util.h>
#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4127 4512)
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#  pragma warning (pop)
#endif

using namespace sne;
using namespace gideon;
using namespace gideon::serverbase;

const char connectionString[] =
    "Provider=sqloledb;"
    "Data Source=localhost,1433;"
    //"Data Source=localhost\\SQLEXPRESS;"
    "Initial Catalog=Gideon_game_test;"
    "User Id=sa;"
    "Password=1234;" 
    "Trusted_Connection=False;";

/**
 * @class MockConfigReader
 */
class MockConfigReader : public server::ConfigReader
{
private:
    virtual bool open(const std::string& filename) {
        filename;
        return true;
    }

    virtual std::string getString(const char* sectionName,
        const char* keyName) {
        if (std::string("database") == sectionName) {
            if (std::string("connection-type") == keyName) {
                return "direct";
            }
            if (std::string("connection-count") == keyName) {
                return "1";
            }
            if (std::string("connection-timeout") == keyName) {
                return "3";
            }
            if (std::string("connection-string") == keyName) {
                return connectionString;
            }
        }
        return "";
    }
};

/**
* @class SqlServerGameDatabaseTest
*
* SqlServer GameDatabase 테스트
* - database/migrate/sql/test/create_database_and_user.sql을 실행해야 한다
*/
class SqlServerGameDatabaseTest : public testing::Test
{ 
public:
    SqlServerGameDatabaseTest() :
        db_(0) {}

    virtual void SetUp();
    virtual void TearDown();

    void addItem(ObjectId itemId, SlotId slotId);
    void addQuestItem(ObjectId objectId, QuestCode questCode);
    void createCharacter(ObjectId& objectId);
    MailInfo createMailInfo(const Nickname& nickname, ObjectId playerId, const MailTitle& title);
private:
    void resetTables();

    
protected:
    core::InitOle* initOle_;

    database::DatabaseManager* gameDatabaseManager_;
    database::AdoDatabase* db_;
public:
    ObjectId accountId_;
    ObjectId characterId_;
};



MailInfo SqlServerGameDatabaseTest::createMailInfo(const Nickname& nickname, ObjectId playerId, const MailTitle& title)
{
    MailInfo mailInfo;

    mailInfo.titleInfo_.title_ = title;
    mailInfo.titleInfo_.hasAttachment_ = true;
    mailInfo.titleInfo_.time_ = getTime();
    mailInfo.titleInfo_.nickname_ = nickname;
    mailInfo.titleInfo_.objectId_ = playerId;
    mailInfo.contentInfo_.gameMoney_ = 100;
    mailInfo.contentInfo_.mailInventory_.addItem(ItemInfo(1234, 1, 4321, 1));
    return mailInfo;
}



void SqlServerGameDatabaseTest::SetUp()
{
    initOle_ = new core::InitOle();

    MockConfigReader configReader;
    base::IoContextTask* nullIoContextTask = nullptr;
    GideonDatabaseFactory databaseFactory(*nullIoContextTask, "zone_server");

    gameDatabaseManager_ =
        database::DatabaseManagerFactory::create(
            configReader, databaseFactory).release();

    ASSERT_TRUE(0 != gameDatabaseManager_);

    db_ = new database::AdoDatabase;
    ASSERT_EQ(true,
        db_->open(connectionString));

    resetTables();
}


void SqlServerGameDatabaseTest::TearDown()
{
    if (gameDatabaseManager_ != nullptr) {
        delete gameDatabaseManager_;
    }

    if (db_ != nullptr) {
        db_->close();
        delete db_;
    }

    delete initOle_;
}


void SqlServerGameDatabaseTest::resetTables()
{
    try {
        db_->execute("DELETE FROM Building_Equip_Item_Socket_Infos");
        db_->execute("DELETE FROM Building_Equip_Item_Infos");
        db_->execute("DELETE FROM Building_Accessory_Item_Infos");
        db_->execute("DELETE FROM Building_Inventories");
        db_->execute("DELETE FROM Buildings");
        db_->execute("DELETE FROM Auction_Accessory_Item_Infos");
        db_->execute("DELETE FROM Auction_Equip_Item_Socket_Infos");
        db_->execute("DELETE FROM Auction_Equip_Item_Infos");
        db_->execute("DELETE FROM Auction_Inventories");
        db_->execute("DELETE FROM Auctions");
        db_->execute("DELETE FROM Mail_Accessory_Item_Infos");
        db_->execute("DELETE FROM Mail_Equip_Item_Scoket_Infos");
        db_->execute("DELETE FROM Mail_Equip_Item_Infos");
        db_->execute("DELETE FROM Mail_Inventories");
        db_->execute("DELETE FROM Mails");

        db_->execute("DELETE FROM Guild_Applicants");
        db_->execute("DELETE FROM Guild_logs");
        db_->execute("DELETE FROM Guild_Bank_Event_logs");
        db_->execute("DELETE FROM Guild_Event_logs");
        db_->execute("DELETE FROM Guild_Game_Money_Event_Logs");
        db_->execute("DELETE FROM Guild_Accessory_Item_Infos");
        db_->execute("DELETE FROM Guild_Equip_Item_Infos");
        db_->execute("DELETE FROM Guild_Equip_Item_Scoket_Infos");
        db_->execute("DELETE FROM Guild_Inventories");
        db_->execute("DELETE FROM Guild_Vaults");
        db_->execute("DELETE FROM Guild_Ranks");
        db_->execute("DELETE FROM Guild_Members");
        db_->execute("DELETE FROM Guild_Inventories");
        db_->execute("DELETE FROM Guilds");
        db_->execute("DELETE FROM Mercenary_Quests");
        db_->execute("DELETE FROM Accept_Repeat_Quest_Missions");
        db_->execute("DELETE FROM Accept_Repeat_Quests");
        db_->execute("DELETE FROM Complete_Repeat_Quests");
        db_->execute("DELETE FROM Accept_Quest_Missions");
        db_->execute("DELETE FROM Accept_Quests");
        db_->execute("DELETE FROM Complete_Quests");
        db_->execute("DELETE FROM GameMonies");
        db_->execute("DELETE FROM ActionBars");
        db_->execute("DELETE FROM Skills");
        db_->execute("DELETE FROM Accessory_Item_Infos");
        db_->execute("DELETE FROM Equip_Item_Scoket_Infos");
        db_->execute("DELETE FROM Equip_Item_Infos");
        db_->execute("DELETE FROM Accessories");
        db_->execute("DELETE FROM Equipments");
        db_->execute("DELETE FROM Inventories");
        db_->execute("DELETE FROM Quest_Inventories");
        db_->execute("DELETE FROM dbo.Character_Appearance_Infos");
        db_->execute("DELETE FROM dbo.Character_Inventory_Infos");
        db_->execute("DELETE FROM Characters");


        {
            database::Guard<GameDatabase> db(gameDatabaseManager_);

            CreateCharacterEquipments equipments;
            equipments[epHelmet].equipCode_ = servertest::defaultHelmetEquipCode;
            equipments[epHelmet].equipId_ = 123456;
            equipments[epRightHand].equipCode_  = servertest::defaultOneHandSwordEquipCode;
            equipments[epRightHand].equipId_  = 123457;
            characterId_ = 1;
            accountId_ = 1;

            ASSERT_EQ(ecOk, db->createCharacter(characterId_, accountId_, L"test1",
                EquipType::etOneHandSword, ccWarrior, stMale, equipments,
                CharacterAppearance(1, 1, 1), 1, WorldPosition()));

            BaseGuildInfo info(2, L"test2", 1);
            ASSERT_EQ(true, db->createGuild(info));
            ASSERT_EQ(true, db->addGuildMember(2, GuildMemberInfo(characterId_, L"test1", gmpLord, false, 0)));
        }

        char buf[255] = {0,};
        sprintf(buf, "INSERT INTO Complete_Quests (character_id, quest_code) "
            "VALUES (%" PRIu64 ", 1)", characterId_);
        db_->execute(buf);
        memset(buf, 0, 255);
        sprintf(buf, "INSERT INTO Accept_Quests (character_id, quest_code) "
            "VALUES (%" PRIu64 ", 2)", characterId_);
        db_->execute(buf);
        memset(buf, 0, 255);
        sprintf(buf, "INSERT INTO dbo.Accept_Quest_Missions(character_id, quest_code, mission_code, goal_code, value) "
            "VALUES (%" PRIu64 ", 2, 1, 0, 1)", characterId_);
        db_->execute(buf);

        char buf2[255] = {0,};
        sprintf(buf2, "INSERT INTO Complete_Repeat_Quests (character_id, quest_code, accept_at) "
            "VALUES (%" PRIu64 ", 1001, GETDATE())", characterId_);
        db_->execute(buf2);
        memset(buf2, 0, 255);
        sprintf(buf2, "INSERT INTO Accept_Repeat_Quests(character_id, quest_code, accept_at) "
            "VALUES (%" PRIu64 ", 1002, GETDATE())", characterId_);
        db_->execute(buf2);
        memset(buf2, 0, 255);
        sprintf(buf2, "INSERT INTO dbo.Accept_Repeat_Quest_Missions(character_id, quest_code, mission_code, goal_code, value) "
            "VALUES (%" PRIu64 ", 1002, 1, 0, 1)", characterId_);
        db_->execute(buf2);
    }
    catch (const database::AdoException& /*e*/) {
        ASSERT_TRUE(false) << db_->getLastError();
    }
}


void SqlServerGameDatabaseTest::addItem(ObjectId objectId, SlotId slotId)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    const ItemInfo itemInfo(servertest::defaultHelmetEquipCode, 1, objectId, slotId);

    ASSERT_EQ(true, db->addInventoryItem(accountId_, characterId_, itemInfo, itPc));
}

void SqlServerGameDatabaseTest::addQuestItem(ObjectId objectId, QuestCode questCode)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    const QuestItemInfo itemInfo(objectId, 1, 0, 1, questCode);

    ASSERT_EQ(true, db->addQuestItem(accountId_, characterId_, itemInfo));
}


void SqlServerGameDatabaseTest::createCharacter(ObjectId& objectId)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    CreateCharacterEquipments equipments;
    equipments[epHelmet].equipCode_ = servertest::defaultHelmetEquipCode;
    equipments[epHelmet].equipId_ = 1000;
    equipments[epRightHand].equipCode_  = servertest::defaultOneHandSwordEquipCode;
    equipments[epRightHand].equipId_  = 1001;
    const CharacterAppearance appearance(1, 1, 1); 
    ASSERT_EQ(ecOk,
        db->createCharacter(objectId, accountId_, L"test1_2",
            EquipType::etOneHandSword, ccWarrior, stFemale, equipments,
            appearance, 1, WorldPosition()));
}


TEST_F(SqlServerGameDatabaseTest, testGetProperties)
{
    server::Properties props;
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->getProperties(props));
    ASSERT_TRUE(0 < props.size());
}


TEST_F(SqlServerGameDatabaseTest, testCreateCharacter)
{
    ObjectId characterId = invalidObjectId;

    database::Guard<GameDatabase> db(gameDatabaseManager_);

    CreateCharacterEquipments equipments;
    equipments[epHelmet].equipCode_ = servertest::defaultHelmetEquipCode;
    equipments[epHelmet].equipId_ = 1000;
    equipments[epRightHand].equipCode_  = servertest::defaultOneHandSwordEquipCode;
    equipments[epRightHand].equipId_  = 1001;

    ASSERT_EQ(ecOk,
        db->createCharacter(characterId, accountId_, L"test1_1", EquipType::etOneHandSword,
            ccWarrior, stFemale, equipments, CharacterAppearance(), 1, WorldPosition()));
    ASSERT_TRUE(isValidObjectId(characterId));

    ASSERT_EQ(ecCharacterDuplicatedNickname,
        db->createCharacter(characterId, accountId_, L"test1_1", EquipType::etOneHandSword,
            ccWarrior, stMale, equipments, CharacterAppearance(), 1, WorldPosition()));
}


TEST_F(SqlServerGameDatabaseTest, testGetCharacters)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    FullUserInfo userInfo;
    ASSERT_EQ(false, db->getFullUserInfo(userInfo, invalidAccountId));

    ASSERT_EQ(true, db->getFullUserInfo(userInfo, accountId_));
    ASSERT_EQ(accountId_, userInfo.accountId_);
    ASSERT_EQ(1, userInfo.characters_.size());

    const FullCharacterInfo* character = userInfo.getCharacter(characterId_);
    ASSERT_TRUE(character != nullptr);
    ASSERT_TRUE(Nickname(L"test1") == character->nickname_);
    ASSERT_EQ(ccWarrior, character->characterClass_);
}


TEST_F(SqlServerGameDatabaseTest, testGetZone)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    //ZoneInfo info;
    //ASSERT_EQ(true, db->getZoneInfo(info, "test"));
    //ASSERT_EQ(1, info.zoneId_);
}


TEST_F(SqlServerGameDatabaseTest, testGetZoneList)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    //ZoneInfos infos;
    //ASSERT_EQ(true, db->getZoneInfoList(infos, 1));
    //ASSERT_EQ(1, infos.size());
}


TEST_F(SqlServerGameDatabaseTest, testGetWorldTime)
{
    // TODO:
}


TEST_F(SqlServerGameDatabaseTest, testGetCharacterInfo)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    FullCharacterInfo characterInfo;
    ASSERT_EQ(false, db->getCharacterInfo(characterInfo, invalidObjectId));

    ASSERT_EQ(true, db->getCharacterInfo(characterInfo, characterId_));
    ASSERT_EQ(characterId_, characterInfo.objectId_);
    ASSERT_TRUE(Nickname(L"test1") == characterInfo.nickname_);
    ASSERT_EQ(ccWarrior, characterInfo.characterClass_);
}


TEST_F(SqlServerGameDatabaseTest, testUpdateCharacterPoints)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->updateCharacterPoints(characterId_,
        Points()));
}


TEST_F(SqlServerGameDatabaseTest, testEquipItem)
{
    addItem(invalidObjectId + 1, 1);
    addItem(invalidObjectId + 2, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->equipItem(accountId_, characterId_,
        invalidObjectId + 1, epInvalid));
}


TEST_F(SqlServerGameDatabaseTest, testUnequipItem)
{
    addItem(invalidObjectId + 1, 1);
    addItem(invalidObjectId + 2, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->equipItem(accountId_, characterId_,
        invalidObjectId + 1, epInvalid));
    ASSERT_EQ(true, db->unequipItem(accountId_, characterId_,
        invalidObjectId + 1, invalidSlotId, epInvalid));
}

TEST_F(SqlServerGameDatabaseTest, testEquipAccessoryItem)
{
    addItem(invalidObjectId + 1, 1);
    addItem(invalidObjectId + 2, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->equipAccessoryItem(accountId_, characterId_,
        invalidObjectId + 1, aiRing1));
}


TEST_F(SqlServerGameDatabaseTest, testUnequipAccessoryItem)
{
    addItem(invalidObjectId + 1, 1);
    addItem(invalidObjectId + 2, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->equipAccessoryItem(accountId_, characterId_,
        invalidObjectId + 1, aiRing1));
    ASSERT_EQ(true, db->unequipAccessoryItem(accountId_, characterId_,
        invalidObjectId + 1, invalidSlotId, aiRing1));
}



TEST_F(SqlServerGameDatabaseTest, testMoveInventoryItem)
{
    addItem(invalidObjectId + 1, 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->moveInventoryItem(invalidObjectId + 1, 3));
}


TEST_F(SqlServerGameDatabaseTest, testSwitchInventoryItem)
{
    addItem(invalidObjectId + 1, 1);
    addItem(invalidObjectId + 2, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->switchInventoryItem(invalidObjectId + 1, 2,
        invalidObjectId + 2, 1));
}


TEST_F(SqlServerGameDatabaseTest, testAddInventoryItem)
{
    // TODO:
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    const ItemInfo itemInfo(servertest::defaultHelmetEquipCode, 1, invalidObjectId + 1, 2);
    
    ASSERT_EQ(true, db->addInventoryItem(accountId_, characterId_, itemInfo, itPc));
}


TEST_F(SqlServerGameDatabaseTest, testRemoveIventoryItem)
{
    addItem(invalidObjectId + 1, 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->removeInventoryItem(invalidObjectId + 1));
}


TEST_F(SqlServerGameDatabaseTest, testUpdateInventoryItemCount)
{
    // TODO:
    addItem(invalidObjectId + 1, 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);    

    ASSERT_EQ(true, db->updateInventoryItemCount(invalidObjectId + 1, 0));
}


TEST_F(SqlServerGameDatabaseTest, testDeleteCharacter)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    GuildId guildId = invalidGuildId;
    ASSERT_EQ(ecOk, db->deleteCharacter(guildId, characterId_));
}


TEST_F(SqlServerGameDatabaseTest, testReplaceInventoryWithEquipItem)
{
    addItem(invalidObjectId + 1, 1);
    addItem(invalidObjectId + 2, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->equipItem(accountId_, characterId_, invalidObjectId + 1, epGaiters));
    ASSERT_EQ(true, db->replaceInventoryWithEquipItem(accountId_, characterId_, invalidObjectId + 1,
        epGaiters, 2, invalidObjectId + 2, epGaiters));
}


TEST_F(SqlServerGameDatabaseTest, testReplaceInventoryWithAccessiryItem)
{
    addItem(invalidObjectId + 1, 1);
    addItem(invalidObjectId + 2, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->equipAccessoryItem(accountId_, characterId_, invalidObjectId + 1, aiRing1));
    ASSERT_EQ(true, db->replaceInventoryWithAccessoryItem(accountId_, characterId_, invalidObjectId + 1,
        aiRing1, 2, invalidObjectId + 2, aiRing1));
}

TEST_F(SqlServerGameDatabaseTest, testGetMaxInventoryId)
{
    const ObjectId minId = getMinZoneObjectId(1);
    ObjectId itemId = 0;
    {
        database::Guard<GameDatabase> db(gameDatabaseManager_);
        ASSERT_EQ(true, db->getMaxInventoryId(itemId, 1));
        ASSERT_EQ(itemId, minId);
    }
    addItem(minId + 1, 1);
    {
        database::Guard<GameDatabase> db(gameDatabaseManager_);
        ASSERT_EQ(true, db->getMaxInventoryId(itemId, 1));
        ASSERT_EQ(itemId, minId + 1);        
    }
}


TEST_F(SqlServerGameDatabaseTest, testChangeCharacterState)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->changeCharacterState(characterId_, cstCombat));
}


TEST_F(SqlServerGameDatabaseTest, testSaveActionBar)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->saveActionBar(characterId_, abcStartIndex, abpStartIndex, 1000));
}


TEST_F(SqlServerGameDatabaseTest, testSaveCharacterStats)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    DBCharacterStats stats;
    stats.zoneId_ = 1;
    ASSERT_EQ(true, db->saveCharacterStats(accountId_, characterId_, stats));
}


TEST_F(SqlServerGameDatabaseTest, testAddInventoryEquipItem)
{
    // TODO:
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    const ItemInfo itemInfo(servertest::defaultHelmetEquipCode, 1, invalidObjectId + 1, 2);

    ASSERT_EQ(true, db->addInventoryEquipItem(accountId_, characterId_, itemInfo, itPc));
}


TEST_F(SqlServerGameDatabaseTest, testChangeEquipItemInfo)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    const ItemInfo itemInfo(servertest::defaultHelmetEquipCode, 1, invalidObjectId + 1, 2);

    ASSERT_EQ(true, db->addInventoryEquipItem(accountId_, characterId_, itemInfo,itPc));
    ASSERT_EQ(true, db->changeEquipItemInfo(invalidObjectId + 1, servertest::upgradeHelmetEquipCode, 1));

}


TEST_F(SqlServerGameDatabaseTest, testLearnSkill)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->learnSkill(accountId_, characterId_, 0, 12345));
    ASSERT_EQ(true, db->learnSkill(accountId_, characterId_, 12345, 123457));
}


TEST_F(SqlServerGameDatabaseTest, testGetQuests)
{
    QuestCodes completeQuestCodes;
    AcceptedQuestInfos infos;
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->getQuests(characterId_, completeQuestCodes, infos));
    ASSERT_EQ(1, completeQuestCodes.size());
    ASSERT_EQ(1, infos.size());
    AcceptedQuestInfo& info = (*infos.begin());
    ASSERT_EQ(1, info.questGoalInfosMap_.size());
}

TEST_F(SqlServerGameDatabaseTest, testAcceptedQuest)
{
    QuestCodes completeQuestCodes;
    AcceptedQuestInfos infos;
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->acceptQuest(characterId_, 3));
}


TEST_F(SqlServerGameDatabaseTest, testCancelQuest)
{
    QuestCodes completeQuestCodes;
    AcceptedQuestInfos infos;
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->cancelQuest(characterId_, 2));
}


TEST_F(SqlServerGameDatabaseTest, testCompleteQuest)
{
    QuestCodes completeQuestCodes;
    AcceptedQuestInfos infos;
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->completeQuest(characterId_, 3));
}


TEST_F(SqlServerGameDatabaseTest, testInsertQuestMission)
{
    QuestCode questCode = 2;
    QuestMissionCode missionCode = 3;
    QuestGoalInfo info(1111, 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->insertQuestMission(characterId_, questCode, missionCode, info));
}


TEST_F(SqlServerGameDatabaseTest, testUpdateQuestMission)
{
    QuestCode questCode = 2;
    QuestMissionCode missionCode = 1;
    QuestGoalInfo info(0, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->updateQuestMission(characterId_, questCode, missionCode, info));
}


TEST_F(SqlServerGameDatabaseTest, testAddQuestItem)
{
    addQuestItem(invalidObjectId + 1, 1);
}


TEST_F(SqlServerGameDatabaseTest, testRemoveQuetItem)
{
    addItem(invalidObjectId + 1, 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->removeQuestItem(invalidObjectId + 1));
}


TEST_F(SqlServerGameDatabaseTest, testUpdateQuetItemUsableCount)
{
    // TODO:
    addQuestItem(invalidObjectId + 1, 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);    

    ASSERT_EQ(true, db->updateQuestItemUsableCount(invalidObjectId + 1, 0));
}


TEST_F(SqlServerGameDatabaseTest, testUpdateEnchantItem)
{
    // TODO:
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    const ItemInfo itemInfo(servertest::defaultHelmetEquipCode, 1, invalidObjectId + 1, 2);

    ASSERT_EQ(true, db->addInventoryEquipItem(accountId_, characterId_, itemInfo, itPc));
    ASSERT_EQ(true, db->addEquipSocketOption(accountId_, characterId_, invalidObjectId + 1, 1, EquipSocketInfo()));
    ASSERT_EQ(true, db->removeEquipSocketOption(invalidObjectId + 1, 1));
}


TEST_F(SqlServerGameDatabaseTest, testAddAccessory)
{
    // TODO:
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    const ItemInfo itemInfo(1, 1, invalidObjectId + 1, 2);

    ASSERT_EQ(true, db->addInventoryAccessoryItem(accountId_, characterId_, itemInfo, itPc));
}

TEST_F(SqlServerGameDatabaseTest, testCreateGuild)
{
    ObjectId characterId;
    createCharacter(characterId);

    BaseGuildInfo info(1, L"test1", 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->createGuild(info));
}


TEST_F(SqlServerGameDatabaseTest, testAddGuildGuild)
{
    ObjectId characterId;
    createCharacter(characterId);

    BaseGuildInfo info(1, L"test1", 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->createGuild(info));
    ASSERT_EQ(true, db->addGuildMember(1, GuildMemberInfo(characterId, L"test1", gmpLord, false, 0)));
}


TEST_F(SqlServerGameDatabaseTest, testGetMaxGuildId)
{
    ObjectId characterId;
    createCharacter(characterId);

    BaseGuildInfo info(1, L"test1", 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->createGuild(info));
    GuildId guildId = invalidGuildId;
    ASSERT_EQ(true, db->getMaxGuildId(guildId));
    ASSERT_EQ(GuildId(2), guildId);
}

TEST_F(SqlServerGameDatabaseTest, testGetGuildInfos)
{
    BaseGuildInfo info(1, L"test1", 1);
    ObjectId characterId;
    createCharacter(characterId);
    database::Guard<GameDatabase> db(gameDatabaseManager_);

    ASSERT_EQ(true, db->createGuild(info));
    GuildInfos infos;
    ASSERT_EQ(true, db->getGuildInfos(infos));
    ASSERT_EQ(2, infos.size());
    ASSERT_EQ(true, db->addGuildMember(1, GuildMemberInfo(characterId, L"test1", gmpLord, false, 0)));
    GuildMemberInfos memberInfos;
    ASSERT_EQ(true, db->getGuildMemberInfos(1, memberInfos));
    ASSERT_EQ(1, memberInfos.size());
}

TEST_F(SqlServerGameDatabaseTest, testAcceptRepeatQuest)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->acceptRepeatQuest(characterId_, 1003, getTime()));

}

TEST_F(SqlServerGameDatabaseTest, testGetRepeatQuests)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    CompletedRepeatQuestInfos completeQuestInfos;
    AcceptedRepeatQuestInfos acceptQuestInfos;
    QuestCodes removeQuestCodes;
    ASSERT_EQ(true, db->getRepeatQuests(characterId_, completeQuestInfos, acceptQuestInfos, removeQuestCodes));
}

TEST_F(SqlServerGameDatabaseTest, testCancelRepeatQuests)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->cancelRepeatQuest(characterId_, 1002));

}


TEST_F(SqlServerGameDatabaseTest, testInsertRepeatQuestMission)
{
    QuestCode questCode = 1002;
    QuestMissionCode missionCode = 3;
    QuestGoalInfo info(1111, 1);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->insertRepeatQuestMission(characterId_, questCode, missionCode, info));
}


TEST_F(SqlServerGameDatabaseTest, testUpdateRepeatQuestMission)
{
    QuestCode questCode = 1002;
    QuestMissionCode missionCode = 1;
    QuestGoalInfo info(0, 2);
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->updateRepeatQuestMission(characterId_, questCode, missionCode, info));
}


TEST_F(SqlServerGameDatabaseTest, testInsertMail)
{
    MailInfo realMailInfo = createMailInfo(L"test1", characterId_,  L"testmail");
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ObjectId reveiverId = db->getPlayerId(L"test1");
    if (! isValidObjectId(reveiverId)) {
        realMailInfo.titleInfo_.mailType_ = mtNormal;
        reveiverId = realMailInfo.titleInfo_.objectId_;
    }    

    if (db->insertMail(realMailInfo, reveiverId)) {
        for (const ItemMap::value_type& value : realMailInfo.contentInfo_.mailInventory_.items_) {
            const ItemInfo& itemInfo = value.second;
            ASSERT_EQ(true, db->addMailItem(reveiverId, realMailInfo.titleInfo_.mailId_, itemInfo));
        }
    }
    else {
        ASSERT_TRUE(false);
    }
    // TODO: 추후에 정상정일 경우 브로드 케스팅으로 수정
    MailInfos mailInfos;
    ASSERT_EQ(true, db->getMails(mailInfos, characterId_));
    ASSERT_EQ(1, mailInfos.size());
    MailInfos::const_iterator pos = mailInfos.begin();
    const MailInfo& receiveMail = (*pos).second;
    ASSERT_EQ(true, receiveMail.titleInfo_.isValid());

    ASSERT_EQ(true, db->deleteMailItems(realMailInfo.titleInfo_.mailId_));
}


TEST_F(SqlServerGameDatabaseTest, testInsertAuction)
{
    FullAuctionInfo fullAucionInfo;
    fullAucionInfo.auctionId_ = 1;
    fullAucionInfo.currentBidMoney_ = 100;
    fullAucionInfo.buyoutMoney_ = 100;
    fullAucionInfo.itemInfo_ = ItemInfo(1, 1, 1, 1);
    fullAucionInfo.sellerId_ = characterId_;

    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->insertAuction(1, 1, fullAucionInfo));
    ASSERT_EQ(true, db->addAuctionEquipInfo(1, 1, fullAucionInfo.itemInfo_.equipItemInfo_));
    ASSERT_EQ(true, db->updateBid(1, 1, characterId_, 2));
    
    ServerAuctionInfoMap infoMap;
    ASSERT_EQ(true, db->getAuctions(infoMap, 1));
    ASSERT_EQ(1, infoMap.size());
    infoMap.clear();
    ASSERT_EQ(true, db->deleteAuction(1, 1));
    ASSERT_EQ(true, db->getAuctions(infoMap, 1));
    ASSERT_EQ(true, infoMap.empty());
}


TEST_F(SqlServerGameDatabaseTest, testCheckDuplicateNickname)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(ecCharacterDuplicatedNickname, db->checkDuplicateNickname(L"test1"));
    ASSERT_EQ(ecOk, db->checkDuplicateNickname(L"tesdasfadf"));
 }


TEST_F(SqlServerGameDatabaseTest, testUpdateArenaPoint)
{
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->updateArenaPoint(characterId_ , 0));
}

TEST_F(SqlServerGameDatabaseTest, testCreateBuilding)
{
    ObjectId buildingId = getMinZoneObjectId(1);
    BuildingInfo info;
    info.objectType_ = otBuilding;
    info.objectId_ = buildingId;
    info.buildingCode_ = 1111;
    info.startBuildTime_ = getTime() - 1000;
    info.state_ = bstBuilding;
    database::Guard<GameDatabase> db(gameDatabaseManager_);
    ASSERT_EQ(true, db->createBuilding(info));
    ItemInfo itemInfo(1, 1, 1, 1);
    ASSERT_EQ(true, db->addBuildingInventoryEquipItem(buildingId, itemInfo, itBuilding));

    ObjectIdSet deleteItems;
    ObjectIdSet deleteBuilds;
    ObjectId maxBuildingId = invalidObjectId;
    BuildingInfoMap infoMap;
    ASSERT_EQ(true, db->getBuildingInfos(deleteItems, deleteBuilds, maxBuildingId, infoMap, 1));
    ASSERT_EQ(1, infoMap.size());
    ASSERT_EQ(maxBuildingId, buildingId);
    ASSERT_EQ(true, db->removeBuildingInventoryItem(itemInfo.itemId_));
}
