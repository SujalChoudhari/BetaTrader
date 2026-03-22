#include <gtest/gtest.h>
#include "fix_client/FixMessageParser.h"
#include "logging/Logger.h"

class FixMessageParserTests : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        logging::Logger::Init("parser_test_logger", "logs/parser_test.log", true, false);
    }
};

TEST_F(FixMessageParserTests, ParseExecutionReport) {
    std::string msg = "8=FIX.4.4\x01" "9=142\x01" "35=8\x01" "49=BETA_EXCHANGE\x01" "56=CLIENT_1\x01"
                      "34=2\x01" "52=20251030-10:00:00.000\x01" 
                      "37=100\x01" "11=200\x01" "17=EXEC1\x01" "39=0\x01" "55=EURUSD\x01"
                      "54=1\x01" "38=1000\x01" "14=0\x01" "151=1000\x01" "31=0\x01" "32=0\x01"
                      "60=20251030-10:00:00.000\x01" "10=185\x01";

    auto parsed = fix_client::FixMessageParser::parse(msg);
    EXPECT_TRUE(std::holds_alternative<fix::ExecutionReport>(parsed));
    
    if (auto* report = std::get_if<fix::ExecutionReport>(&parsed)) {
        EXPECT_EQ(report->getExchangeOrderId(), 100);
        EXPECT_EQ(report->getClientOrderId(), 200);
        EXPECT_EQ(report->getStatus(), common::OrderStatus::New);
        EXPECT_EQ(report->getOrderQuantity(), 1000);
        EXPECT_EQ(report->getSymbol(), common::Instrument::EURUSD);
        EXPECT_EQ(report->getSide(), common::OrderSide::Buy);
    }
}

TEST_F(FixMessageParserTests, ParseMarketDataSnapshot) {
    // A synthetic snapshot message: W (MarketDataSnapshotFullRefresh)
    // with 2 repeating group entries (Bid, Ask)
    std::string msg = "8=FIX.4.4\x01" "9=119\x01" "35=W\x01" "49=BETA_EXCHANGE\x01" "56=CLIENT_1\x01"
                      "34=3\x01" "52=20251030-10:00:00.000\x01"
                      "262=REQ1\x01" "55=GBPUSD\x01" "268=2\x01"
                      "269=0\x01" "270=1.2345\x01" "271=100000\x01" "290=1\x01"
                      "269=1\x01" "270=1.2347\x01" "271=200000\x01" "290=1\x01"
                      "10=185\x01";

    auto parsed = fix_client::FixMessageParser::parse(msg);
    EXPECT_TRUE(std::holds_alternative<fix::MarketDataSnapshotFullRefresh>(parsed));

    if (auto* snap = std::get_if<fix::MarketDataSnapshotFullRefresh>(&parsed)) {
        EXPECT_EQ(snap->mdReqID, "REQ1");
        EXPECT_EQ(snap->symbol, common::Instrument::GBPUSD);
        ASSERT_EQ(snap->entries.size(), 2);
        
        EXPECT_EQ(snap->entries[0].entryType, fix::MDEntryType::Bid);
        EXPECT_DOUBLE_EQ(snap->entries[0].price, 1.2345);
        EXPECT_EQ(snap->entries[0].size, 100000);
        
        EXPECT_EQ(snap->entries[1].entryType, fix::MDEntryType::Offer);
        EXPECT_DOUBLE_EQ(snap->entries[1].price, 1.2347);
        EXPECT_EQ(snap->entries[1].size, 200000);
    }
}

TEST_F(FixMessageParserTests, ParseMarketDataIncremental) {
    std::string msg = "8=FIX.4.4\x01" "9=108\x01" "35=X\x01" "49=BETA_EXCHANGE\x01" "56=CLIENT_1\x01"
                      "34=4\x01" "52=20251030-10:00:00.000\x01"
                      "262=REQ1\x01" "268=1\x01"
                      "279=1\x01" "55=USDJPY\x01" "269=0\x01" "270=150.25\x01" "271=50000\x01" "290=2\x01"
                      "10=156\x01";

    auto parsed = fix_client::FixMessageParser::parse(msg);
    EXPECT_TRUE(std::holds_alternative<fix::MarketDataIncrementalRefresh>(parsed));

    if (auto* inc = std::get_if<fix::MarketDataIncrementalRefresh>(&parsed)) {
        EXPECT_EQ(inc->mdReqID, "REQ1");
        ASSERT_EQ(inc->entries.size(), 1);
        EXPECT_EQ(inc->symbol, common::Instrument::USDJPY); // Inherited from first entry if present

        EXPECT_EQ(inc->entries[0].updateAction, fix::MDUpdateAction::Change);
        EXPECT_EQ(inc->entries[0].entryType, fix::MDEntryType::Bid);
        EXPECT_DOUBLE_EQ(inc->entries[0].price, 150.25);
        EXPECT_EQ(inc->entries[0].size, 50000);
        EXPECT_EQ(inc->entries[0].entryPosition, 2);
    }
}

TEST_F(FixMessageParserTests, BadChecksumReturnsMonostate) {
    std::string msg = "8=FIX.4.4\x01" "9=142\x01" "35=8\x01" "49=BETA_EXCHANGE\x01" "56=CLIENT_1\x01"
                      "34=2\x01" "52=20251030-10:00:00.000\x01" 
                      "10=999\x01"; // Guaranteed bad checksum

    auto parsed = fix_client::FixMessageParser::parse(msg);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(parsed));
}
