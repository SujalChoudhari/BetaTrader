#include "data/SequenceRepository.h"
#include <cstdint>
#include <string>

namespace fix {

    struct SessionState {
        bool isLoggedOn = false;
        uint32_t inSeqNum = 0;
        uint32_t outSeqNum = 0;
        std::string senderCompId = "";
    };

} // namespace fix