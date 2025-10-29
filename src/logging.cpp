#include "logging.hpp"

#include <filesystem>

namespace Logging {

void LoggingMaster::waitForTasks() {
    std::unique_lock<std::mutex> lock(addTaskMx);
    addTaskCV.wait(lock);
}

void LoggingMaster::taskAdded() {
    std::unique_lock<std::mutex> lock(addTaskMx);
    addTaskCV.notify_one();
}


LoggingMaster::LoggingMaster(const std::string &logfilePath) :
    logfileMaster{logfilePath + std::filesystem::path::preferred_separator + getLogfilename()} {

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

LoggingMaster& LoggingMaster::getInstance(const std::string& logfileDir) {
    static LoggingMaster inst(logfileDir);
    return inst;
}

}  // namespace Logging
