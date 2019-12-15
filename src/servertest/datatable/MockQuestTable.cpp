#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockQuestTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {


MockQuestTable::MockQuestTable()
{
    fillQuests();
}

const datatable::QuestTemplate* MockQuestTable::getQuestTemplate(QuestCode code) const
{
    const QuestMap::const_iterator pos = questMap_.find(code);
    if (pos != questMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}


void MockQuestTable::fillQuests()
{
    fillKillQuest();
    fillObtainQuest();
    fillTransportQuest();
    fillKillRepeatQuest();
}


void MockQuestTable::fillKillQuest()
{
    gdt::quest_t* quest = new gdt::quest_t;
    quest->quest_code(killScorpionQuestCode);
    quest->issue_condition(qicNpc);
    quest->issue_info(normalNpcCode);
    quest->issue_level(1);
    quest->issue_special_condition_1(qiscNone);
    quest->special_condition_info_1(0);
    quest->special_condition_value_1(0);
    quest->issue_special_condition_2(qiscNone);
    quest->special_condition_info_2(0);
    quest->special_condition_value_2(0);
    quest->complete_npc(::gdt::npc_code_t(0));

    quest->remuneration_kind_1(qrkMoney);
    quest->remuneration_info_1(0);
    quest->remuneration_value_1(1000);

    quest->remuneration_kind_2(qrkNone);
    quest->remuneration_info_2(0);
    quest->remuneration_value_2(0);

    quest->remuneration_kind_3(qrkNone);
    quest->remuneration_info_3(0);
    quest->remuneration_value_3(0);

    quest->mission_code_1(scorpionKillMissionCode);
    quest->mission_code_2(::gdt::quest_mission_code_t(0));
    quest->mission_code_3(::gdt::quest_mission_code_t(0));
    quest->is_party_quest(false);
    quest->is_repeat_quest(false);

    quest->select_remuneration_item_code_1(0);
    quest->select_remuneration_item_code_2(0);
    quest->select_remuneration_item_code_3(0);
    quest->select_remuneration_item_code_4(0);
    quest->select_remuneration_item_code_5(0);

    questMap_.emplace(killScorpionQuestCode, datatable::QuestTemplate(*quest));
}


void MockQuestTable::fillObtainQuest()
{
    gdt::quest_t* quest = new gdt::quest_t;
    quest->quest_code(obtainScorpionQuestCode);
    quest->issue_condition(qicNpc);
    quest->issue_info(normalNpcCode);
    quest->issue_level(1);
    quest->issue_special_condition_1(qiscNone);
    quest->special_condition_info_1(0);
    quest->special_condition_value_1(0);
    quest->issue_special_condition_2(qiscNone);
    quest->special_condition_info_2(0);
    quest->special_condition_value_2(0);
    quest->complete_npc(::gdt::npc_code_t(0));

    quest->remuneration_kind_1(qrkMoney);
    quest->remuneration_info_1(0);
    quest->remuneration_value_1(1000);

    quest->remuneration_kind_2(qrkNone);
    quest->remuneration_info_2(0);
    quest->remuneration_value_2(0);

    quest->remuneration_kind_3(qrkNone);
    quest->remuneration_info_3(0);
    quest->remuneration_value_3(0);

    quest->mission_code_1(scorpionObtainMissionCode);
    quest->mission_code_2(::gdt::quest_mission_code_t(0));
    quest->mission_code_3(::gdt::quest_mission_code_t(0));
    quest->is_party_quest(false);
    quest->is_repeat_quest(false);

    quest->select_remuneration_item_code_1(0);
    quest->select_remuneration_item_code_2(0);
    quest->select_remuneration_item_code_3(0);
    quest->select_remuneration_item_code_4(0);
    quest->select_remuneration_item_code_5(0);

    questMap_.emplace(obtainScorpionQuestCode, datatable::QuestTemplate(*quest));
}


void MockQuestTable::fillTransportQuest()
{
    gdt::quest_t* quest = new gdt::quest_t;
    quest->quest_code(transportQuestCode);
    quest->issue_condition(qicNpc);
    quest->issue_info(normalNpcCode);
    quest->issue_level(1);
    quest->issue_special_condition_1(qiscNone);
    quest->special_condition_info_1(0);
    quest->special_condition_value_1(0);
    quest->issue_special_condition_2(qiscNone);
    quest->special_condition_info_2(0);
    quest->special_condition_value_2(0);
    quest->complete_npc(::gdt::npc_code_t(0));

    quest->remuneration_kind_1(qrkMoney);
    quest->remuneration_info_1(0);
    quest->remuneration_value_1(1000);

    quest->remuneration_kind_2(qrkNone);
    quest->remuneration_info_2(0);
    quest->remuneration_value_2(0);

    quest->remuneration_kind_3(qrkNone);
    quest->remuneration_info_3(0);
    quest->remuneration_value_3(0);

    quest->mission_code_1(questItemTransportMissionCode);
    quest->mission_code_2(::gdt::quest_mission_code_t(0));
    quest->mission_code_3(::gdt::quest_mission_code_t(0));
    quest->is_party_quest(false);
    quest->is_repeat_quest(false);

    quest->select_remuneration_item_code_1(0);
    quest->select_remuneration_item_code_2(0);
    quest->select_remuneration_item_code_3(0);
    quest->select_remuneration_item_code_4(0);
    quest->select_remuneration_item_code_5(0);

    questMap_.emplace(transportQuestCode, datatable::QuestTemplate(*quest));
}


void MockQuestTable::fillKillRepeatQuest()
{
    gdt::quest_t* quest = new gdt::quest_t;
    quest->quest_code(repeatQuestCode);
    quest->issue_condition(qicNpc);
    quest->issue_info(normalNpcCode);
    quest->issue_level(1);
    quest->issue_special_condition_1(qiscNone);
    quest->special_condition_info_1(0);
    quest->special_condition_value_1(0);
    quest->issue_special_condition_2(qiscNone);
    quest->special_condition_info_2(0);
    quest->special_condition_value_2(0);
    quest->complete_npc(::gdt::npc_code_t(0));

    quest->remuneration_kind_1(qrkMoney);
    quest->remuneration_info_1(0);
    quest->remuneration_value_1(1000);

    quest->remuneration_kind_2(qrkNone);
    quest->remuneration_info_2(0);
    quest->remuneration_value_2(0);

    quest->remuneration_kind_3(qrkNone);
    quest->remuneration_info_3(0);
    quest->remuneration_value_3(0);

    quest->mission_code_1(scorpionKillMissionCode);
    quest->mission_code_2(::gdt::quest_mission_code_t(0));
    quest->mission_code_3(::gdt::quest_mission_code_t(0));
    quest->is_party_quest(false);
    quest->is_repeat_quest(false);

    quest->select_remuneration_item_code_1(0);
    quest->select_remuneration_item_code_2(0);
    quest->select_remuneration_item_code_3(0);
    quest->select_remuneration_item_code_4(0);
    quest->select_remuneration_item_code_5(0);

    questMap_.emplace(repeatQuestCode, datatable::QuestTemplate(*quest));
}


void MockQuestTable::destroyQuests()
{
    for (QuestMap::value_type& value : questMap_) {
        datatable::QuestTemplate& qt = value.second;
        qt.deleteQuestInfo();		
    }
}

}} //namespace gideon { namespace servertest {
