#include "common/Instrument.h"
#include "common/Types.h"
#include "common/Order.h"
#include "common/Trade.h"
#include "logging/Logger.h"
#include "logging/Runbook.h"
#include "common_fix/ExecutionReport.h"
#include <gtest/gtest.h>
#include <chrono>

using namespace common;

TEST(CommonTests, InstrumentSerialization) {
    EXPECT_EQ(to_string(Instrument::EURUSD), "EURUSD");
    EXPECT_EQ(to_string(Instrument::USDJPY), "USDJPY");
    EXPECT_EQ(from_string("EURUSD"), Instrument::EURUSD);
    EXPECT_EQ(from_string("USDMXN"), Instrument::USDMXN);
    EXPECT_THROW(from_string("INVALID"), std::invalid_argument);
}

TEST(CommonTests, OrderSideSerialization) {
    EXPECT_EQ(to_string(OrderSide::Buy), "Buy");
    EXPECT_EQ(to_string(OrderSide::Sell), "Sell");
    EXPECT_EQ(from_string_OrderSide("Buy"), OrderSide::Buy);
    EXPECT_EQ(from_string_OrderSide("Sell"), OrderSide::Sell);
    EXPECT_THROW(from_string_OrderSide("INVALID"), std::invalid_argument);
}

TEST(CommonTests, OrderTypeSerialization) {
    EXPECT_EQ(to_string(OrderType::Limit), "Limit");
    EXPECT_EQ(to_string(OrderType::Market), "Market");
    EXPECT_EQ(from_string_OrderType("Limit"), OrderType::Limit);
    EXPECT_EQ(from_string_OrderType("Market"), OrderType::Market);
    EXPECT_THROW(from_string_OrderType("INVALID"), std::invalid_argument);
}

TEST(CommonTests, OrderStatusSerialization) {
    EXPECT_EQ(to_string(OrderStatus::New), "New");
    EXPECT_EQ(to_string(OrderStatus::Filled), "Filled");
    EXPECT_EQ(from_string_OrderStatus("New"), OrderStatus::New);
    EXPECT_EQ(from_string_OrderStatus("Filled"), OrderStatus::Filled);
    EXPECT_THROW(from_string_OrderStatus("INVALID"), std::invalid_argument);
}

TEST(CommonTests, TimeInForceSerialization) {
    EXPECT_EQ(to_string(TimeInForce::DAY), "DAY");
    EXPECT_EQ(to_string(TimeInForce::GTC), "GTC");
    EXPECT_EQ(from_string_TimeInForce("DAY"), TimeInForce::DAY);
    EXPECT_EQ(from_string_TimeInForce("GTC"), TimeInForce::GTC);
    EXPECT_THROW(from_string_TimeInForce("INVALID"), std::invalid_argument);
}

TEST(CommonTests, OrderObject) {
    Timestamp now = std::chrono::system_clock::now();
    Order order(1, 100, Instrument::EURUSD, "client1", "bob", OrderSide::Buy, 
                OrderType::Limit, TimeInForce::DAY, 1000, 1.2345, now);
                
    EXPECT_EQ(order.getClientOrderId(), 1);
    EXPECT_EQ(order.getId(), 100);
    EXPECT_EQ(order.getSymbol(), Instrument::EURUSD);
    EXPECT_EQ(order.getClientId(), "client1");
    EXPECT_EQ(order.getSenderCompID(), "bob");
    EXPECT_EQ(order.getSide(), OrderSide::Buy);
    EXPECT_EQ(order.getOrderType(), OrderType::Limit);
    EXPECT_EQ(order.getTimeInForce(), TimeInForce::DAY);
    EXPECT_EQ(order.getOriginalQuantity(), 1000);
    EXPECT_EQ(order.getRemainingQuantity(), 1000);
    EXPECT_EQ(order.getPrice(), 1.2345);
    EXPECT_EQ(order.getTimestamp(), now);
    EXPECT_EQ(order.getStatus(), OrderStatus::New);
    
    order.setStatus(OrderStatus::PartiallyFilled);
    order.setRemainingQuantity(500);
    EXPECT_EQ(order.getStatus(), OrderStatus::PartiallyFilled);
    EXPECT_EQ(order.getRemainingQuantity(), 500);
    
    order.setPrice(1.2346);
    EXPECT_EQ(order.getPrice(), 1.2346);
    
    order.setClientOrderId(2);
    EXPECT_EQ(order.getClientOrderId(), 2);
    
    order.setCoreOrderId(200);
    EXPECT_EQ(order.getId(), 200);
    
    order.setOriginalQuantity(2000);
    EXPECT_EQ(order.getOriginalQuantity(), 2000);
    EXPECT_EQ(order.getRemainingQuantity(), 2000);
    
    Timestamp later = now + std::chrono::seconds(1);
    order.setTimestamp(later);
    EXPECT_EQ(order.getTimestamp(), later);
}

