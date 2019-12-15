#pragma once

#include <gideon/Common.h>
#include <sne/base/session/manager/SessionManager.h>
#include <sne/server/common/Property.h>
#include <sne/base/utility/Assert.h>
#include <sne/base/utility/Logger.h>

namespace gideon { namespace serverbase {

/**
 * @class SessionService
 *
 * 세션 서비스
 */
class SessionService :
    public sne::base::SessionManagerCallback
{
public:
    SessionService() :
        highConcurrentSessionCount_(0),
        maxConcurrentSessionCount_(0),
        currentSessionCount_(0) {}

    virtual ~SessionService() {}

public:
    bool isServiceThrottling() const {
        std::unique_lock<std::mutex> lock(lock_);

        if (currentSessionCount_ < maxConcurrentSessionCount_) {
            return false;
        }

        SNE_LOG_WARNING("Too many sessions are issued(%u > %u).",
            currentSessionCount_, maxConcurrentSessionCount_);
        return true;
    }

protected:
    bool loadProperties() {
        highConcurrentSessionCount_ = SNE_PROPERTIES::getProperty<size_t>(
            "c2s.high_concurrent_session_count");

        maxConcurrentSessionCount_ = SNE_PROPERTIES::getProperty<size_t>(
            "c2s.max_concurrent_session_count");

        SNE_ASSERT(maxConcurrentSessionCount_ >= highConcurrentSessionCount_);

        return true;
    }

    bool isServiceBusy() const {
        std::unique_lock<std::mutex> lock(lock_);
        return currentSessionCount_ >= highConcurrentSessionCount_;
    }

    std::mutex& getLock() {
        return lock_;
    }

private:
    // = sne::base::SessionManagerCallback overriding
    virtual void sessionAcquired(size_t acquiredSessionCount) {
        std::unique_lock<std::mutex> lock(lock_);
        currentSessionCount_ = acquiredSessionCount;
    }

    virtual void sessionReleased(size_t acquiredSessionCount) {
        std::unique_lock<std::mutex> lock(lock_);
        currentSessionCount_ = acquiredSessionCount;
    }

private:
    /// 처리가 허용되는 최고 클라이언트 세션수.
    /// 이 범위를 넘어서면 바로 에러 리턴된다.
    size_t highConcurrentSessionCount_;
    /// 처리가 허용되는 최대 클라이언트 세션수.
    /// 이 범위를 넘어서면 바로 접속 해제된다.
    size_t maxConcurrentSessionCount_;
    /// 현재 클라이언트 세션 수
    size_t currentSessionCount_;

    mutable std::mutex lock_;
};

}} // namespace gideon { namespace serverbase {
