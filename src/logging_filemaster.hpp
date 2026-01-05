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
#include <QFile>
#include <QPoint>
#else
#include <fstream>
#endif // QT_CORE_LIB

#include "logging_common.hpp"


namespace Logging
{

class LoggingFileMaster
{
#ifdef QT_CORE_LIB
    QFile logfile;                        //! Логфайл
    QTextStream logfileStream{&logfile};  //! Поток ввода в файл данных
#else
    std::string logfilePath;    //! Путь до логфайла
    std::fstream logfile;       //! Логфайл
#endif // QT_CORE_LIB

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

#ifdef QT_CORE_LIB
   template <typename T>
   void writeToFile(const T& v) {
       logfileStream << v << " ";
   }
#else
   template <typename T>
   void writeToFile(const T& v) {
       logfile << v << " ";
   }

#endif // QT_CORE_LIB


   void addEndline() {
#ifdef QT_CORE_LIB
       logfileStream << Qt::endl;
#else
       logfile << std::endl;
#endif // QT_CORE_LIB
   }

public:
   LoggingFileMaster(const std::string& filePath);
   ~LoggingFileMaster();

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

#ifdef QT_CORE_LIB
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
#endif // QT_CORE_LIB
           (writeToFile(args), ...);
            addEndline();

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

#ifdef QT_CORE_LIB
template <>
inline void LoggingFileMaster::writeToFile(const std::string& v) {
    logfileStream << v.c_str() << " ";
}

template <>
inline void LoggingFileMaster::writeToFile(const QPoint& v) {
    logfileStream << "{" << v.x() << "; " << v.y() << "} ";
}

template <>
inline void LoggingFileMaster::writeToFile(const QPointF& v) {
    logfileStream << "{" << v.x() << "; " << v.y() << "} ";
}
#endif // QT_CORE_LIB

}
