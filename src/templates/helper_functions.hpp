#pragma once

#include "helper_structures.hpp"

namespace Logger
{

template <typename StreamT, typename T>
void writeData(StreamT&& oStream, T&& oValue) {
    using BaseT = std::decay_t<T>;
    if constexpr (IsMap<BaseT>::value) {
        writeData(oStream, "{");
        if (oValue.empty()) {
            writeData(oStream, "}");
            return;
        }

        auto pIt = oValue.begin();
        writeData(oStream, pIt->first);
        writeData(oStream, ":");
        writeData(oStream, pIt->second);
        std::advance(pIt, 1);

        while (pIt != oValue.end()) {
            writeData(oStream, "|");
            writeData(oStream, pIt->first);
            writeData(oStream, ":");
            writeData(oStream, pIt->second);
            std::advance(pIt, 1);
        }
        writeData(oStream, "}");

    } else if constexpr (IsContainer<BaseT>::value) {
        writeData(oStream, "{");
        if (oValue.empty()) {
            writeData(oStream, "}");
            return;
        }

        auto pIt = oValue.begin();
        writeData(oStream, *pIt);
        std::advance(pIt, 1);

        while (pIt != oValue.end()) {
            writeData(oStream, "|");
            writeData(oStream, *pIt);
            std::advance(pIt, 1);
        }
        writeData(oStream, "}");
    } else if constexpr(std::is_same_v<BaseT, bool>) {
        oStream << (oValue ? "true" : "false") << " ";
    } else {
        oStream << oValue << " ";
    }
}

}