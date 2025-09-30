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

#ifdef QT_CORE
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPoint>
#else
#include <fstream>
#include <iostream>
#endif // QT_CORE

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

    /**
     * @brief The LoggingHelper class Помощник вывода данных в логфайл
     */
    class LoggingHelper {
#ifdef QT_CORE
        QDebug m_dbgStream{qDebug()};  //! Поток вывода (для Qt)
#endif // QT_CORE
    public:

        /**
         * @brief operator () Оператор для вывода данных в терминал и записи в
         * файл
         * @param val данные
         */
        template<typename T>
        void operator()(T&& val) {
#ifdef QT_CORE
            m_dbgStream << val;
#else
            std::cout << val;
#endif // QT_CORE
        }
    };

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
    void printLog(const T& v) {
#ifdef QT_CORE
        m_dbgStream << v;
#else
        std::cout << v << " ";
#endif // QT_CORE
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
#ifdef QT_CORE
            m_dbgStream = qDebug();
#endif // QT_CORE

            printLog(timestamp + " [" +
                                    logTypeStringColored<lt>() + "] ");
            logfileMaster.log<lt, isSync>(timestamp + " [" +
                                          logTypeString<lt>() + "] ");

#if __cplusplus < 201701UL
            boost::fusion::for_each(boost::fusion::make_tuple(args...), printLog);
#else
            (printLog(args), ...);
            (logfileMaster.log<lt, isSync>(args), ...);
            logfileMaster.log<lt, isSync>("\n");
#endif  // C++ 17

#ifndef QT_CORE
            std::cout << std::endl;
#endif // QT_CORE

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
