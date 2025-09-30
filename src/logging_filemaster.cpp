#include "logging_filemaster.hpp"

namespace Logging
{

LoggingFileMaster::LoggingFileMaster(const std::string &filePath) :
    logfilePath{ filePath }
{
    isWorking = true;
    logThread = std::thread([this]() {
        isThreadExited = false;
        while (isWorking) {
            waitForTasks();

            taskListMx.lock();
            while (!taskList.empty()) {
                auto nextTask = taskList.front();
                taskList.pop_front();
                taskListMx.unlock();

                nextTask();
                taskListMx.lock();
            }
            taskListMx.unlock();
        }
        isThreadExited = true;
    });
}

void LoggingFileMaster::waitForTasks() {
    std::unique_lock<std::mutex> lock(addTaskMx);
    addTaskCV.wait(lock);
}

void LoggingFileMaster::taskAdded() {
    std::unique_lock<std::mutex> lock(addTaskMx);
    addTaskCV.notify_one();
}

LoggingFileMaster::~LoggingFileMaster() {
    isWorking = false;
    while (!isThreadExited) {
        taskAdded();
    }
    if (logThread.joinable()) {
        logThread.join();
    }
}

}
