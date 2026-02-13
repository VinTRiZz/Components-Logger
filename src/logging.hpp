#pragma once

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include <boost/noncopyable.hpp>
#include <boost/core/demangle.hpp>

#if __cplusplus < 201701UL
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/make_tuple.hpp>
#endif  // C++ 17

#ifdef COMPONENTS_IS_ENABLED_QT
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPoint>
#else
#include <fstream>
#include <iostream>
#endif // COMPONENTS_IS_ENABLED_QT

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

    std::mutex logfileWriteMx; //! Мьютекс для записи в логфайл
    LoggingFileMaster logfileMaster;

    /**
     * @brief waitForTasks Ожидать сигнала о добавлении задачи на вывод
     */
    void waitForTasks();

    /**
     * @brief taskAdded Уведомить о получении задачи на вывод
     */
    void taskAdded();

    LoggingMaster(const std::string& logfilePath);
    ~LoggingMaster();

    template <typename T>
    void printLog(const T& v
#ifdef COMPONENTS_IS_ENABLED_QT
    , QDebug& dbgStream) {
        dbgStream << v;
#else
                  ) {
        std::cout << v << " ";
#endif // COMPONENTS_IS_ENABLED_QT
    }

public:
    LoggingFileMaster& getLogfileMaster();

    static LoggingMaster& getInstance(const std::string &logfileDir = ".");

    /**
     * @brief log Вывести данные в потоке логгирования. Для синхронного вывода
     * укажите isSync как true
     * @param args Данные на вывод
     */
    template<LoggingType lt, bool isSync, typename... Args>
    void log(Args... args) {
        auto timestamp = getCurrentTimestampFormatted();

        auto task = [=]() {
#ifdef COMPONENTS_IS_ENABLED_QT
            auto dbgStream = qDebug();
            if constexpr (lt != LoggingType::Empty) {
                printLog(timestamp + " [" + logTypeStringColored<lt>() + "] ", dbgStream);
            }
            (printLog(args, dbgStream), ...);
#else
            if constexpr (lt != LoggingType::Empty) {
                printLog(timestamp + " [" +
                                        logTypeStringColored<lt>() + "] ");
            }
            (printLog(args), ...);
#endif // COMPONENTS_IS_ENABLED_QT

            logfileWriteMx.lock();
            if constexpr (lt != LoggingType::Empty) {
                logfileMaster.log<lt, isSync>(timestamp + " [" + logTypeString<lt>() + "] ", args...);
            } else {
                logfileMaster.log<lt, isSync>(args...);
            }
            logfileWriteMx.unlock();

#ifndef COMPONENTS_IS_ENABLED_QT
            std::cout << std::endl;
#endif // COMPONENTS_IS_ENABLED_QT

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

#ifndef COMPONENTS_IS_ENABLED_QT
template <>
inline void LoggingMaster::printLog(const bool& v) {
    std::cout << (v ? "true" : "false") << " ";
}
#endif // NOT COMPONENTS_IS_ENABLED_QT

#ifdef COMPONENTS_IS_ENABLED_QT
template <>
inline void LoggingMaster::printLog(const std::string& v, QDebug& dbgStream) {
    dbgStream << v.c_str();
}
#endif // COMPONENTS_IS_ENABLED_QT

}  // namespace Logging

// Параллельный логгер (макросы вывода данных через другой поток)
#define COMPLOG_TYPENAME(LOGGING_LOGITEM)                                          \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Debug, false>("Type of object: [", #LOGGING_LOGITEM, "] is: [", boost::core::demangle(typeid(LOGGING_LOGITEM).name()), "]")
#define COMPLOG_EMPTY(...)                                                         \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Empty, false>(__VA_ARGS__)
#define COMPLOG_DEBUG(...)                                                         \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Debug, false>(__VA_ARGS__)
#define COMPLOG_INFO(...)                                                          \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Info, false>(__VA_ARGS__)
#define COMPLOG_WARNING(...)                                                       \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Warning, false>(__VA_ARGS__)
#define COMPLOG_ERROR(...)                                                         \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Error, false>(__VA_ARGS__)
#define COMPLOG_OK(...)                                                            \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Ok, false>(__VA_ARGS__)

// Синхронная версия логгера (макросы вывода данных через текущий поток)
#define COMPLOG_TYPENAME_SYNC(LOGGING_LOGITEM)                                     \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Debug, true>("Type of object: [", #LOGGING_LOGITEM, "] is: [", boost::core::demangle(typeid(LOGGING_LOGITEM).name()), "]")
#define COMPLOG_EMPTY_SYNC(...)                                                    \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Empty, true>(__VA_ARGS__)
#define COMPLOG_DEBUG_SYNC(...)                                                    \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Debug, true>(__VA_ARGS__)
#define COMPLOG_INFO_SYNC(...)                                                     \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Info, true>(__VA_ARGS__)
#define COMPLOG_WARNING_SYNC(...)                                                  \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Warning, true>(__VA_ARGS__)
#define COMPLOG_ERROR_SYNC(...)                                                    \
    Logging::LoggingMaster::getInstance()                                      \
        .log<Logging::LoggingType::Error, true>(__VA_ARGS__)
#define COMPLOG_OK_SYNC(...)                                                       \
    Logging::LoggingMaster::getInstance().log<Logging::LoggingType::Ok, true>( \
        __VA_ARGS__)
