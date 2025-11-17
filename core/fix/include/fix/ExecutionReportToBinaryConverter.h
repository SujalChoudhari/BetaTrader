#pragma once

#include "fix/ExecutionReport.h"
#include <vector>

namespace fix {

    /**
     * @class ExecutionReportToBinaryConverter
     * @brief A static utility class for serializing an `ExecutionReport` into a FIX message.
     *
     * This class handles the conversion of a structured `ExecutionReport` object
     * into a raw binary FIX message (Execution Report, 35=8).
     */
    class ExecutionReportToBinaryConverter {
    public:
        /**
         * @brief Converts an `ExecutionReport` object into a raw FIX message.
         * @param executionReport The report object to serialize.
         * @return A vector of characters containing the complete, valid FIX message.
         */
        static std::vector<char> convert(const ExecutionReport& executionReport);
    };

} // namespace fix
