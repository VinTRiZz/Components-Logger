#pragma once

#include <string>

#ifdef COMPONENTS_LOGGER_USE_QT
#include <QDateTime>
#else
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#endif // COMPONENTS_LOGGER_USE_QT


namespace Logging
{


/**
 * @brief The LoggingType enum Тип данных для вывода
 */
enum class LoggingType { Empty, Debug, Info, Warning, Error, Ok };

/**
 * @brief logTypeStringColored Получение выделенного цветом текста для логов
 * @return Строка с управляющими символами
 */
template<LoggingType LogType>
constexpr const char* logTypeStringColored() {
    switch (LogType) {
        case LoggingType::Info:
            return "\033[37m INFO \033[0m";
        case LoggingType::Warning:
            return "\033[33m WARN \033[0m";

        case LoggingType::Ok:
            return "\033[32m  OK  \033[0m";
        case LoggingType::Error:
            return "\033[31m FAIL \033[0m";

        case LoggingType::Debug:
            return "\033[35m DEBG \033[0m";
    }
    return "";
}

/**
 * @brief logTypeString Получение не выделенного цветом текста для логов
 * @return Строка без управляющих последовательностей
 */
template<LoggingType LogType>
constexpr const char* logTypeString() {
    switch (LogType) {
        case LoggingType::Info:
            return " INFO ";
        case LoggingType::Warning:
            return " WARN ";

        case LoggingType::Ok:
            return "  OK  ";
        case LoggingType::Error:
            return " FAIL ";

        case LoggingType::Debug:
            return " DEBG ";
    }
    return "";
}

static std::string getLogfilename() {
#ifdef COMPONENTS_LOGGER_USE_QT
    return QDateTime::currentDateTime()
        .toString("yyyy-MM-dd_hh-mm-ss.log")
        .toStdString();
#else
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

    std::time_t now_c = std::chrono::system_clock::to_time_t(now_ms);
    std::tm now_tm;

    // Use localtime_s for thread safety (Windows)
#ifdef _WIN32
    localtime_s(&now_tm, &now_c);
#else
    // Use localtime_r for thread safety (POSIX)
    localtime_r(&now_c, &now_tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d_%H-%M-%S.log");

    return oss.str();
#endif // COMPONENTS_LOGGER_USE_QT
}


static std::string getCurrentTimestampFormatted() {
#ifdef COMPONENTS_LOGGER_USE_QT
    return QDateTime::currentDateTime()
        .toString("yyyy-MM-dd_hh:mm:ss")
        .toStdString();
#else
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

    std::time_t now_c = std::chrono::system_clock::to_time_t(now_ms);
    std::tm now_tm;

    // Use localtime_s for thread safety (Windows)
#ifdef _WIN32
    localtime_s(&now_tm, &now_c);
#else
    // Use localtime_r for thread safety (POSIX)
    localtime_r(&now_c, &now_tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3)
        << (now_ms.time_since_epoch().count() % 1000);

    return oss.str();
#endif // COMPONENTS_LOGGER_USE_QT
}

}
