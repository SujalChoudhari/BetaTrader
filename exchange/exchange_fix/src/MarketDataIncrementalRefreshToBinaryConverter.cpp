#include "fix/MarketDataIncrementalRefreshToBinaryConverter.h"
#include "common/Types.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include "logging/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fix
{

std::string MarketDataIncrementalRefreshToBinaryConverter::convert(const MarketDataIncrementalRefresh& refresh, uint32_t msgSeqNum)
{
    LOG_INFO("Starting conversion for MarketDataIncrementalRefresh, MDReqID: {}", refresh.mdReqID);

    std::stringstream bodyStream;

    if (!refresh.mdReqID.empty()) {
        bodyStream << static_cast<int>(fix::Tag::MDReqID) << "=" << refresh.mdReqID << fix::SOH;
        LOG_INFO("  Body += MDReqID(262): {}", refresh.mdReqID);
    }
    bodyStream << static_cast<int>(fix::Tag::NoMDEntries) << "=" << refresh.entries.size() << fix::SOH;
    LOG_INFO("  Body += NoMDEntries(268): {}", refresh.entries.size());

    for (const auto& entry : refresh.entries)
    {
        bodyStream << static_cast<int>(fix::Tag::MDUpdateAction) << "=" << static_cast<char>(entry.updateAction) << fix::SOH;
        bodyStream << static_cast<int>(fix::Tag::MDEntryType) << "=" << static_cast<char>(entry.entryType) << fix::SOH;
        bodyStream << static_cast<int>(fix::Tag::Symbol) << "=" << common::to_string(refresh.symbol) << fix::SOH;
        bodyStream << static_cast<int>(fix::Tag::MDEntryPx) << "=" << std::fixed << std::setprecision(4) << entry.price << fix::SOH;
        bodyStream << static_cast<int>(fix::Tag::MDEntrySize) << "=" << entry.size << fix::SOH;
        
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm tm_buf;
        gmtime_r(&time_t_now, &tm_buf);

        bodyStream << static_cast<int>(fix::Tag::MDEntryTime) << "=" 
                   << std::put_time(&tm_buf, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count()
                   << fix::SOH;
        LOG_INFO("    Entry: Action={}, Type={}, Symbol={}, Px={}, Size={}", static_cast<char>(entry.updateAction), static_cast<char>(entry.entryType), common::to_string(refresh.symbol), entry.price, entry.size);
    }

    std::string bodyString = bodyStream.str();
    LOG_INFO("Final Body String (Before Header Calc): '{}', Length: {}", bodyString, bodyString.length());
    
    std::stringstream headerStream;
    headerStream << static_cast<int>(fix::Tag::MsgType) << "=" << fix::MSG_TYPE_MARKET_DATA_INCREMENTAL_REFRESH << fix::SOH;
    headerStream << static_cast<int>(fix::Tag::SenderCompID) << "=" << "TRADING_CORE" << fix::SOH;
    headerStream << static_cast<int>(fix::Tag::TargetCompID) << "=" << refresh.targetSessionID << fix::SOH;
    headerStream << static_cast<int>(fix::Tag::MsgSeqNum) << "=" << msgSeqNum << fix::SOH;
    
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tm_buf;
    gmtime_r(&time_t_now, &tm_buf);
    headerStream << static_cast<int>(fix::Tag::SendingTime) << "=" 
                  << std::put_time(&tm_buf, "%Y%m%d-%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count()
                  << fix::SOH;

    std::string headerString = headerStream.str();
    size_t bodyLength = headerString.length() + bodyString.length();
    LOG_INFO("Calculated BodyLength(9): {} (Header: {} + Body: {})", bodyLength, headerString.length(), bodyString.length());

    std::stringstream finalMessageStream;
    finalMessageStream << static_cast<int>(fix::Tag::BeginString) << "=" << "FIXT.1.1" << fix::SOH;
    finalMessageStream << static_cast<int>(fix::Tag::BodyLength) << "=" << bodyLength << fix::SOH;
    finalMessageStream << headerString << bodyString;
    
    std::string messageSansChecksum = finalMessageStream.str();

    unsigned int checksum = 0;
    for (char c : messageSansChecksum) {
        checksum += static_cast<unsigned int>(c);
    }
    checksum %= 256;
    LOG_INFO("Calculated Checksum(10): {}", checksum);

    finalMessageStream << static_cast<int>(fix::Tag::CheckSum) << "=" << std::setfill('0') << std::setw(3) << checksum << fix::SOH;

    std::string finalMessage = finalMessageStream.str();
    LOG_INFO("Serialized MarketDataIncrementalRefresh: {}", finalMessage);
    return finalMessage;
}

} // namespace fix
