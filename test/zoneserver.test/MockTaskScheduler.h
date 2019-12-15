#pragma once

#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

/**
 * @class MockTaskScheduler
 */
class MockTaskScheduler : public sne::base::TaskScheduler
{
public:
    /**
     * @class MockTaskFuture
     */
    class MockTaskFuture : public sne::base::AsyncFuture,
        public sne::base::FutureObserver,
        public sne::core::ThreadSafeMemoryPoolMixin<MockTaskFuture>
    {
    public:
        MockTaskFuture(sne::base::Runnable* task, bool shouldDeleteTask) :
            task_(task),
            shouldDeleteTask_(shouldDeleteTask){}

        virtual ~MockTaskFuture() {
            if (shouldDeleteTask_) {
                delete task_;
            }
        }

    private:
        virtual void cancel() {}

        virtual bool isCancelled() const {
            return false;
        }

    private:
        // = sne::base::FutureObserver overriding
        virtual void update(const Future& /*future*/) {
            task_->run();
        }

    private:
        sne::base::Runnable* task_;
        bool shouldDeleteTask_;
    };

public:
    MockTaskScheduler(sne::base::IoContextTask& ioContextTask) :
        sne::base::TaskScheduler(ioContextTask) {}

private:
    virtual bool activate(size_t /*threadCount*/,
        size_t /*timerThreadCount*/) {
        return true;
    }

    virtual void deactivate() {}

    virtual void schedule(uint64_t /*id*/, sne::base::RunnablePtr task) {
        task->run();
    }

    virtual sne::base::Future::Ref schedule(
        sne::base::RunnablePtr task, sne::msec_t /*delay*/) {
        auto taskFuture = std::make_unique<MockTaskFuture>(task.release(), true);
        taskFuture->attach(taskFuture.get(), false);
        sne::base::Future::Ref future(taskFuture.release());
        future->completed();
        return future;
    }

    virtual sne::base::Future::Ref schedule(
        sne::base::Runnable& task, sne::msec_t /*delay*/) {
        auto taskFuture = std::make_unique<MockTaskFuture>(&task, false);
        taskFuture->attach(taskFuture.get(), false);
        sne::base::Future::Ref future(taskFuture.release());
        future->completed();
        return future;
    }

    virtual sne::base::Future::Ref schedule(
        sne::base::RunnablePtr task, sne::msec_t /*delay*/, sne::msec_t /*interval*/) {
        auto taskFuture = std::make_unique<MockTaskFuture>(task.release(), true);
        taskFuture->attach(taskFuture.get(), false);
        sne::base::Future::Ref future(taskFuture.release());
        future->completed();
        return future;
    }

    virtual sne::base::Future::Ref schedule(
        sne::base::Runnable& task, sne::msec_t /*delay*/, sne::msec_t /*interval*/) {
        auto taskFuture = std::make_unique<MockTaskFuture>(&task, false);
        taskFuture->attach(taskFuture.get(), false);
        sne::base::Future::Ref future(taskFuture.release());
        future->completed();
        return future;
    }

    virtual void cancel(sne::base::ScheduledTaskId /*taskId*/) {}
};
