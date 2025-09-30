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
#endif // QT_CORE

#include "logging_common.hpp"


namespace Logging
{

class LoggingFileMaster
{
#ifdef QT_CORE
    QFile logfile;                        //! Логфайл
    QTextStream logfileStream{&logfile};  //! Поток ввода в файл данных
#else
    std::string logfilePath;    //! Путь до логфайла
    std::fstream logfile;       //! Логфайл
#endif // QT_CORE

    std::mutex logfileMx;  //! Мьютекс для единоличной записи данных в файл

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
    * @brief waitForTasks Ожидать сигнала о добавлении задачи на вывод
    */
   void waitForTasks();

   /**
    * @brief taskAdded Уведомить о получении задачи на вывод
    */
   void taskAdded();

   template <typename T>
   void logger(T v) {
       logfile << v << " ";
   }

public:
   LoggingFileMaster(const std::string& filePath);
   ~LoggingFileMaster();

#ifdef QT_CORE
   static QString getCurrentLogfile();
#else
   static std::string geCurrentLogfile();
#endif // QT_CORE

   /**
    * @brief log Вывести данные в потоке логгирования. Для синхронного вывода
    * укажите isSync как true
    * @param args Данные на вывод
    */
   template<LoggingType lt, bool isSync, typename... Args>
   void log(Args... args) {
       auto timestamp = getCurrentTimestampFormatted();

       auto task = [=]() {
           logfileMx.lock();

#ifdef QT_CORE
           logfile.open(QIODevice::Append);
           if (!logfile.isOpen()) {
               throw std::runtime_error(
                   std::string("Error opening logfile (logfile path: ") +
                   logfile.fileName().toStdString() + ")");
           }
#else
            logfile.open(logfilePath, std::ios_base::out | std::ios_base::app);
            if (!logfile.is_open()) {
                throw std::runtime_error(
                    std::string("Error opening logfile (logfile path: ") +
                    logfilePath + ")");
            }
#endif // QT_CORE

#if __cplusplus < 201701UL
           boost::fusion::for_each(boost::fusion::make_tuple(args...), logger);
#else
           (logger(args), ...);
#endif  // C++ 17

#ifdef QT_CORE
           logfileStream << Qt::endl;
#endif // QT_CORE

           logfile.flush();
           logfile.close();
           logfileMx.unlock();
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


#ifdef QT_CORE // TODO: Write-up
// ==============================================================================
// //
// ==============================================================================
// //
// =================================== FOR QT TYPES
// ============================= //
// ==============================================================================
// //
// ==============================================================================
// //

template<>
inline void LoggingMaster::LoggingHelper::ostreamWriteOnly(std::string val) {
    m_dbgStream << val.c_str();
}

template<>
inline void LoggingMaster::LoggingHelper::ostreamWriteOnly(QString val) {
    m_dbgStream << val.toUtf8().data();
}

template<>
inline void LoggingMaster::LoggingHelper::ostreamWriteOnly(QPoint val) {
    m_dbgStream << val;
}

template<>
inline void LoggingMaster::LoggingHelper::ostreamWriteOnly(QPointF val) {
    m_dbgStream << val;
}

template<>
inline void LoggingMaster::LoggingHelper::fileWriteOnly(std::string val) {
    LoggingMaster::getInstance().logfileStream << val.c_str() << " ";
}

template<>
inline void LoggingMaster::LoggingHelper::fileWriteOnly(const char* val) {
    LoggingMaster::getInstance().logfileStream << QByteArray(val) << " ";
}

template<>
inline void LoggingMaster::LoggingHelper::fileWriteOnly(QString val) {
    LoggingMaster::getInstance().logfileStream << val.toUtf8().data() << " ";
}

template<>
inline void LoggingMaster::LoggingHelper::fileWriteOnly(QPoint val) {
    LoggingMaster::getInstance().logfileStream
        << "{P " << QString::number(val.x()).toUtf8().data() << "; "
        << QString::number(val.y()).toUtf8().data() << "} ";
}

template<>
inline void LoggingMaster::LoggingHelper::fileWriteOnly(QPointF val) {
    LoggingMaster::getInstance().logfileStream
        << "{P " << QString::number(val.x()).replace(",", ".").toUtf8().data()
        << "; " << QString::number(val.y()).replace(",", ".").toUtf8().data()
        << "} ";
}

template<>
inline void LoggingMaster::LoggingHelper::fileWriteOnly(double val) {
    // Принудительная замена с игнором локали
    auto resv = QString::number(val);
    resv = resv.replace(",", ".");

    LoggingMaster::getInstance().logfileStream << resv.toUtf8().data() << " ";
}

template<>
inline void LoggingMaster::LoggingHelper::fileWriteOnly(QVariant val) {
    LoggingMaster::getInstance().logfileStream << val.toString().toUtf8().data()
                                               << " ";
}
#endif // QT_CORE


}
