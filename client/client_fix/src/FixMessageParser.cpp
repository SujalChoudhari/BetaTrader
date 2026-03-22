#include "fix_client/FixMessageParser.h"
#include "common_fix/FixUtils.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include "logging/Logger.h"
#include <map>
#include <numeric>
#include <stdexcept>

namespace fix_client {

    ParsedFixMessage FixMessageParser::parse(const std::string& fixMessage) {
        try {
            const std::string_view messageStringView(fixMessage);

            // 1. Basic Checksum Validation
            const std::string checksumTag = std::string(1, fix::SOH) + std::to_string(static_cast<int>(fix::Tag::CheckSum)) + "=";
            const size_t checksumPos = messageStringView.rfind(checksumTag);
            if (checksumPos == std::string_view::npos) {
                LOG_WARN("FixMessageParser::parse - FIX message does not contain a checksum: {}", fixMessage.substr(0, 100));
                return std::monostate{};
            }

            const std::string_view messageUpToChecksum = messageStringView.substr(0, checksumPos + 1);
            const int expectedChecksum = std::stoi(std::string(messageStringView.substr(checksumPos + checksumTag.length())));

            unsigned int calculatedChecksum = 0;
            for (const char c : messageUpToChecksum) {
                calculatedChecksum += static_cast<unsigned int>(c);
            }
            calculatedChecksum %= 256;

            if (calculatedChecksum != expectedChecksum) {
                LOG_WARN("FixMessageParser::parse - Checksum validation failed. Expected: {}, Calculated: {}.", expectedChecksum, calculatedChecksum);
                return std::monostate{};
            }

            // 2. Extract Tag 35 (MsgType)
            const auto tagMap = fix::splitToMap(messageStringView, fix::SOH);
            auto msgTypeIt = tagMap.find(static_cast<int>(fix::Tag::MsgType));
            if (msgTypeIt == tagMap.end() || msgTypeIt->second.empty()) {
                LOG_WARN("FixMessageParser::parse - No MsgType (35) found.");
                return std::monostate{};
            }

            char msgType = msgTypeIt->second[0];

            // 3. Dispatch to specific parsers
            if (msgType == fix::MSG_TYPE_EXECUTION_REPORT) {
                auto report = parseExecutionReport(fixMessage);
                if (report) return *report;
            } else if (msgType == fix::MSG_TYPE_MARKET_DATA_SNAPSHOT_FULL_REFRESH) {
                auto snapshot = parseMarketDataSnapshot(fixMessage);
                if (snapshot) return *snapshot;
            } else if (msgType == fix::MSG_TYPE_MARKET_DATA_INCREMENTAL_REFRESH) {
                auto refresh = parseMarketDataIncremental(fixMessage);
                if (refresh) return *refresh;
            }

            // Fallback for unhandled or successfully ignored message types (Logon Ack, Heartbeat, etc)
            return std::monostate{};

        } catch (const std::exception& e) {
            LOG_ERROR("FixMessageParser::parse - Exception parsing message: {}", e.what());
            return std::monostate{};
        }
    }

