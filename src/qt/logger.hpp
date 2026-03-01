#pragma once
#ifdef COMPONENTS_IS_ENABLED_QT

#include <QDebug>

#include "../instancebase.hpp"
#include "filewriter.hpp"

namespace LoggerQt {
using namespace Logger;

/**
 * @brief The Instance class Мастер вывода информации (логов). Синглетон
 */
class Instance : public InstanceBase {
    template <typename T>
    void printLog(const T& v, QDebug& dbgStream) {
        dbgStream << v;
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
            auto dbgStream = qDebug();
            if constexpr (lt != Level::Empty) {
                printLog(timestamp + " [" + createLogtypeColoredString<lt>() + "] ", dbgStream);
            }
            (printLog(args, dbgStream), ...);

            if constexpr (lt != Level::Empty) {
                m_logfileWriter.log<lt>(timestamp + " [" + createLogtypeString<lt>() + "] ", args...);
            } else {
                m_logfileWriter.log<lt>(args...);
            }
        };

        if constexpr (isSync) {
            task();
        } else {
            addTask(std::move(task));
        }
    }

    FileWriter& getFilewriter();

private:
    FileWriter m_logfileWriter; //! Мастер записи данных в файл

    // InstanceBase interface
    void init(const std::string &logfileDir) override;
};

template <>
inline void Instance::printLog(const std::string& v, QDebug& dbgStream) {
    dbgStream << v.c_str();
}

}

#endif // COMPONENTS_IS_ENABLED_QT
