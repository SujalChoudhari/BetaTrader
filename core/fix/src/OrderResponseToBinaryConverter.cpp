#include "fix/OrderResponseToBinaryConverter.h"
#include "common/Instrument.h"
#include "fix/Protocol.h"
#include "fix/Tags.h"
#include <chrono>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace fix {
    namespace {
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

    std::vector<char>
    OrderResponseToBinaryConverter::convert(const OrderResponse& orderResponse)
    {
        std::stringstream bodyStringStream;

        bodyStringStream << static_cast<int>(Tag::OrderID) << "="
                         << orderResponse.getExchangeOrderId() << SOH;
        bodyStringStream << static_cast<int>(Tag::ClOrdID) << "="
                         << orderResponse.getClientOrderId() << SOH;
        bodyStringStream << static_cast<int>(Tag::ExecID) << "="
                         << orderResponse.getExecutionId() << SOH;
        bodyStringStream << static_cast<int>(Tag::OrdStatus) << "="
                         << orderStatusToChar(orderResponse.getStatus()) << SOH;
        if (!orderResponse.getText().empty()) {
            bodyStringStream << static_cast<int>(Tag::Text) << "="
                             << orderResponse.getText() << SOH;
        }
        bodyStringStream << static_cast<int>(Tag::Symbol) << "="
                         << common::to_string(orderResponse.getSymbol()) << SOH;
        bodyStringStream << static_cast<int>(Tag::Side) << "="
                         << orderSideToChar(orderResponse.getSide()) << SOH;
        bodyStringStream << static_cast<int>(Tag::OrderQty) << "="
                         << orderResponse.getOrderQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::CumQty) << "="
                         << orderResponse.getCumulativeQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::LeavesQty) << "="
                         << orderResponse.getLeavesQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::LastPx) << "=" << std::fixed
                         << std::setprecision(4) << orderResponse.getLastPrice()
                         << SOH;
        bodyStringStream << static_cast<int>(Tag::LastQty) << "="
                         << orderResponse.getLastQuantity() << SOH;
        bodyStringStream << static_cast<int>(Tag::TransactTime) << "="
                         << timestampToString(
                                    orderResponse.getTransactionTime())
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
                           << orderResponse.getSenderCompId() << SOH;
        headerStringStream << static_cast<int>(Tag::TargetCompID) << "="
                           << orderResponse.getTargetCompId() << SOH;
        headerStringStream << static_cast<int>(Tag::MsgSeqNum) << "="
                           << orderResponse.getMessageSequenceNumber() << SOH;
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

        std::string finalString = finalStringStream.str();
        return {finalString.begin(), finalString.end()};
    }
} // namespace fix