    std::optional<fix::ExecutionReport> FixMessageParser::parseExecutionReport(const std::string& fixMessage) {
        try {
            const auto tagMap = fix::splitToMap(fixMessage, fix::SOH);

            // Extract string views or copy to string before conversion
            auto getStr = [&](fix::Tag tag) -> std::string {
                auto it = tagMap.find(static_cast<int>(tag));
                return (it != tagMap.end()) ? std::string(it->second) : "";
            };

            std::string senderCompIdStr = getStr(fix::Tag::SenderCompID);
            std::string targetCompIdStr = getStr(fix::Tag::TargetCompID);
            
            // Note: SenderCompId on the wire is actual string, e.g. "BETA_EXCHANGE"
            // The constructor expects uint32_t CompID for Sender/Target, assuming numeric ID if applicable,
            // or 0 if we ignore it on the client side since we know it's the exchange.
            // For now, parsing strings into integer CompID might throw if they aren't numbers. 
            // The server uses std::string senderCompId in outBuilders, so we will use 0 for now as a placeholder 
            // since the struct `ExecutionReport` expects `uint32_t` `CompID`. 
            fix::CompID senderCompId = 0; 
            fix::CompID targetCompId = 0; 

            uint64_t msgSeqNum = std::stoull(getStr(fix::Tag::MsgSeqNum));
            uint64_t exchangeOrderId = std::stoull(getStr(fix::Tag::OrderID));
            
            std::string clOrdIdStr = getStr(fix::Tag::ClOrdID);
            uint64_t clientOrderId = clOrdIdStr.empty() ? 0 : std::stoull(clOrdIdStr);
            
            std::string execId = getStr(fix::Tag::ExecID);
            
            char ordStatusChar = getStr(fix::Tag::OrdStatus)[0];
            common::OrderStatus status;
            switch(ordStatusChar) {
                case fix::ORDER_STATUS_NEW: status = common::OrderStatus::New; break;
                case fix::ORDER_STATUS_PARTIALLY_FILLED: status = common::OrderStatus::PartiallyFilled; break;
                case fix::ORDER_STATUS_FILLED: status = common::OrderStatus::Filled; break;
                case fix::ORDER_STATUS_CANCELED: status = common::OrderStatus::Cancelled; break;
                case fix::ORDER_STATUS_REJECTED: status = common::OrderStatus::Rejected; break;
                default: throw std::invalid_argument("Unknown OrderStatus char");
            }
            
            std::string text = getStr(fix::Tag::Text);
            common::Instrument symbol = common::from_string(getStr(fix::Tag::Symbol));
            common::OrderSide side = fix::charToOrderSide(getStr(fix::Tag::Side)[0]);
            
            uint64_t orderQty = std::stoull(getStr(fix::Tag::OrderQty));
            uint64_t cumQty = std::stoull(getStr(fix::Tag::CumQty));
            uint64_t leavesQty = std::stoull(getStr(fix::Tag::LeavesQty));
            
            std::string lastPxStr = getStr(fix::Tag::LastPx);
            double lastPrice = lastPxStr.empty() ? 0.0 : std::stod(lastPxStr);
            
            std::string lastQtyStr = getStr(fix::Tag::LastQty);
            uint64_t lastQty = lastQtyStr.empty() ? 0 : std::stoull(lastQtyStr);
            
            common::Timestamp transactTime = fix::parseTimestamp(getStr(fix::Tag::TransactTime));

            return fix::ExecutionReport(senderCompId, targetCompId, msgSeqNum, exchangeOrderId, clientOrderId,
                                        execId, status, text, symbol, side, orderQty, cumQty, leavesQty,
                                        lastPrice, lastQty, transactTime);
        } catch (const std::exception& e) {
            LOG_ERROR("FixMessageParser::parseExecutionReport - Failed: {}", e.what());
            return std::nullopt;
        }
    }

    std::optional<fix::MarketDataSnapshotFullRefresh> FixMessageParser::parseMarketDataSnapshot(const std::string& fixMessage) {
        try {
            const auto tagMap = fix::splitToMap(fixMessage, fix::SOH);
            auto getStr = [&](fix::Tag tag) -> std::string {
                auto it = tagMap.find(static_cast<int>(tag));
                return (it != tagMap.end()) ? std::string(it->second) : "";
            };

            fix::MarketDataSnapshotFullRefresh msg;
            msg.mdReqID = getStr(fix::Tag::MDReqID);
            msg.symbol = common::from_string(getStr(fix::Tag::Symbol));
            msg.targetSessionID = 0; // Not explicitly serialized in FIX, tracked internally

            std::string noMdEntriesStr = getStr(fix::Tag::NoMDEntries);
            if (noMdEntriesStr.empty()) return msg; 
            
            int noMdEntries = std::stoi(noMdEntriesStr);
            
            // To parse repeating groups correctly, we need to iterate over the message string sequentially
            // since splitToMap overwrites duplicate keys. For MarketData, we will do a linear scan.
            
            const std::string_view msgView(fixMessage);
            size_t pos = 0;
            
            // Find start of repeating group (Tag 268 NoMDEntries)
            std::string startTag = "268=" + noMdEntriesStr + "\x01";
            pos = msgView.find(startTag);
            if (pos == std::string_view::npos) return msg;
            pos += startTag.length();

            for (int i = 0; i < noMdEntries; ++i) {
                fix::MarketDataEntry entry{};
                
                // MDEntryType (269) is the first tag of the repeating group
                size_t typePos = msgView.find("269=", pos);
                if (typePos != std::string_view::npos) {
                    entry.entryType = static_cast<fix::MDEntryType>(msgView[typePos + 4]);
                    pos = typePos + 6; // Move past "269=X\x01"
                }

                // Followed by 270=Px, 271=Size, 290=Position, 273=Time (order not strictly guaranteed but typical)
                // A safer iterative approach until the next 269:
                size_t next269 = msgView.find("269=", pos);
                if (next269 == std::string_view::npos) next269 = msgView.length();
                
                auto blockStr = std::string(msgView.substr(pos, next269 - pos));
                auto blockMap = fix::splitToMap(blockStr, fix::SOH);
                
                auto itPx = blockMap.find(static_cast<int>(fix::Tag::MDEntryPx));
                if (itPx != blockMap.end()) entry.price = std::stod(std::string(itPx->second));
                
                auto itSize = blockMap.find(static_cast<int>(fix::Tag::MDEntrySize));
                if (itSize != blockMap.end()) entry.size = std::stoull(std::string(itSize->second));
                
                auto itPos = blockMap.find(static_cast<int>(fix::Tag::MDEntryPositionNo));
                if (itPos != blockMap.end()) entry.entryPosition = std::stoi(std::string(itPos->second));
                
                // Ignoring 273 (Time) since the common server converter usually doesn't send it or sets it to "now"
                
                msg.entries.push_back(entry);
                pos = next269;
            }

            return msg;
        } catch (const std::exception& e) {
            LOG_ERROR("FixMessageParser::parseMarketDataSnapshot - Failed: {}", e.what());
            return std::nullopt;
        }
    }

