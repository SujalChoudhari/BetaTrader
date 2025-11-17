#pragma once

#include "fix/ExecutionReport.h"
#include <vector>

namespace fix {

    class ExecutionReportToBinaryConverter {
    public:
        static std::vector<char> convert(const ExecutionReport& executionReport);
    };

}
