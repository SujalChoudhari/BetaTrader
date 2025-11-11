#pragma once

#include "fix/OrderResponse.h"
#include <vector>

namespace fix {

    class OrderResponseToBinaryConverter {
    public:
        static std::vector<char> convert(const OrderResponse& orderResponse);
    };

} // namespace fix