TEST(CommonTests, TradeObject) {
    Timestamp now = std::chrono::system_clock::now();
    Trade trade(10, Instrument::GBPUSD, 100, 200, 500, 1.5, now);
    
    EXPECT_EQ(trade.getTradeId(), 10);
    EXPECT_EQ(trade.getOrderSymbol(), Instrument::GBPUSD);
    EXPECT_EQ(trade.getBuyOrderId(), 100);
    EXPECT_EQ(trade.getSellOrderId(), 200);
    EXPECT_EQ(trade.getQuantity(), 500);
    EXPECT_EQ(trade.getPrice(), 1.5);
    EXPECT_EQ(trade.getTimestamp(), now);
}

TEST(CommonTests, ExecutionReportObject) {
    Timestamp now = std::chrono::system_clock::now();
    fix::ExecutionReport report(49, 56, 1, 37, 11, "exec1", OrderStatus::New, 
                                "text", Instrument::EURUSD, OrderSide::Buy, 
                                100, 0, 100, 0.0, 0, now);
                                
    EXPECT_EQ(report.getSenderCompId(), 49);
    EXPECT_EQ(report.getTargetCompId(), 56);
    EXPECT_EQ(report.getMessageSequenceNumber(), 1);
    EXPECT_EQ(report.getExchangeOrderId(), 37);
    EXPECT_EQ(report.getClientOrderId(), 11);
    EXPECT_EQ(report.getExecutionId(), "exec1");
    EXPECT_EQ(report.getStatus(), OrderStatus::New);
    EXPECT_EQ(report.getText(), "text");
    EXPECT_EQ(report.getSymbol(), Instrument::EURUSD);
    EXPECT_EQ(report.getSide(), OrderSide::Buy);
    EXPECT_EQ(report.getOrderQuantity(), 100);
    EXPECT_EQ(report.getCumulativeQuantity(), 0);
    EXPECT_EQ(report.getLeavesQuantity(), 100);
    EXPECT_EQ(report.getLastPrice(), 0.0);
    EXPECT_EQ(report.getLastQuantity(), 0);
    EXPECT_EQ(report.getTransactionTime(), now);
}

TEST(CommonTests, LoggerAndRunbook) {
    // Test logger init and shutdown
    logging::Logger::Init("test_logger", "logs/test.log", true, true, spdlog::level::info);
    LOG_INFO("Test info message");
    logging::Logger::Shutdown();
    
    // Runbook formatting
    runbook::ErrorDefinition err("E001", "Something broke", "Fix it");
    std::string formatted = runbook::FormatRunbookLog(err, "Context: {}", "some context");
    EXPECT_NE(formatted.find("E001"), std::string::npos);
    EXPECT_NE(formatted.find("Something broke"), std::string::npos);
    EXPECT_NE(formatted.find("Fix it"), std::string::npos);
    EXPECT_NE(formatted.find("some context"), std::string::npos);
}
