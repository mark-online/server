#include "ZoneServerPCH.h"
#include "EventTriggerManager.h"
#include "Events.h"
#include "Actions.h"
#include <gideon/cs/datatable/EventTriggerTable.h>
#include <gideon/cs/datatable/NpcTable.h>
#include <gideon/cs/datatable/HarvestTable.h>
#include <gideon/cs/datatable/TreasureTable.h>
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/cs/datatable/BuildingTable.h>
#include <gideon/cs/datatable/QuestItemTable.h>
#include <gideon/cs/shared/data/ObeliskInfo.h>
#include <gideon/cs/datatable/DeviceTable.h>
#include <gideon/cs/datatable/ElementTable.h>
#include <gideon/cs/datatable/QuestActivationMissionTable.h>
#include <gideon/cs/datatable/QuestKillMissionTable.h>

namespace gideon { namespace zoneserver { namespace ai { namespace evt {

bool isAllowedEntityCode(DataCode entityCode)
{
    if (! isValidDataCode(entityCode)) {
        return false;
    }

    switch (getCodeType(entityCode)) {
    case ctNpc:
        return NPC_TABLE->getNpcTemplate(entityCode) != nullptr;
    case ctHarvest:
        return HARVEST_TABLE->getHarvest(entityCode) != nullptr;
    case ctTreasure:
        return TREASURE_TABLE->getTreasure(entityCode) != nullptr;
    case ctAnchor:
        return ANCHOR_TABLE->getAnchorTemplate(entityCode) != nullptr;
    case ctBuilding:
        return BUILDING_TABLE->getBuildingTemplate(entityCode) != nullptr;
    case ctElement:
        return ELEMENT_TABLE->getElement(entityCode) != nullptr;
    case ctQuestItem:
        return QUEST_ITEM_TABLE->getQuestItemTemplate(entityCode) != nullptr;
    case ctObelisk:
        return entityCode == uniqueObeliskCode;
    case ctDevice:
        return DEVICE_TABLE->getDeviceTemplate(entityCode) != nullptr;
    case ctQuestMission:
        if (isActivationMission(getQuestMissionType(entityCode))) {
            return QUEST_ACTIVATION_MISSION_TABLE->getQuestActivationMissionTemplate(entityCode) != nullptr;
        }
        else if (isKillMission(getQuestMissionType(entityCode))) {
            return QUEST_KILL_MISSION_TABLE->getQuestKillMissionTemplate(entityCode) != nullptr;
        }
        else {
            return false;
        }
    // TODO: grave stone 지원 (예, NPC가 플레이어 집행)
    }
    SNE_LOG_ERROR("EventAI - Invalid entity_code(%d)", entityCode);
    return false;
}


// = EventTriggerManager

EventTriggerManager& EventTriggerManager::instance()
{
    static EventTriggerManager s_EventTriggerManager;
    return s_EventTriggerManager;
}


EventTriggerManager::~EventTriggerManager()
{
    for (EventMultiMapMap::value_type& value : eventMapMap_) {
        for (EventMultiMap::value_type& value2 : value.second) {
            boost::checked_delete(value2.second);
        }
    }
}


bool EventTriggerManager::initialize(MapCode globalMapCode)
{
    if (! EVT_TABLE) {
        assert(false);
        return false;
    }

    using namespace datatable;
    for (const EventTriggerTable::EventTriggerListMap::value_type& value :
        EVT_TABLE->getEventTriggerListMap()) {
        const DataCode entityCode = value.first;
        for (const gdt::evt_t* eventTemplate : value.second) {
            const MapCode mapCode = eventTemplate->map_code();
            if (isValidMapCode(mapCode) && (mapCode != globalMapCode)) {
                continue;
            }

            std::unique_ptr<Event> event = createEvent(*eventTemplate);
            if (! event) {
                return false;
            }
            if (! loadActions(*event, *eventTemplate)) {
                return false;
            }
            assert(entityCode == event->getEntityCode());
            Event* releasedEvent = event.release();
            eventMapMap_[entityCode].insert(
                EventMultiMap::value_type(releasedEvent->getEventType(), releasedEvent));
        }
    }

    return true;
}


void EventTriggerManager::trigger(EventType eventType, go::Entity& owner, DataCode dataCode)
{
    const EventMultiMap* eventMap = getEventMap(dataCode);
    if (! eventMap) {
        return;
    }

    EventMultiMap::const_iterator pos = eventMap->lower_bound(eventType);
    const EventMultiMap::const_iterator end = eventMap->upper_bound(eventType);
    for (; pos != end; ++pos) {
        Event* event = (*pos).second;
        if (event->process(owner)) {
            event->processActions(owner, nullptr);
        }
    }
}


std::unique_ptr<Event> EventTriggerManager::createEvent(const gdt::evt_t& eventTemplate)
{
    std::unique_ptr<Event> event;
    const int eventType = eventTemplate.event_type();
    switch (eventType) {
    case evtTimerInCombat:
        event = std::make_unique<EventTimerInCombat>();
        break;
    case evtTimerInPeace:
        event = std::make_unique<EventTimerInPeace>();
        break;
    case evtLosInPeace:
        event = std::make_unique<EventLosInPeace>();
        break;
    case evtHp:
        event = std::make_unique<EventHp>();
        break;
    case evtSpawned:
        event = std::make_unique<EventSpawned>();
        break;
    case evtDespawned:
        event = std::make_unique<EventDespawned>();
        break;
    case evtDied:
        event = std::make_unique<EventDied>();
        break;
    case evtWanderPaused:
        event = std::make_unique<EventWanderPaused>();
        break;
    case evtPathNodeArrived:
        event = std::make_unique<EventPathNodeArrived>();
        break;
    case evtDeviceActivated:
        event = std::make_unique<EventDeviceActivated>();
        break;
    case evtItemUsed:
        event = std::make_unique<EventItemUsed>();
        break;
    case evtActivationMissionCompleted:
        event = std::make_unique<EventActivationMissionCompleted>();
        break;
    case evtKillMissionCompleted:
        event = std::make_unique<EventKillMissionCompleted>();
        break;
    default:
        SNE_LOG_ERROR("EventTriggerManager::createEvent() - Invalid EventType(%d)", eventType);
        assert(false);
    }

    if (event != nullptr) {
        if (! event->initialize(eventTemplate)) {
            event.reset();
        }
    }
    return event;
}


bool EventTriggerManager::loadActions(Event& event, const gdt::evt_t& eventTemplate)
{
    if (! loadAction(event, 0, eventTemplate, eventTemplate.action1_type(),
        int(eventTemplate.action1_param1()), int(eventTemplate.action1_param2()),
        int(eventTemplate.action1_param3()), int(eventTemplate.action1_param4()),
        int(eventTemplate.action1_param5()))) {
        return false;
    }

    if (! loadAction(event, 1, eventTemplate, eventTemplate.action2_type(),
        int(eventTemplate.action2_param1()), int(eventTemplate.action2_param2()),
        int(eventTemplate.action2_param3()), int(eventTemplate.action2_param4()),
        int(eventTemplate.action2_param5()))) {
        return false;
    }

    if (! loadAction(event, 2, eventTemplate, eventTemplate.action3_type(),
        int(eventTemplate.action3_param1()), int(eventTemplate.action3_param2()),
        int(eventTemplate.action3_param3()), int(eventTemplate.action3_param4()),
        int(eventTemplate.action3_param5()))) {
        return false;
    }

    return true;
}


bool EventTriggerManager::loadAction(Event& event, int index, const gdt::evt_t& eventTemplate,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    if (actionType == actNothing) {
        return true;
    }

    auto action = createAction(eventTemplate.event_type(), actionType,
        param1, param2, param3, param4, param5);
    if (! action.get()) {
        return false;
    }
    event.setAction(std::move(action), index);
    return true;
}


std::unique_ptr<Action> EventTriggerManager::createAction(int eventType,
    int actionType, int param1, int param2, int param3, int param4, int param5)
{
    std::unique_ptr<Action> action;
    switch (actionType) {
    case actThreat:
        action = std::make_unique<ActionThreat>();
        break;
    case actFlee:
        action = std::make_unique<ActionFlee>();
        break;
    case actFleeForAssist:
        action = std::make_unique<ActionFleeForAssist>();
        break;
    case actSetPhase:
        action = std::make_unique<ActionSetPhase>();
        break;
    case actIncreasePhase:
        action = std::make_unique<ActionIncreasePhase>();
        break;
    case actRandomPhaseRange:
        action = std::make_unique<ActionRandomPhaseRange>();
        break;
    case actPlayAction:
        action = std::make_unique<ActionPlayAction>();
        break;
    case actSummonNpc:
        action = std::make_unique<ActionSummonNpc>();
        break;
    case actFormUp:
        action = std::make_unique<ActionFormUp>();
        break;
    case actTransform:
        action = std::make_unique<ActionTransform>();
        break;
    case actTeleport:
        action = std::make_unique<ActionTeleport>();
        break;
    case actMoveThruPath:
        action = std::make_unique<ActionMoveThruPath>();
        break;
    case actActivateDevice:
        action = std::make_unique<ActionActivateDevice>();
        break;
    default:
        SNE_LOG_ERROR("EventTriggerManager::createAction() - Invalid ActionType(%d)", actionType);
        assert(false);
    }

    if (action != nullptr) {
        if (! action->initialize(static_cast<EventType>(eventType),
            actionType, param1, param2, param3, param4, param5)) {
            action.reset();
        }
    }
    return action;
}


EventTriggerManager::EventMultiMap* EventTriggerManager::getEventMap(DataCode dataCode)
{
    const EventMultiMapMap::iterator pos = eventMapMap_.find(dataCode);
    if (pos != eventMapMap_.end()) {
        return &(*pos).second;
    }
    return nullptr;
}

}}}} // namespace gideon { namespace zoneserver { namespace ai { namespace evt {
