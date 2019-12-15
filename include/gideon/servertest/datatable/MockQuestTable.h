#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/QuestTable.h>

namespace gideon { namespace servertest {

/**
* @class MockQuestTable
*/
class GIDEON_SERVER_API MockQuestTable : public gideon::datatable::QuestTable
{
public:
    MockQuestTable();
    virtual ~MockQuestTable() {
        destroyQuests();
    }
    virtual const gideon::datatable::QuestTemplate* getQuestTemplate(QuestCode code) const;

private:
    void fillQuests();
    void fillKillQuest();
    void fillObtainQuest();
    void fillTransportQuest();
    void fillKillRepeatQuest();
    void destroyQuests();

    virtual const QuestMap& getQuestMap() const {
        return questMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }
    
private:
    QuestMap questMap_;
};

}} // namespace gideon { namespace servertest {
