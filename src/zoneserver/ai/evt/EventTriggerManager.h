#pragma once

#include "EventTriggerDef.h"
#include <gideon/cs/shared/data/Code.h>
#include <gideon/cs/shared/data/WorldInfo.h>
#include <boost/noncopyable.hpp>

namespace gdt {
class evt_t;
} // namespace gdt {

namespace gideon { namespace zoneserver { namespace go {
class Player;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

class Event;
class Action;

/**
 * @class EventTriggerManager
 */
class EventTriggerManager : public boost::noncopyable
{
public:
    typedef sne::core::MultiMap<EventType, Event*> EventMultiMap;
    typedef sne::core::HashMap<DataCode, EventMultiMap> EventMultiMapMap;

public:
    static EventTriggerManager& instance();

public:
    ~EventTriggerManager();

    bool initialize(MapCode globalMapCode);

public:
    /// 플레이어가 데이터코드에 해당하는 행위를 하였다(아이템 사용, 미션 완료 등)
    void trigger(EventType eventType, go::Entity& owner, DataCode dataCode);

public:
    EventMultiMap* getEventMap(DataCode dataCode);

private:
    std::unique_ptr<Event> createEvent(const gdt::evt_t& eventTemplate);

    bool loadActions(Event& event, const gdt::evt_t& eventTemplate);
    bool loadAction(Event& event, int index, const gdt::evt_t& eventTemplate,
        int actionType, int param1, int param2, int param3, int param4, int param5);
    std::unique_ptr<Action> createAction(int eventType,
        int actionType, int param1, int param2, int param3, int param4, int param5);

private:
    EventMultiMapMap eventMapMap_;
};

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {


#define EVT_MANAGER gideon::zoneserver::ai::evt::EventTriggerManager::instance()
