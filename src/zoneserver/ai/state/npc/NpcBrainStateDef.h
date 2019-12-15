#pragma once

namespace gideon { namespace zoneserver { namespace ai {

/**
 * @enum NpcBrainStateId
 */
enum NpcBrainStateId
{
    stateUnknown = -1,

    stateGlobal = 0,

    stateIdle = 1,
    stateActive = 2,
    stateAttacking = 3,
    stateThinking = 4,
    stateEvading = 5,
    stateResting = 6,
    stateDialog = 7,
    stateFleeing = 8,
    stateSeekingAssist = 9,

    stateCount
};


inline bool isValid(NpcBrainStateId value)
{
    return (stateUnknown < value) && (value < stateCount);
}


template <typename T>
inline NpcBrainStateId toNpcBrainStateId(T value)
{
    return static_cast<NpcBrainStateId>(value);
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
