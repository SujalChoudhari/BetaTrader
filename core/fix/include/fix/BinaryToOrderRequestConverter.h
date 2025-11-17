#pragma once

#include "fix/OrderRequest.h"
#include <vector>

namespace fix {

    class BinaryToOrderRequestConverter {
    public:
        static OrderRequest convert(const std::vector<char>& binaryData);
    };

}
