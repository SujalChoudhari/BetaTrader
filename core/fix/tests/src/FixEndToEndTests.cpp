#include "fix/FixServer.h"
#include "fix/FixSession.h"
#include "fix/OutboundMessageBuilder.h"
#include "trading_core/TradingCore.h"
#include "trading_core/OrderIDGenerator.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include <asio.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <future>
#include <chrono>

using namespace fix;

namespace {
class MockTradingCore : public trading_core::TradingCore {
public:
    MockTradingCore() : trading_core::TradingCore() {
        // We don't want partitions for this end-to-end test to keep it simple
    }

    void submitCommand(std::unique_ptr<trading_core::Command> command) const override {
        std::lock_guard<std::mutex> lock(mMutex);
        mCommands.push_back(std::move(command));
        mPromise.set_value(true);
    }

    void subscribeToExecutions(trading_core::TradingCore::ExecutionReportCallback callback) override {
        mExecCallback = callback;
    }

    void subscribeToMarketData(common::Symbol symbol, common::SessionID sessionId) override {
        mSubscribedSymbol = symbol;
        mSubscribedSession = sessionId;
    }
    
    void unsubscribeFromMarketData(common::Symbol symbol, common::SessionID sessionId) override {}
    void unsubscribeFromMarketData(common::SessionID sessionId) override {}

    mutable std::vector<std::unique_ptr<trading_core::Command>> mCommands;
    mutable std::promise<bool> mPromise;
    trading_core::TradingCore::ExecutionReportCallback mExecCallback;
    mutable std::mutex mMutex;
    
    // For Market Data Verification
    common::Instrument mSubscribedSymbol = common::Instrument::COUNT;
    common::SessionID mSubscribedSession = 0;
};
} // namespace

// Helper: read from socket with a timeout so tests never hang.
// Returns the data read, or empty string on timeout/error.
std::string readWithTimeout(asio::ip::tcp::socket& socket, std::chrono::milliseconds timeout = std::chrono::milliseconds(2000)) {
    char data[4096];
    std::error_code readEc;
    size_t bytesRead = 0;
    bool completed = false;

    socket.async_read_some(asio::buffer(data), [&](const std::error_code& ec, size_t len) {
        readEc = ec;
        bytesRead = len;
        completed = true;
    });

    // Run io_context for up to 'timeout' duration to process the async read
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!completed && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    if (!completed) {
        socket.cancel();  // Cancel the pending read
        // Give time for cancellation callback
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return "";
    }

    if (readEc) return "";
    return std::string(data, bytesRead);
}

