#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "fix_client/FixClientSession.h"
#include "fix_client/AuthManager.h"
#include "fix_client/SeqNumStore.h"
#include "fix_client/FixMessageParser.h"
#include "fix/FixServer.h"
#include "fix/FixSessionManager.h"
#include <exchange_app/TradingCore.h>
#include "common_fix/OrderRequest.h"
#include "common_fix/ExecutionReport.h"
#include "logging/Logger.h"

#include <asio.hpp>
#include <thread>
#include <chrono>
#include <future>
#include <filesystem>

class MockTradingCore : public trading_core::TradingCore {
public:
    MockTradingCore() : trading_core::TradingCore(nullptr, false) {}

    MOCK_METHOD(void, submitCommand, (std::unique_ptr<trading_core::Command> command), (const, override));
    MOCK_METHOD(data::AuthRepository*, getAuthRepository, (), (const));
    MOCK_METHOD(void, subscribeToMarketData, (common::Symbol symbol, common::SessionID sessionID), (override));
    MOCK_METHOD(void, unsubscribeFromMarketData, (common::Symbol symbol, common::SessionID sessionID), (override));
};

// Concrete mock generator to avoid segfaults in real server parsing
// Removed unused RealOrderIDGenerator class

class ClientFixEndToEndTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Suppress logs during tests unless debugging
        logging::Logger::Init("test_logger", "logs/test.log", true, false);
        std::filesystem::remove_all("seq_store");

        // Setup Server
        mockCore = std::make_unique<MockTradingCore>();
        EXPECT_CALL(*mockCore, getAuthRepository()).WillRepeatedly(::testing::Return(nullptr));

        serverIoContext = std::make_unique<asio::io_context>();
        server = std::make_unique<fix::FixServer>(*serverIoContext, 9091, *mockCore, nullptr);
        server->getManager().loadConfig({"CLIENT_1"});

        serverThread = std::thread([this]() {
            server->run();
        });

        // Setup Client
        clientIoContext = std::make_unique<asio::io_context>();
        clientSession = std::make_shared<fix_client::FixClientSession>(*clientIoContext, "CLIENT_1", "BETA_EXCHANGE");
        authManager = std::make_unique<fix_client::AuthManager>(clientSession);

        // Run client IO in background
        clientGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(asio::make_work_guard(*clientIoContext));
        clientThread = std::thread([this]() {
            clientIoContext->run();
        });
    }

    void TearDown() override {
        clientSession->disconnect();
        clientGuard.reset();
        server->stop();
        
        if (clientThread.joinable()) clientThread.join();
        if (serverThread.joinable()) serverThread.join();
    }

    std::unique_ptr<MockTradingCore> mockCore;
    
    std::unique_ptr<asio::io_context> serverIoContext;
    std::unique_ptr<fix::FixServer> server;
    std::thread serverThread;

    std::unique_ptr<asio::io_context> clientIoContext;
    std::shared_ptr<fix_client::FixClientSession> clientSession;
    std::unique_ptr<fix_client::AuthManager> authManager;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> clientGuard;
    
    std::thread clientThread;
};

TEST_F(ClientFixEndToEndTests, ConnectLogonAndAuthenticate) {
    std::promise<fix_client::FixClientState> activePromise;
    auto activeFuture = activePromise.get_future();
    
    clientSession->setStateChangeCallback([&activePromise](fix_client::FixClientState state) {
        if (state == fix_client::FixClientState::Active) {
            activePromise.set_value(state);
        }
    });

    clientSession->connect("127.0.0.1", 9091);
    
    // Wait for TCP connect
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(clientSession->getState(), fix_client::FixClientState::Connected);

    // Send Logon
    clientSession->sendLogon(30, true);

    // Wait for Active state (Logon ACK received)
    auto status = activeFuture.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(status, std::future_status::ready) << "Timeout waiting for Active state";
    
    EXPECT_EQ(clientSession->getState(), fix_client::FixClientState::Active);
}

TEST_F(ClientFixEndToEndTests, SequencePersistenceAcrossRestarts) {
    // 1. Initial Logon to establish sequences
    clientSession->connect("127.0.0.1", 9091);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    clientSession->sendLogon(30, true); // Force reset to start at 1
    
    // Wait for active
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(clientSession->getState(), fix_client::FixClientState::Active);
    
    // 2. Disconnect and shutdown
    clientSession->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 3. Create a NEW session pointing to same store
    auto newClientSession = std::make_shared<fix_client::FixClientSession>(*clientIoContext, "CLIENT_1", "BETA_EXCHANGE");
    
    // Verify it loaded sequences (NextSender should be 2 now because Logon was sent)
    // Wait, let's check SeqNumStore directly or through the session
    // Since we don't have a getter for seq nums in session, we'll check it by sending another logon WITHOUT force reset.
    
    newClientSession->connect("127.0.0.1", 9091);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 4. Send Logon WITHOUT force reset
    newClientSession->sendLogon(30, false); 
    
    // If it works, it means it sent 34=2 and server accepted it.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(newClientSession->getState(), fix_client::FixClientState::Active);
}

TEST_F(ClientFixEndToEndTests, SequenceSyncOnLogon) {
    // 1. Setup a client that already has high sequences
    {
        fix_client::SeqNumStore store("CLIENT_1", "seq_store");
        store.setSeqNums(50, 50);
    }
    
    // 2. Start server (which starts at 1,1 for CLIENT_1)
    // server already started in SetUp
    
    // 3. Connect and send Logon with sequence 50
    clientSession->connect("127.0.0.1", 9091);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    clientSession->sendLogon(30, false); // No force reset
    
    // 4. Server should sync and accept it!
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(clientSession->getState(), fix_client::FixClientState::Active);
}

TEST_F(ClientFixEndToEndTests, InboundSequenceSyncDownOnLogon) {
    // 1. Setup a client that expects a HIGH sequence from server
    {
        fix_client::SeqNumStore store("CLIENT_1", "seq_store");
        store.setSeqNums(100, 1); // Expects 100 from server
    }
    
    // 2. Start server (which starts at 1,1)
    // server already started in SetUp
    
    // 3. Connect and send Logon
    clientSession->connect("127.0.0.1", 9091);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    clientSession->sendLogon(30, false);
    
    // 4. Client should see server's '1', sync down to 1, and become Active!
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(clientSession->getState(), fix_client::FixClientState::Active);
}


