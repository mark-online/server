#pragma once

#include <gideon/cs/shared/data/Coordinate.h>

namespace gideon { namespace zoneserver {
class WorldEventMissionCallback;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
* @class WorldEventable
 * .
 */
class WorldEventable
{
public:
    WorldEventable() :
        worldEventMissionCallback_(nullptr),
        shouldNotifyMovement_(false),
        notifyMovementCount_(0),
        joinDistanceSq_(0.0f)
    {
    }

    virtual ~WorldEventable() {}
	
public:
    void setWorldEventMissionCallback(WorldEventMissionCallback& callback, 
        bool shouldNotifyMovement, float32_t joinDistance) {
        worldEventMissionCallback_ = &callback;
        shouldNotifyMovement_ = shouldNotifyMovement;
        joinDistanceSq_ = joinDistance * joinDistance;
    }

    void resetWorldEventable() {
        worldEventMissionCallback_ = nullptr;
        shouldNotifyMovement_ = false;
        notifyMovementCount_ = 0;
        joinDistanceSq_ = 0.0f;
    }

    WorldEventMissionCallback* getWorldEventMissionCallback() {
        return worldEventMissionCallback_;
    }

    const WorldEventMissionCallback* getWorldEventMissionCallback() const {
        return worldEventMissionCallback_;
    }

    bool shouldNotifyMovementable() const {
        return worldEventMissionCallback_ != nullptr && shouldNotifyMovement_;
    }

    bool hasMissionJoinAbility() const {
        return worldEventMissionCallback_ != nullptr && joinDistanceSq_ > 0.0f;
    }

    virtual void notifyMovemoent(const Position& position, bool isStop) = 0;

    virtual float32_t getJoinDistanceSq() const {
        return joinDistanceSq_;
    }

protected:
    void updateNotifyMovementCount(bool isStop) {
        if (isStop) {
            notifyMovementCount_ = 0;
            return;
        }
        const uint32_t notifyCount =20;
        if (notifyMovementCount_ < notifyCount) {
            ++notifyMovementCount_;
        }
        else {
            notifyMovementCount_ = 0;
        }
    }

    bool shouldNotifyMovement() const {
        return notifyMovementCount_ == 0;
    }

private:
    WorldEventMissionCallback* worldEventMissionCallback_;
    bool shouldNotifyMovement_;
    uint32_t notifyMovementCount_;
    float32_t joinDistanceSq_;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
