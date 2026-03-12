#pragma once 
#ifndef COMPONENTS_IS_ENABLED_QT

#include <iostream>

#include "../instancebase.hpp"
#include "filewriter.hpp"

namespace LoggerNoQt {
using namespace Logger;

/**
 * @brief The Instance class Мастер вывода информации (логов). Синглетон
 */
class Instance final : public InstanceBase {
    template <typename T>
    void printLog(T&& v) {
        std::cout << v << " ";
    }

    template <typename T>
    void printErr(T&& v) {
        std::cerr << v << " ";
    }

public:
    ~Instance();

    /**
     * @brief log Вывести данные в потоке логгирования. Для синхронного вывода
     * укажите isSync как true
     * @param args Данные на вывод
     */
    template<Level lt, bool isSync, typename... Args>
    void log(Args&&... args) {
        auto task = [=]() {
            auto timestamp = getTimestamp();
            if constexpr (lt != Level::Empty && lt != Level::Error && lt != Level::Warning) {
                printLog(timestamp + " [" + createLogtypeColoredString<lt>() + "] ");
            }

            if constexpr (lt == Level::Error || lt == Level::Warning) {
                printErr(timestamp + " [" + createLogtypeColoredString<lt>() + "] ");
                (printErr(args), ...);
            } else {
                (printLog(args), ...);
            }

            if constexpr (lt != Level::Empty) {
                m_logfileWriter.log<lt>(timestamp + " [" + createLogtypeString<lt>() + "] ", args...);
            } else {
                m_logfileWriter.log<lt>(args...);
            }

            if constexpr (lt == Level::Error || lt == Level::Warning) {
                std::cerr << std::endl;
            } else {
                std::cout << std::endl;
            }
        };

        if constexpr (isSync) {
            addTaskSync(std::move(task));
        } else {
            addTask(std::move(task));
        }
    }

    FileWriter& getFilewriter();
    
private:
    void init(const std::string& logfileDir) override;
    FileWriter m_logfileWriter; //! Мастер записи данных в файл
};

template <>
inline void Instance::printLog(const bool& v) {
    std::cout << (v ? "true" : "false") << " ";
}

}

#endif // COMPONENTS_IS_ENABLED_QT
