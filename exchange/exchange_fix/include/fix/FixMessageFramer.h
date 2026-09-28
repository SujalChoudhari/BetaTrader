#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace fix {

    enum class FrameStatus {
        NeedMoreData,
        Complete,
        Malformed,
    };

    struct FrameResult {
        FrameStatus status = FrameStatus::NeedMoreData;
        std::size_t consumed = 0;
        std::string frame;
        std::string error;
    };

    /**
     * Extract one complete FIX frame from the front of a receive buffer.
     *
     * BodyLength (Tag 9) determines where the checksum field begins. The
     * checksum is validated over the bytes before Tag 10, so tag-looking text
     * in payload values cannot terminate the frame early.
     */
    FrameResult extractNextFrame(std::string_view buffer);

} // namespace fix
