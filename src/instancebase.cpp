#include "instancebase.hpp"

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <atomic>

#include <filesystem>

#include "common.hpp"

namespace Logger {

struct InstanceBase::Impl {
    std::atomic<bool> isWorking         {false};
    std::atomic<bool> isThreadExited    {true};

    std::thread taskProcessingThread;

    std::mutex taskListMx;
    std::list<std::function<void()> > taskList;

    std::condition_variable notifyCV;
    std::mutex notifyMx;
};

InstanceBase::InstanceBase() :
    d {new Impl}
{
    d->isWorking.store(std::memory_order_release);
    d->taskProcessingThread = std::thread([this]() {
        d->isThreadExited = false;
        while (d->isWorking.load(std::memory_order_acquire)) {
            waitForTasks();

            d->taskListMx.lock();
            while (!d->taskList.empty()) {
                auto nextTask = d->taskList.front();
                d->taskList.pop_front();
                d->taskListMx.unlock();

                nextTask();
                d->taskListMx.lock();
            }
            d->taskListMx.unlock();
        }
        d->isThreadExited.store(true, std::memory_order_release);
    });
}

InstanceBase::~InstanceBase()
{
    if (d->isWorking.load(std::memory_order_acquire)) {
        deinit();
    }
}

void InstanceBase::waitForTasks() {
    std::unique_lock<std::mutex> lock(d->notifyMx);
    d->notifyCV.wait(lock);
}

void InstanceBase::notifyTaskAdded() {
    std::unique_lock<std::mutex> lock(d->notifyMx);
    d->notifyCV.notify_one();
}

void InstanceBase::callInit(const std::string &logfileDir)
{
    this->init(logfileDir);
}

void InstanceBase::deinit()
{
    d->isWorking.store(false, std::memory_order_release);
    while (!d->isThreadExited) {
        notifyTaskAdded();
    }
    if (d->taskProcessingThread.joinable()) {
        d->taskProcessingThread.join();
    }
}

void InstanceBase::addTask(std::function<void ()> &&tsk)
{
    d->taskList.emplace_back(std::move(tsk));
    notifyTaskAdded();
}

} // namespace Logger
