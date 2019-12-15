#include "ZoneServerPCH.h"
#include "AcceptedRepeatQuest.h"


namespace gideon { namespace zoneserver {

AcceptedRepeatQuest::AcceptedRepeatQuest(go::Entity& owner, const datatable::QuestTemplate& questTemplate,
    const AcceptedQuestInfo& acceptQuestInfo, sec_t acceptTime) :
    AcceptedQuest(owner, questTemplate, acceptQuestInfo),
    acceptTime_(acceptTime)
{
}



}} // namespace gideon { namespace zoneserver {