    std::optional<fix::MarketDataIncrementalRefresh> FixMessageParser::parseMarketDataIncremental(const std::string& fixMessage) {
        try {
            const auto tagMap = fix::splitToMap(fixMessage, fix::SOH);
            auto getStr = [&](fix::Tag tag) -> std::string {
                auto it = tagMap.find(static_cast<int>(tag));
                return (it != tagMap.end()) ? std::string(it->second) : "";
            };

            fix::MarketDataIncrementalRefresh msg;
            msg.mdReqID = getStr(fix::Tag::MDReqID);

            std::string noMdEntriesStr = getStr(fix::Tag::NoMDEntries);
            if (noMdEntriesStr.empty()) return msg; 
            
            int noMdEntries = std::stoi(noMdEntriesStr);
            
            // Linear scan for repeating groups
            const std::string_view msgView(fixMessage);
            size_t pos = 0;
            
            std::string startTag = "268=" + noMdEntriesStr + "\x01";
            pos = msgView.find(startTag);
            if (pos == std::string_view::npos) return msg;
            pos += startTag.length();

            for (int i = 0; i < noMdEntries; ++i) {
                fix::MarketDataIncrementalEntry entry{};
                
                // MDUpdateAction (279) is the first tag of the repeating group
                size_t actionPos = msgView.find("279=", pos);
                if (actionPos != std::string_view::npos) {
                    entry.updateAction = static_cast<fix::MDUpdateAction>(msgView[actionPos + 4]);
                    pos = actionPos + 6;
                }

                size_t next279 = msgView.find("279=", pos);
                if (next279 == std::string_view::npos) next279 = msgView.length();
                
                auto blockStr = std::string(msgView.substr(pos, next279 - pos));
                auto blockMap = fix::splitToMap(blockStr, fix::SOH);
                
                // Note: The symbol is often sent per entry in Incremental (Tag 55).
                auto itSym = blockMap.find(static_cast<int>(fix::Tag::Symbol));
                if (itSym != blockMap.end()) msg.symbol = common::from_string(itSym->second);

                auto itType = blockMap.find(static_cast<int>(fix::Tag::MDEntryType));
                if (itType != blockMap.end()) entry.entryType = static_cast<fix::MDEntryType>(itType->second[0]);

                auto itPx = blockMap.find(static_cast<int>(fix::Tag::MDEntryPx));
                if (itPx != blockMap.end()) entry.price = std::stod(std::string(itPx->second));
                
                auto itSize = blockMap.find(static_cast<int>(fix::Tag::MDEntrySize));
                if (itSize != blockMap.end()) entry.size = std::stoull(std::string(itSize->second));
                
                auto itPos = blockMap.find(static_cast<int>(fix::Tag::MDEntryPositionNo));
                if (itPos != blockMap.end()) entry.entryPosition = std::stoi(std::string(itPos->second));
                
                msg.entries.push_back(entry);
                pos = next279;
            }

            return msg;
        } catch (const std::exception& e) {
            LOG_ERROR("FixMessageParser::parseMarketDataIncremental - Failed: {}", e.what());
            return std::nullopt;
        }
    }

} // namespace fix_client