TEST(FixEndToEndTests, FullLogonAndOrderFlow) {
    asio::io_context io;
    auto mockCore = std::make_shared<MockTradingCore>();
    
    // Start server on a specific port
    short port = 18083;
    FixServer server(io, port, *mockCore);
    server.getManager().loadConfig({"CLIENT_A"});

    // Run io_context in a separate thread
    std::jthread ioThread([&io]() { io.run(); });

    // Give io_context a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Client side: Connect to server
    asio::ip::tcp::socket clientSocket(io);
    clientSocket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));

    // Ensure config is loaded. 
    server.getManager().loadConfig({"CLIENT_A"});

    // 1. Send Logon
    std::string logonMsg = OutboundMessageBuilder::buildLogon("CLIENT_A", "BETA_EXCHANGE", 1, 30);
    asio::write(clientSocket, asio::buffer(logonMsg));

    // 2. Read Logon Ack (with timeout)
    std::string response = readWithTimeout(clientSocket);
    ASSERT_FALSE(response.empty()) << "Timed out waiting for Logon Ack";
    EXPECT_NE(response.find("35=A"), std::string::npos); // Logon Ack

    // 3. Send NewOrderSingle
    std::string orderBody = "11=12345\x01" "55=EURUSD\x01" "54=1\x01" "38=100\x01" "44=1.23\x01" "40=2\x01";
    std::string orderMsg = OutboundMessageBuilder::buildMessage("CLIENT_A", "BETA_EXCHANGE", 2, "D", orderBody);
    
    mockCore->mPromise = std::promise<bool>();
    auto futureCmd = mockCore->mPromise.get_future();
    asio::write(clientSocket, asio::buffer(orderMsg));
    ASSERT_EQ(futureCmd.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    {
        std::lock_guard<std::mutex> lock(mockCore->mMutex);
        ASSERT_EQ(mockCore->mCommands.size(), 1);
        EXPECT_EQ(mockCore->mCommands[0]->getType(), trading_core::CommandType::NewOrder);
    }

    // 4. Send OrderCancelRequest
    std::string cancelBody = "11=67890\x01" "41=12345\x01" "37=999\x01" "55=EURUSD\x01" "54=1\x01" "60=20260313-10:00:00.000\x01";
    std::string cancelMsg = OutboundMessageBuilder::buildMessage("CLIENT_A", "BETA_EXCHANGE", 3, "F", cancelBody);
    mockCore->mPromise = std::promise<bool>();
    futureCmd = mockCore->mPromise.get_future();
    asio::write(clientSocket, asio::buffer(cancelMsg));
    ASSERT_EQ(futureCmd.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    {
        std::lock_guard<std::mutex> lock(mockCore->mMutex);
        ASSERT_EQ(mockCore->mCommands.size(), 2);
        EXPECT_EQ(mockCore->mCommands[1]->getType(), trading_core::CommandType::CancelOrder);
    }

    // 5. Send OrderCancelReplaceRequest (Modify)
    std::string modifyBody = "11=77777\x01" "41=67890\x01" "37=999\x01" "55=EURUSD\x01" "54=1\x01" "38=200\x01" "44=1.24\x01" "40=2\x01" "60=20260313-10:00:00.000\x01";
    std::string modifyMsg = OutboundMessageBuilder::buildMessage("CLIENT_A", "BETA_EXCHANGE", 4, "G", modifyBody);
    mockCore->mPromise = std::promise<bool>();
    futureCmd = mockCore->mPromise.get_future();
    asio::write(clientSocket, asio::buffer(modifyMsg));
    ASSERT_EQ(futureCmd.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    {
        std::lock_guard<std::mutex> lock(mockCore->mMutex);
        ASSERT_EQ(mockCore->mCommands.size(), 3);
        EXPECT_EQ(mockCore->mCommands[2]->getType(), trading_core::CommandType::ModifyOrder);
    }

    // 6. Send MarketDataRequest
    std::string mdBody = "262=REQ123\x01" "263=1\x01" "146=1\x01" "55=EURUSD\x01" "267=1\x01" "269=0\x01";
    std::string mdMsg = OutboundMessageBuilder::buildMessage("CLIENT_A", "BETA_EXCHANGE", 5, "V", mdBody);
    asio::write(clientSocket, asio::buffer(mdMsg));
    // Wait for async processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 7. Simulate ExecutionReport (Ack)
    fix::ExecutionReport report(
        static_cast<fix::CompID>(2), static_cast<fix::CompID>(1), 1, 999, 12345, "EXEC1", common::OrderStatus::New, "Ack",
        common::Instrument::EURUSD, common::OrderSide::Buy, 100, 0, 100, 0, 0, std::chrono::system_clock::now()
    );
    server.onExecutionReport(report);
    std::string execReportMsg = readWithTimeout(clientSocket);
    ASSERT_FALSE(execReportMsg.empty()) << "Timed out waiting for ExecutionReport";
    EXPECT_NE(execReportMsg.find("35=8"), std::string::npos);

    // 8. Simulate Market Data
    fix::MarketDataSnapshotFullRefresh snapshot;
    snapshot.symbol = common::Instrument::EURUSD;
    snapshot.targetSessionID = 1;
    snapshot.mdReqID = "REQ123";
    
    fix::MarketDataEntry bidEntry;
    bidEntry.entryType = fix::MDEntryType::Bid;
    bidEntry.price = 1.2345;
    bidEntry.size = 1000000;
    snapshot.entries.push_back(bidEntry);
    
    fix::MarketDataEntry askEntry;
    askEntry.entryType = fix::MDEntryType::Offer;
    askEntry.price = 1.2346;
    askEntry.size = 1000000;
    snapshot.entries.push_back(askEntry);

    server.onMarketDataSnapshotFullRefresh(snapshot);
    std::string snapshotMsg = readWithTimeout(clientSocket);
    ASSERT_FALSE(snapshotMsg.empty()) << "Timed out waiting for MarketDataSnapshot";
    EXPECT_NE(snapshotMsg.find("35=W"), std::string::npos);
    EXPECT_NE(snapshotMsg.find("REQ123"), std::string::npos);

    fix::MarketDataIncrementalRefresh refresh;
    refresh.symbol = common::Instrument::EURUSD;
    refresh.targetSessionID = 1;
    refresh.mdReqID = "REQ123";
    
    fix::MarketDataIncrementalEntry incEntry;
    incEntry.updateAction = fix::MDUpdateAction::New;
    incEntry.entryType = fix::MDEntryType::Bid;
    incEntry.price = 1.2347;
    incEntry.size = 500000;
    refresh.entries.push_back(incEntry);

    server.onMarketDataIncrementalRefresh(refresh);
    std::string refreshMsg = readWithTimeout(clientSocket);
    ASSERT_FALSE(refreshMsg.empty()) << "Timed out waiting for MarketDataIncRefresh";
    EXPECT_NE(refreshMsg.find("35=X"), std::string::npos);

    // 9. Session Messages
    std::string testReq = OutboundMessageBuilder::buildMessage("CLIENT_A", "BETA_EXCHANGE", 6, "1", "112=TEST1\x01");
    asio::write(clientSocket, asio::buffer(testReq));
    std::string heartbeat = OutboundMessageBuilder::buildMessage("CLIENT_A", "BETA_EXCHANGE", 7, "0", "");
    asio::write(clientSocket, asio::buffer(heartbeat));

    // 10. Logout
    std::string logoutMsg = OutboundMessageBuilder::buildLogout("CLIENT_A", "BETA_EXCHANGE", 8);
    asio::write(clientSocket, asio::buffer(logoutMsg));
    std::string logoutAck = readWithTimeout(clientSocket);
    ASSERT_FALSE(logoutAck.empty()) << "Timed out waiting for Logout Ack";
    EXPECT_NE(logoutAck.find("35=5"), std::string::npos);

    io.stop();
}

TEST(FixEndToEndTests, NegativeScenarios) {
    asio::io_context io;
    auto mockCore = std::make_shared<MockTradingCore>();
    short port = 18084;
    FixServer server(io, port, *mockCore);
    server.getManager().loadConfig({"CLIENT_A"});

    std::jthread ioThread([&io]() { io.run(); });

    // Scenario 1: Message before Logon
    {
        asio::ip::tcp::socket socket(io);
        socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));
        std::string orderBody = "11=12345\x01" "55=EURUSD\x01" "54=1\x01" "38=100\x01" "44=1.23\x01" "40=2\x01";
        std::string orderMsg = OutboundMessageBuilder::buildMessage("CLIENT_A", "BETA_EXCHANGE", 1, "D", orderBody);
        asio::write(socket, asio::buffer(orderMsg));
        
        char data[1024];
        std::error_code ec;
        socket.read_some(asio::buffer(data), ec);
        EXPECT_TRUE(ec == asio::error::eof || ec == asio::error::connection_reset);
    }

    // Scenario 2: Invalid Logon
    {
        asio::ip::tcp::socket socket(io);
        socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));
        std::string logonMsg = OutboundMessageBuilder::buildLogon("STRANGER", "BETA_EXCHANGE", 1, 30);
        asio::write(socket, asio::buffer(logonMsg));
        
        char data[1024];
        std::error_code ec;
        size_t len = socket.read_some(asio::buffer(data), ec);
        if (!ec) {
            std::string response(data, len);
            EXPECT_NE(response.find("35=5"), std::string::npos);
        }
    }

    // Scenario 3: Malformed Message
    {
        asio::ip::tcp::socket socket(io);
        socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));
        std::string malformed = "NOT_A_FIX_MESSAGE\x01" "10=000\x01";
        asio::write(socket, asio::buffer(malformed));
    }

    // Scenario 4: Partial Message
    // Re-load config because the AuthRepository async callback may have overwritten
    // our initial loadConfig, removing CLIENT_A from the valid clients list.
    server.getManager().loadConfig({"CLIENT_A"});
    {
        asio::ip::tcp::socket socket(io);
        socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));
        std::string partial = "8=FIX.4.4\x01" "9=50\x01" "35=A\x01";
        asio::write(socket, asio::buffer(partial));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::string rest = "49=CLIENT_A\x01" "56=BETA_EXCHANGE\x01" "34=10\x01" "52=20231027-10:00:00\x01" "98=0\x01" "108=30\x01" "10=123\x01";
        asio::write(socket, asio::buffer(rest));
        
        char data[1024];
        size_t len = socket.read_some(asio::buffer(data));
        std::string response(data, len);
        EXPECT_NE(response.find("35=A"), std::string::npos);
    }

    io.stop();
}

TEST(FixEndToEndTests, OutboundBuilderCoverage) {
    auto heart = OutboundMessageBuilder::buildHeartbeat("S", "T", 1, "REQ1");
    EXPECT_NE(heart.find("35=0"), std::string::npos);
    EXPECT_NE(heart.find("112=REQ1"), std::string::npos);
    
    auto resend = OutboundMessageBuilder::buildResendRequest("S", "T", 1, 100, 200);
    EXPECT_NE(resend.find("35=2"), std::string::npos);
    EXPECT_NE(resend.find("7=100"), std::string::npos);
    EXPECT_NE(resend.find("16=200"), std::string::npos);

    auto logoutNoText = OutboundMessageBuilder::buildLogout("S", "T", 1, "");
    EXPECT_EQ(logoutNoText.find("58="), std::string::npos);
}
