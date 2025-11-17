#include "fix/FixSession.h"
#include "fix/BinaryToOrderRequestConverter.h"
#include "fix/ExecutionReportToBinaryConverter.h"
#include "fix/FixRunbookDefinations.h"
#include "logging/Runbook.h"
#include "trading_core/NewOrder.h"
#include <iostream>

namespace fix {

    FixSession::FixSession(asio::ip::tcp::socket socket,
                           trading_core::TradingCore& tradingCore,
                           const uint32_t sessionId)
        : mSocket(std::move(socket)), mTradingCore(tradingCore),
          mData(MaxLength), mSessionId(sessionId)
    {
        LOG_INFO("New FIX session created with ID: {}", mSessionId);
    }

    void FixSession::start()
    {
        doRead();
    }

    void FixSession::sendExecutionReport(const ExecutionReport& report)
    {
        auto binaryReport = ExecutionReportToBinaryConverter::convert(report);
        auto self(shared_from_this());
        asio::async_write(
                mSocket, asio::buffer(binaryReport),
                [this, self, report](const std::error_code ec,
                                     std::size_t /*length*/) {
                    if (!ec) {
                        LOG_INFO("Sent ExecutionReport to Session {}: "
                                 "OrderID={}, Status={}",
                                 mSessionId, report.getExchangeOrderId(),
                                 static_cast<int>(report.getStatus()));
                    }
                    else {
                        LOG_ERROR(errors::EFIX3,
                                  "Failed to send ExecutionReport to Session "
                                  "{}: {}",
                                  mSessionId, ec.message());
                    }
                });
    }

    uint32_t FixSession::getSessionID() const
    {
        return mSessionId;
    }

    void FixSession::doRead()
    {
        auto self(shared_from_this());
        mSocket.async_read_some(asio::buffer(mData), [this,
                                                      self](const std::
                                                                    error_code
                                                                            ec,
                                                            const std::size_t
                                                                    length) {
            if (!ec) {
                try {
                    auto orderRequest = BinaryToOrderRequestConverter::convert(
                            std::vector<char>(mData.begin(),
                                              mData.begin() + length));

                    LOG_INFO("Received NewOrderSingle from Session {}: "
                             "Symbol={}, Qty={}, Price={}",
                             mSessionId, common::to_string(orderRequest.symbol),
                             orderRequest.quantity, orderRequest.price);

                    auto common_order = std::make_unique<common::Order>(
                            0, orderRequest.symbol, std::to_string(mSessionId),
                            orderRequest.side, common::OrderType::Limit,
                            common::TimeInForce::DAY,
                            static_cast<int>(orderRequest.quantity),
                            orderRequest.price,
                            std::chrono::system_clock::now());

                    auto newOrderCmd = std::make_unique<trading_core::NewOrder>(
                            common_order->getClientId(),
                            common_order->getTimestamp(),
                            std::move(common_order));

                    mTradingCore.submitCommand(std::move(newOrderCmd));

                    doRead();
                }
                catch (const std::exception& e) {
                    LOG_ERROR(errors::EFIX2,
                              "Failed to process message from Session {}: {}",
                              mSessionId, e.what());
                    // TODO: Disconnect session here
                }
            }
            else {
                if (ec != asio::error::eof) {
                    LOG_ERROR(errors::EFIX3,
                              "Error reading from socket for Session {}: {}",
                              mSessionId, ec.message());
                }
                // Client disconnected
            }
        });
    }

} // namespace fix
