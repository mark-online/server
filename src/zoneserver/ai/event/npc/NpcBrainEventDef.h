#pragma once

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @enum NpcBrainEventId
 */
enum NpcBrainEventId
{
    eventUnknown = -1,

    eventActive = 0, //< AI를 본격 가동한다
    eventNothingToDo = 1, //< 아무 것도 할게 없다
    eventAttacked = 2, //< 공격을 받았다
    eventTired = 3, //< (공격하다가) 지쳤다
    eventReturned = 4, //< 스폰 위치로 복귀하였다
    eventAggressive = 5, //< 플레이어가 반응 지역 내로 접근하였다
    eventThreated = 6, //< 주변 NPC가 위협을 감지하였다(AI link)
    eventDialogRequested = 7, //< 누군가 말을 걸었다
    eventFeared = 8, //< 겁을 집어 먹었다(도망)
    eventSeekAssist = 9, //< 도우미 검색

    eventCount
};


inline bool isValid(NpcBrainEventId value)
{
    return (eventUnknown < value) && (value < eventCount);
}


template <typename T>
inline NpcBrainEventId toNpcBrainEventId(T value)
{
    return static_cast<NpcBrainEventId>(value);
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
