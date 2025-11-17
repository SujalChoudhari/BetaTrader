#pragma once

#include "fix/ExecutionReport.h"
#include <string> // Changed from <vector>

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
         * @return A string containing the complete, valid FIX message.
         */
        static std::string convert(const ExecutionReport& executionReport); // Changed return type
    };

} // namespace fix
