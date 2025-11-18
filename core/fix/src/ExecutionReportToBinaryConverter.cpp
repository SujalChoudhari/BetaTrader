#include "fix/ExecutionReportToBinaryConverter.h"
#include "common/Instrument.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include <chrono>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace fix {
    namespace {
        // TODO: Consider making these helper functions private static members of ExecutionReportToBinaryConverter.
        char orderStatusToChar(const common::OrderStatus status)
        {
            switch (status) {
            case common::OrderStatus::New:
                return ORDER_STATUS_NEW;
            case common::OrderStatus::PartiallyFilled:
                return ORDER_STATUS_PARTIALLY_FILLED;
            case common::OrderStatus::Filled:
                return ORDER_STATUS_FILLED;
            case common::OrderStatus::Cancelled:
                return ORDER_STATUS_CANCELED;
            case common::OrderStatus::Rejected:
                return ORDER_STATUS_REJECTED;
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
            const std::tm tm = *std::gmtime(&timeT);
            std::stringstream ss;
            ss << std::put_time(&tm, "%Y%m%d-%H:%M:%S");
            const auto duration = ts.time_since_epoch();
            const auto millis
                    = std::chrono::duration_cast<std::chrono::milliseconds>(
                              duration)
                      % 1000;
            ss << '.' << std::setfill('0') << std::setw(3) << millis.count();
            return ss.str();
        }
    } // namespace

    std::string ExecutionReportToBinaryConverter::convert(
            const ExecutionReport& executionReport)
    {
        std::stringstream bodyStringStream;

        bodyStringStream << static_cast<int>(Tag::OrderID) << "="
                         << executionReport.getExchangeOrderId() << SOH;
        bodyStringStream << static_cast<int>(Tag::ClOrdID) << "="
                         << executionReport.getClientOrderId() << SOH;
        bodyStringStream << static_cast<int>(Tag::ExecID) << "="
                         << executionReport.getExecutionId() << SOH;
        bodyStringStream << static_cast<int>(Tag::OrdStatus) << "="
                         << orderStatusToChar(executionReport.getStatus())
                         << SOH;
        if (!executionReport.getText().empty()) {
            bodyStringStream << static_cast<int>(Tag::Text) << "="
                             << executionReport.getText() << SOH;
        }
        bodyStringStream << static_cast<int>(Tag::Symbol) << "="
                         << common::to_string(executionReport.getSymbol())
                         << SOH;
        bodyStringStream << static_cast<int>(Tag::Side) << "="
                         << orderSideToChar(executionReport.getSide()) << SOH;
        bodyStringStream << static_cast<int>(Tag::OrderQty) << "="
                         << executionReport.getOrderQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::CumQty) << "="
                         << executionReport.getCumulativeQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::LeavesQty) << "="
                         << executionReport.getLeavesQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::LastPx) << "=" << std::fixed
                         << std::setprecision(4)
                         << executionReport.getLastPrice() << SOH;
        bodyStringStream << static_cast<int>(Tag::LastQty) << "="
                         << executionReport.getLastQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::TransactTime) << "="
                         << timestampToString(
                                    executionReport.getTransactionTime())
                         << SOH;

        std::string bodyString = bodyStringStream.str();

        std::stringstream headerStringStream;
        headerStringStream << static_cast<int>(Tag::BeginString) << "="
                           << FIX_BEGIN_STRING << SOH;
        headerStringStream << static_cast<int>(Tag::BodyLength) << "="
                           << bodyString.length() << SOH;
        headerStringStream << static_cast<int>(Tag::MsgType) << "="
                           << MSG_TYPE_EXECUTION_REPORT << SOH;
        headerStringStream << static_cast<int>(Tag::SenderCompID) << "="
                           << executionReport.getSenderCompId() << SOH;
        headerStringStream << static_cast<int>(Tag::TargetCompID) << "="
                           << executionReport.getTargetCompId() << SOH;
        headerStringStream << static_cast<int>(Tag::MsgSeqNum) << "="
                           << executionReport.getMessageSequenceNumber() << SOH;
        headerStringStream << static_cast<int>(Tag::SendingTime) << "="
                           << timestampToString(
                                      std::chrono::system_clock::now())
                           << SOH;

        std::string headerStr = headerStringStream.str();
        std::string messageSansChecksum = headerStr + bodyString;

        unsigned int checksum = 0;
        for (char c: messageSansChecksum) {
            checksum += static_cast<unsigned int>(c);
        }
        checksum %= 256;

        std::stringstream finalStringStream;
        finalStringStream << messageSansChecksum;
        finalStringStream << static_cast<int>(Tag::CheckSum) << "="
                          << std::setfill('0') << std::setw(3) << checksum
                          << SOH;

        return finalStringStream.str();
    }
} // namespace fix
