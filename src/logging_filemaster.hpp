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

#ifdef LOGGER_USE_QT
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPoint>
#else
#include <fstream>
#endif // LOGGER_USE_QT

#include "logging_common.hpp"


namespace Logging
{

class LoggingFileMaster
{
#ifdef LOGGER_USE_QT
    QFile logfile;                        //! Логфайл
    QTextStream logfileStream{&logfile};  //! Поток ввода в файл данных
#else
    std::string logfilePath;    //! Путь до логфайла
    std::fstream logfile;       //! Логфайл
#endif // LOGGER_USE_QT

#ifdef LOGGER_USE_QT
   template <typename T>
   void writeToFile(const T& v) {
       logfileStream << v << " ";
   }
#else
   template <typename T>
   void writeToFile(const T& v) {
       logfile << v << " ";
   }

#endif // LOGGER_USE_QT


   void addEndline() {
#ifdef LOGGER_USE_QT
       logfileStream << Qt::endl;
#else
       logfile << std::endl;
#endif // LOGGER_USE_QT
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
#ifdef LOGGER_USE_QT
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
#endif // LOGGER_USE_QT
       (writeToFile(args), ...);
        addEndline();

       logfile.flush();
       logfile.close();
   }
};

#ifdef LOGGER_USE_QT
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
#endif // LOGGER_USE_QT

}
