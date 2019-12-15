#include "ZoneServerTestPCH.h"

#ifndef NDEBUG

#include <gideon/cs/shared/data/UnionEntityInfo.h>
#include <gideon/cs/shared/data/UserInfo.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <gtest/gtest.h>

using namespace gideon;

/**
* @class DataSizeTest
*
* 실제 테스트라기 보다는 데이터 크기를 한 눈에 파악하기 위함
*/
class DataSizeTest : public testing::Test
{ 
};


TEST_F(DataSizeTest, testIdSize)
{
    ASSERT_EQ(8, sizeof(AccountId));
    ASSERT_EQ(8, sizeof(ObjectId));
}


TEST_F(DataSizeTest, testStringSize)
{
    ASSERT_EQ(48, sizeof(UserId));
    ASSERT_EQ(48, sizeof(Nickname));
    ASSERT_EQ(48, sizeof(ChatMessage));
}


TEST_F(DataSizeTest, testCreatureInfoSize)
{
    ASSERT_EQ(12, sizeof(Points));
    ASSERT_EQ(12, sizeof(Position));
    ASSERT_EQ(16, sizeof(ObjectPosition));
    ASSERT_EQ(8, sizeof(CurrentLevel));

    ASSERT_EQ(16, sizeof(GameObjectInfo));
    ASSERT_EQ(32, sizeof(EntityInfo));
    ASSERT_EQ(56, sizeof(CreatureInfo));

    ASSERT_EQ(816, sizeof(UnionEntityInfo));
}


TEST_F(DataSizeTest, testCharacterInfoSize)
{
    ASSERT_EQ(1, sizeof(CharacterAppearance));
    ASSERT_EQ(160, sizeof(CharacterEquipments));
    ASSERT_EQ(40, sizeof(CreatureSkills));

    ASSERT_EQ(136, sizeof(CharacterInfo));
    ASSERT_EQ(1272, sizeof(FullCharacterInfo));
}


TEST_F(DataSizeTest, testNpcInfoSize)
{
    ASSERT_EQ(64, sizeof(NpcInfo));
    //ASSERT_EQ(416, sizeof(FullNpcInfo));
}


TEST_F(DataSizeTest, testMoreInfoSize)
{
    ASSERT_EQ(104, sizeof(MoreNpcInfo));
    ASSERT_EQ(312, sizeof(MoreCharacterInfo));
}


TEST_F(DataSizeTest, testInventorySize)
{
    ASSERT_EQ(32, sizeof(ItemInfo));
    ASSERT_EQ(88, sizeof(InventoryInfo));
}


TEST_F(DataSizeTest, testUserInfoSize)
{
    ASSERT_EQ(8, sizeof(BaseUserInfo));
    ASSERT_EQ(56, sizeof(FullUserInfo));
}


TEST_F(DataSizeTest, testZoneInfoSize)
{
    ASSERT_EQ(96, sizeof(ZoneInfo));
}

#endif // NDEBUG