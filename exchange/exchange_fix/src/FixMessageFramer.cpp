#include "fix/FixMessageFramer.h"
#include "common_fix/Protocol.h"
#include <cctype>
#include <charconv>
#include <limits>
#include <utility>

namespace fix {
    namespace {

        FrameResult malformed(std::string error)
        {
            return FrameResult{FrameStatus::Malformed, 0, {}, std::move(error)};
        }

        bool parseUnsigned(std::string_view value, std::size_t& result)
        {
            if (value.empty()) return false;
            for (const unsigned char character: value) {
                if (!std::isdigit(character)) return false;
            }

            const auto* first = value.data();
            const auto* last = first + value.size();
            const auto parsed = std::from_chars(first, last, result);
            return parsed.ec == std::errc{} && parsed.ptr == last;
        }

        unsigned int checksum(std::string_view message)
        {
            unsigned int sum = 0;
            for (const unsigned char character: message) sum += character;
            return sum % 256;
        }

    } // namespace

    FrameResult extractNextFrame(const std::string_view buffer)
    {
        if (buffer.empty()) return {};
        if (buffer.size() < 2) return {};
        if (buffer.substr(0, 2) != "8=") {
            return malformed("FIX frame must start with BeginString (Tag 8)");
        }

        const auto beginStringEnd = buffer.find(SOH);
        if (beginStringEnd == std::string_view::npos) return {};
        if (buffer.substr(0, beginStringEnd) != "8=FIX.4.4") {
            return malformed("unsupported FIX BeginString");
        }

        const auto bodyLengthStart = beginStringEnd + 1;
        if (buffer.size() < bodyLengthStart + 2) return {};
        if (buffer.substr(bodyLengthStart, 2) != "9=") {
            return malformed("FIX frame is missing BodyLength (Tag 9)");
        }

        const auto bodyLengthEnd = buffer.find(SOH, bodyLengthStart + 2);
        if (bodyLengthEnd == std::string_view::npos) return {};

        std::size_t bodyLength = 0;
        if (!parseUnsigned(buffer.substr(bodyLengthStart + 2,
                                         bodyLengthEnd - bodyLengthStart - 2),
                           bodyLength)) {
            return malformed("FIX BodyLength is not an unsigned decimal value");
        }

        const auto bodyStart = bodyLengthEnd + 1;
        if (bodyLength > std::numeric_limits<std::size_t>::max() - bodyStart) {
            return malformed("FIX BodyLength overflows the receive buffer");
        }
        const auto checksumStart = bodyStart + bodyLength;
        constexpr std::size_t checksumFieldLength = 7; // 10=ddd<SOH>
        if (buffer.size() < checksumStart + 3) return {};

        if (buffer.substr(checksumStart, 3) != "10=") {
            return malformed(
                    "FIX checksum field is not at the BodyLength boundary");
        }
        if (buffer.size() < checksumStart + checksumFieldLength) return {};
        const auto checksumDigits = buffer.substr(checksumStart + 3, 3);
        std::size_t expectedChecksum = 0;
        if (!parseUnsigned(checksumDigits, expectedChecksum)
            || buffer[checksumStart + 6] != SOH) {
            return malformed("FIX checksum field is malformed");
        }

        const auto frameLength = checksumStart + checksumFieldLength;
        if (checksum(buffer.substr(0, checksumStart)) != expectedChecksum) {
            return malformed("FIX checksum validation failed");
        }

        return FrameResult{FrameStatus::Complete,
                           frameLength,
                           std::string(buffer.substr(0, frameLength)),
                           {}};
    }

} // namespace fix
