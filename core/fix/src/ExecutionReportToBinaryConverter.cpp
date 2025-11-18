#include "fix/ExecutionReportToBinaryConverter.h"
#include "common/Instrument.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include "logging/Logger.h"
#include <chrono>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace fix {
    namespace {
        char orderStatusToChar(const common::OrderStatus status)
        {
            switch (status) {
            case common::OrderStatus::New: return ORDER_STATUS_NEW;
            case common::OrderStatus::PartiallyFilled: return ORDER_STATUS_PARTIALLY_FILLED;
            case common::OrderStatus::Filled: return ORDER_STATUS_FILLED;
            case common::OrderStatus::Cancelled: return ORDER_STATUS_CANCELED;
            case common::OrderStatus::Rejected: return ORDER_STATUS_REJECTED;
            }
            throw std::invalid_argument("Invalid OrderStatus");
        }

        char orderSideToChar(const common::OrderSide side)
        {
            if (side == common::OrderSide::Buy) return ORDER_SIDE_BUY;
            if (side == common::OrderSide::Sell) return ORDER_SIDE_SELL;
            throw std::invalid_argument("Invalid OrderSide");
        }

        std::string timestampToString(const common::Timestamp& ts)
        {
            const auto timeT = std::chrono::system_clock::to_time_t(ts);
            std::tm tm_buf;
            gmtime_s(&tm_buf, &timeT);
            std::stringstream ss;
            ss << std::put_time(&tm_buf, "%Y%m%d-%H:%M:%S");
            const auto duration = ts.time_since_epoch();
            const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
            ss << '.' << std::setfill('0') << std::setw(3) << millis.count();
            return ss.str();
        }
    }

    std::string ExecutionReportToBinaryConverter::convert(const ExecutionReport& report)
    {
        LOG_INFO("Starting conversion for ExecutionReport, OrderID: {}", report.getExchangeOrderId());

        std::stringstream bodyStream;
        bodyStream << static_cast<int>(Tag::OrderID) << "=" << report.getExchangeOrderId() << SOH;
        bodyStream << static_cast<int>(Tag::ClOrdID) << "=" << report.getClientOrderId() << SOH;
        bodyStream << static_cast<int>(Tag::ExecID) << "=" << report.getExecutionId() << SOH;
        bodyStream << static_cast<int>(Tag::OrdStatus) << "=" << orderStatusToChar(report.getStatus()) << SOH;
        if (!report.getText().empty()) {
            bodyStream << static_cast<int>(Tag::Text) << "=" << report.getText() << SOH;
        }
        bodyStream << static_cast<int>(Tag::Symbol) << "=" << common::to_string(report.getSymbol()) << SOH;
        bodyStream << static_cast<int>(Tag::Side) << "=" << orderSideToChar(report.getSide()) << SOH;
        bodyStream << static_cast<int>(Tag::OrderQty) << "=" << report.getOrderQuantity() << SOH;
        bodyStream << static_cast<int>(Tag::CumQty) << "=" << report.getCumulativeQuantity() << SOH;
        bodyStream << static_cast<int>(Tag::LeavesQty) << "=" << report.getLeavesQuantity() << SOH;
        bodyStream << static_cast<int>(Tag::LastPx) << "=" << std::fixed << std::setprecision(5) << report.getLastPrice() << SOH;
        bodyStream << static_cast<int>(Tag::LastQty) << "=" << report.getLastQuantity() << SOH;
        bodyStream << static_cast<int>(Tag::TransactTime) << "=" << timestampToString(report.getTransactionTime()) << SOH;
        
        std::string bodyString = bodyStream.str();
        LOG_INFO("  Body String: '{}', Length: {}", bodyString, bodyString.length());

        std::stringstream headerStream;
        headerStream << static_cast<int>(Tag::MsgType) << "=" << MSG_TYPE_EXECUTION_REPORT << SOH;
        headerStream << static_cast<int>(Tag::SenderCompID) << "=" << report.getSenderCompId() << SOH;
        headerStream << static_cast<int>(Tag::TargetCompID) << "=" << report.getTargetCompId() << SOH;
        headerStream << static_cast<int>(Tag::MsgSeqNum) << "=" << report.getMessageSequenceNumber() << SOH;
        headerStream << static_cast<int>(Tag::SendingTime) << "=" << timestampToString(std::chrono::system_clock::now()) << SOH;

        std::string headerString = headerStream.str();
        size_t bodyLength = headerString.length() + bodyString.length();
        LOG_INFO("  Calculated BodyLength(9): {} (Header: {} + Body: {})", bodyLength, headerString.length(), bodyString.length());

        std::stringstream finalMessageStream;
        finalMessageStream << static_cast<int>(Tag::BeginString) << "=" << FIX_BEGIN_STRING << SOH;
        finalMessageStream << static_cast<int>(Tag::BodyLength) << "=" << bodyLength << SOH;
        finalMessageStream << headerString << bodyString;

        std::string messageSansChecksum = finalMessageStream.str();
        
        unsigned int checksum = 0;
        for (char c : messageSansChecksum) {
            checksum += static_cast<unsigned int>(c);
        }
        checksum %= 256;
        LOG_INFO("  Calculated Checksum(10): {}", checksum);

        finalMessageStream << static_cast<int>(Tag::CheckSum) << "=" << std::setfill('0') << std::setw(3) << checksum << SOH;
        
        std::string finalMessage = finalMessageStream.str();
        LOG_INFO("Serialized ExecutionReport: {}", finalMessage);
        return finalMessage;
    }
} // namespace fix
