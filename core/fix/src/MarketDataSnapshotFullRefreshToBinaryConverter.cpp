#include "fix/MarketDataSnapshotFullRefreshToBinaryConverter.h"
#include "common/Types.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include "logging/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fix
{

std::string MarketDataSnapshotFullRefreshToBinaryConverter::convert(const MarketDataSnapshotFullRefresh& snapshot)
{
    // TODO: Implement robust FIX message serialization. This is a simplified skeleton.
    // This function needs to be refactored to use a more structured approach for building FIX messages,
    // potentially a dedicated FIX message builder class, to ensure correctness of header, body, and trailer.

    std::stringstream bodyContentStream;

    bodyContentStream << static_cast<int>(fix::Tag::MDReqID) << "=" << snapshot.mdReqID << fix::SOH;
    bodyContentStream << static_cast<int>(fix::Tag::Symbol) << "=" << common::to_string(snapshot.symbol) << fix::SOH;
    bodyContentStream << static_cast<int>(fix::Tag::NoMDEntries) << "=" << snapshot.entries.size() << fix::SOH;

    for (const auto& entry : snapshot.entries)
    {
        bodyContentStream << static_cast<int>(fix::Tag::MDEntryType) << "=" << static_cast<char>(entry.entryType) << fix::SOH;
        bodyContentStream << static_cast<int>(fix::Tag::MDEntryPx) << "=" << std::fixed << std::setprecision(4) << entry.price << fix::SOH;
        bodyContentStream << static_cast<int>(fix::Tag::MDEntrySize) << "=" << std::fixed << std::setprecision(0) << entry.size << fix::SOH;
        bodyContentStream << static_cast<int>(fix::Tag::MDEntryPositionNo) << "=" << entry.entryPosition << fix::SOH;
        // TODO: Implement a proper timestamp to string conversion utility for FIX.
        bodyContentStream << static_cast<int>(fix::Tag::MDEntryTime) << "=" << "YYYYMMDD-HH:MM:SS.sss" << fix::SOH;
    }

    std::string bodyString = bodyContentStream.str();
    size_t bodyLength = bodyString.length();

    std::stringstream headerStringStream;
    headerStringStream << static_cast<int>(fix::Tag::BeginString) << "=" << fix::FIX_BEGIN_STRING << fix::SOH;
    headerStringStream << static_cast<int>(fix::Tag::BodyLength) << "=" << bodyLength << fix::SOH;
    headerStringStream << static_cast<int>(fix::Tag::MsgType) << "=" << fix::MSG_TYPE_MARKET_DATA_SNAPSHOT_FULL_REFRESH << fix::SOH;
    // TODO: Add SenderCompID (Tag 49), TargetCompID (Tag 56), MsgSeqNum (Tag 34) from the session context.
    // TODO: Implement a proper timestamp to string conversion utility for FIX.
    headerStringStream << static_cast<int>(fix::Tag::SendingTime) << "=" << "YYYYMMDD-HH:MM:SS.sss" << fix::SOH;

    std::string messageSansChecksum = headerStringStream.str() + bodyString;

    unsigned int checksum = 0;
    for (char c: messageSansChecksum) {
        checksum += static_cast<unsigned int>(c);
    }
    checksum %= 256;

    std::stringstream finalStringStream;
    finalStringStream << messageSansChecksum;
    finalStringStream << static_cast<int>(fix::Tag::CheckSum) << "="
                      << std::setfill('0') << std::setw(3) << checksum
                      << fix::SOH;

    LOG_INFO("MarketDataSnapshotFullRefreshToBinaryConverter::convert - Serialized MarketDataSnapshotFullRefresh (skeleton). MDReqID: {}", snapshot.mdReqID);
    return finalStringStream.str();
}

} // namespace fix
