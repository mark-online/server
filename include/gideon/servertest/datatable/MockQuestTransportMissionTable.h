#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/QuestTransportMissionTable.h>

namespace gideon { namespace servertest {

/**
* @class MockQuestTransportMissionTable
*/
class GIDEON_SERVER_API MockQuestTransportMissionTable : public gideon::datatable::QuestTransportMissionTable
{
public:
    MockQuestTransportMissionTable();	


    virtual const datatable::QuestTransportMissionTemplate* getQuestTransportMissionTemplate(QuestMissionCode code) const;


private:
    void fillQuestTransportMission();

    virtual const QuestTransportMissionMap& getQuestTransportMissionMap() const  {
        return missionMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    QuestTransportMissionMap missionMap_;
};

}} // namespace gideon { namespace servertest {