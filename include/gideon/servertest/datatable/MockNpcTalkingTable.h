#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/NpcTalkingTable.h>

namespace gideon { namespace servertest {

/**
* @class MockNpcTalkingTable
*/
class GIDEON_SERVER_API MockNpcTalkingTable : public datatable::NpcTalkingTable
{
public:
    MockNpcTalkingTable() {}

    virtual const datatable::NpcTalkingTemplate* getNpcTalkingTemplate(NpcTalkingCode code) const override ;

    virtual const datatable::NpcTalkingList* getNpcTalkingList(NpcCode code) const override {
        code;
        return nullptr;
    }

private:
    virtual const NpcTalkingListMap& getNpcTalkingListMap() const override {
        return talkingListMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }
    
private:
    NpcTalkingListMap talkingListMap_;
};

}} // namespace gideon { namespace servertest {
