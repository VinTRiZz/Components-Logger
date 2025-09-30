#include "logging.hpp"

namespace Logging {

void LoggingMaster::waitForTasks() {
    std::unique_lock<std::mutex> lock(addTaskMx);
    addTaskCV.wait(lock);
}

void LoggingMaster::taskAdded() {
    std::unique_lock<std::mutex> lock(addTaskMx);
    addTaskCV.notify_one();
}


LoggingMaster::LoggingMaster() :
    logfileMaster{"app.log"} {

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

LoggingMaster::~LoggingMaster() {
    isWorking = false;
    while (!isThreadExited) {
        taskAdded();
    }
    if (logThread.joinable()) {
        logThread.join();
    }
}

LoggingFileMaster &LoggingMaster::getLogfileMaster()
{
    return logfileMaster;
}

LoggingMaster& LoggingMaster::getInstance() {
    static LoggingMaster inst;
    return inst;
}

}  // namespace Logging
