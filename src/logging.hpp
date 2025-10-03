#pragma once

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include <boost/noncopyable.hpp>

#if __cplusplus < 201701UL
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/make_tuple.hpp>
#endif  // C++ 17

#ifdef QT_CORE_LIB
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPoint>
#else
#include <fstream>
#include <iostream>
#endif // QT_CORE_LIB

#include "logging_common.hpp"
#include "logging_filemaster.hpp"

namespace Logging {

/**
 * @brief The LoggingMaster class Мастер вывода информации (логов). Синглетон
 */
class LoggingMaster : public boost::noncopyable {
    bool isWorking{
        false};  //! Флаг для определения, обязан ли логгер работать дальше
    bool isThreadExited{true};  //! Флаг для определения, закончил ли выполнение
                                //! поток вывода логов
    std::thread logThread;      //! Поток вывода логов
    std::list<std::function<void()>>
        taskList;           //! Список задач по выводу данных в поток логов
    std::mutex taskListMx;  //! Мьютекс для получения данных из списка задач по
                            //! выводу данных
    std::condition_variable addTaskCV;  //! Переменная для ожидания добавления
                                        //! данных на вывод в поток логов
    std::mutex addTaskMx;  //! Мьютекс для ожидания добавления данных на вывод в
                           //! поток логов

    LoggingFileMaster logfileMaster;

    /**
     * @brief waitForTasks Ожидать сигнала о добавлении задачи на вывод
     */
    void waitForTasks();

    /**
     * @brief taskAdded Уведомить о получении задачи на вывод
     */
    void taskAdded();

    LoggingMaster();
    ~LoggingMaster();

    template <typename T>
    void printLog(const T& v
#ifdef QT_CORE_LIB
    , QDebug& dbgStream) {
        dbgStream << v;
#else
                  ) {
        std::cout << v << " ";
#endif // QT_CORE_LIB
    }

public:
    LoggingFileMaster& getLogfileMaster();

    static LoggingMaster& getInstance();

    /**
     * @brief log Вывести данные в потоке логгирования. Для синхронного вывода
     * укажите isSync как true
     * @param args Данные на вывод
     */
    template<LoggingType lt, bool isSync, typename... Args>
    void log(Args... args) {
        auto timestamp = getCurrentTimestampFormatted();

        auto task = [=]() {
#ifdef QT_CORE_LIB
            auto dbgStream = qDebug();
            printLog(timestamp + " [" + logTypeStringColored<lt>() + "] ", dbgStream);
            (printLog(args, dbgStream), ...);
#else
            printLog(timestamp + " [" +
                                    logTypeStringColored<lt>() + "] ");
            (printLog(args), ...);
#endif // QT_CORE_LIB


            logfileMaster.log<lt, isSync>(timestamp + " [" +
                                          logTypeString<lt>() + "] ");
            (logfileMaster.log<lt, isSync>(args), ...);
            logfileMaster.log<lt, isSync>("\n");

#ifndef QT_CORE_LIB
            std::cout << std::endl;
#endif // QT_CORE_LIB

        };

        if constexpr (isSync) {
            task();
            return;
        }

        taskListMx.lock();
        taskList.push_back(task);
        taskListMx.unlock();
        taskAdded();
    }
};

#ifdef QT_CORE_LIB
template <>
inline void LoggingMaster::printLog(const std::string& v, QDebug& dbgStream) {
    dbgStream << v.c_str();
}
#endif // QT_CORE_LIB

}  // namespace Logging

// Параллельный логгер (макросы вывода данных через другой поток)
#define LOG_EMPTY(...)                                                         \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Empty, false>(__VA_ARGS__)
#define LOG_DEBUG(...)                                                         \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Debug, false>(__VA_ARGS__)
#define LOG_INFO(...)                                                          \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Info, false>(__VA_ARGS__)
#define LOG_WARNING(...)                                                       \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Warning, false>(__VA_ARGS__)
#define LOG_ERROR(...)                                                         \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Error, false>(__VA_ARGS__)
#define LOG_OK(...)                                                            \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Ok, false>(__VA_ARGS__)

// Синхронная версия логгера (макросы вывода данных через текущий поток)
#define LOG_EMPTY_SYNC(...)                                                    \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Empty, true>(__VA_ARGS__)
#define LOG_DEBUG_SYNC(...)                                                    \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Debug, true>(__VA_ARGS__)
#define LOG_INFO_SYNC(...)                                                     \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Info, true>(__VA_ARGS__)
#define LOG_WARNING_SYNC(...)                                                  \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Warning, true>(__VA_ARGS__)
#define LOG_ERROR_SYNC(...)                                                    \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Error, true>(__VA_ARGS__)
#define LOG_OK_SYNC(...)                                                       \
    Logging::LoggingMaster::getInstance().log<Logging::LoggingType::Ok, true>( \
        __VA_ARGS__)
