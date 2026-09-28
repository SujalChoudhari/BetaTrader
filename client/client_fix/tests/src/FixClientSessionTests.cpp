#include <gtest/gtest.h>
#define private public  // Alternative to friend if header edit is risky, but we used friend
#include "fix_client/FixClientSession.h"
#include "client_ui/ClientEventLog.h"
#undef private
#include <asio.hpp>
#include <array>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

// Since we added friend class FixClientSessionTests, we can access internals.
namespace fix_client {

namespace {

std::string withChecksum(std::string message) {
    unsigned int checksum = 0;
    for (char value : message) checksum += static_cast<unsigned int>(value);

    char checksumText[4];
    std::snprintf(checksumText, sizeof(checksumText), "%03u", checksum % 256);
    return message + "10=" + checksumText + "\x01";
}

std::string executionReport(uint32_t sequence, char status) {
    return withChecksum(
        "8=FIX.4.4\x01" "35=8\x01" "34=" + std::to_string(sequence) + "\x01"
        "37=100\x01" "11=200\x01"
        "17=EXEC1\x01" + std::string("39=") + status + "\x01"
        "55=EURUSD\x01" "54=1\x01" "38=1000\x01" "14=500\x01"
        "151=500\x01" "31=1.2345\x01" "32=500\x01"
        "60=20251030-10:00:00.000\x01");
}

} // namespace

class FixClientSessionTests : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove_all("session_test_store");
        ioContext = std::make_unique<asio::io_context>();
        session = std::make_shared<FixClientSession>(*ioContext, "TEST_CLIENT", "TEST_SERVER", "session_test_store");
    }

    void TearDown() override {
        std::filesystem::remove_all("session_test_store");
    }

    std::unique_ptr<asio::io_context> ioContext;
    std::shared_ptr<FixClientSession> session;
};

TEST_F(FixClientSessionTests, InitialState) {
    EXPECT_EQ(session->getState(), FixClientState::Disconnected);
}

TEST_F(FixClientSessionTests, StateTransitionsManual) {
    // Testing the changeState helper
    session->changeState(FixClientState::Connected);
    EXPECT_EQ(session->getState(), FixClientState::Connected);
    
    session->changeState(FixClientState::Active);
    EXPECT_EQ(session->getState(), FixClientState::Active);
}

TEST_F(FixClientSessionTests, HandleLogonResponse) {
    session->changeState(FixClientState::LogonSent);
    
    // Simulate a Logon message (35=A) from server
    // 8=FIX.4.4|9=70|35=A|49=TEST_SERVER|56=TEST_CLIENT|34=1|52=20251030-10:00:00|98=0|108=30|10=...
    std::string logonMsg = "8=FIX.4.4\x01" "9=70\x01" "35=A\x01" "49=TEST_SERVER\x01" "56=TEST_CLIENT\x01" "34=1\x01" "10=000\x01";
    
    session->handleProtocolMessage(logonMsg);
    
    // Should transition to Active
    EXPECT_EQ(session->getState(), FixClientState::Active);
}

TEST_F(FixClientSessionTests, HandleLogoutResponse) {
    session->changeState(FixClientState::Active);
    
    std::string logoutMsg = "8=FIX.4.4\x01" "35=5\x01" "49=TEST_SERVER\x01" "56=TEST_CLIENT\x01" "34=2\x01" "10=000\x01";
    
    session->handleProtocolMessage(logoutMsg);
    
    // Should transition to Disconnected/Disconnected after processing logout
    EXPECT_EQ(session->getState(), FixClientState::Disconnected);
}

TEST_F(FixClientSessionTests, HandleHeartbeat) {
    session->changeState(FixClientState::Active);
    uint32_t initialSeq = session->mSeqStore.getNextTargetSeqNum(); // Should be 1
    
    std::string hbMsg = "8=FIX.4.4\x01" "35=0\x01" "49=TEST_SERVER\x01" "56=TEST_CLIENT\x01" "34=1\x01" "10=000\x01";
    session->handleProtocolMessage(hbMsg);
    
    // Sequence should have increased
    EXPECT_EQ(session->mSeqStore.getNextTargetSeqNum(), initialSeq + 1);
}

TEST_F(FixClientSessionTests, HandleTestRequest) {
    session->changeState(FixClientState::Active);
    
    // TestRequest (35=1) with Tag 112 (TestReqID)
    std::string trMsg = "8=FIX.4.4\x01" "35=1\x01" "112=TEST_123\x01" "34=1\x01" "10=000\x01";
    
    // This should trigger sending a Heartbeat with 112=TEST_123.
    // Since we don't have a real socket, it might log an error but we can verify the call happened if we had a mock.
    // For now, let's just ensure it doesn't crash and increases sequence.
    session->handleProtocolMessage(trMsg);
    EXPECT_EQ(session->mSeqStore.getNextTargetSeqNum(), 2);
}

TEST_F(FixClientSessionTests, SequenceGapHandling) {
    session->changeState(FixClientState::Active);
    session->mSeqStore.setSeqNums(1, 1);
    
    // Incoming message with SeqNum=10 (Gap! Expected 1)
    std::string gapMsg = "8=FIX.4.4\x01" "35=0\x01" "34=10\x01" "10=000\x01";
    
    session->handleProtocolMessage(gapMsg);
    
    // In our simplified client, we "sync up" to the gap to avoid death loops,
    // so it should move to 11.
    EXPECT_EQ(session->mSeqStore.getNextTargetSeqNum(), 11);
}

TEST_F(FixClientSessionTests, ResendRequestProcessing) {
    session->changeState(FixClientState::Active);
    session->mSeqStore.setSeqNums(1, 1);
    
    // Server requests resend (35=2) from 1 to 0 (infinity)
    std::string rrMsg = "8=FIX.4.4\x01" "35=2\x01" "7=1\x01" "16=0\x01" "34=1\x01" "10=000\x01";
    
    session->handleProtocolMessage(rrMsg);
    
    // Should increase target seq num to 2
    EXPECT_EQ(session->mSeqStore.getNextTargetSeqNum(), 2);
}

TEST_F(FixClientSessionTests, SequenceResetHandle) {
    session->changeState(FixClientState::Active);
    
    // SequenceReset (35=4) with NewSeqNo (36)
    std::string srMsg = "8=FIX.4.4\x01" "35=4\x01" "36=100\x01" "34=1\x01" "10=000\x01";
    
    session->handleProtocolMessage(srMsg);
    
    // InSeq should jump to 100
    EXPECT_EQ(session->mSeqStore.getNextTargetSeqNum(), 100);
}

TEST_F(FixClientSessionTests, EmitsOutboundOrderIntentForBlotterTracking) {
    std::vector<NewOrderIntent> intents;
    session->setOrderIntentCallback([&intents](const NewOrderIntent& intent) {
        intents.push_back(intent);
    });
    session->changeState(FixClientState::Active);

    session->sendNewOrder("EURUSD", '1', 1.2345, 100, '2', '0');

    ASSERT_EQ(intents.size(), 1U);
    EXPECT_FALSE(intents.front().clientOrderId.empty());
    EXPECT_EQ(intents.front().symbol, "EURUSD");
    EXPECT_EQ(intents.front().side, '1');
    EXPECT_EQ(intents.front().orderType, '2');
    EXPECT_EQ(intents.front().quantity, 100);
    EXPECT_EQ(intents.front().price, 1.2345);
}

TEST_F(FixClientSessionTests, EmitsStructuredSessionRejectExecutionAndMarketDataEvents) {
    std::vector<FixClientEvent> events;
    std::size_t parsedExecutionReports = 0;
    session->setEventCallback([&events](const FixClientEvent& event) {
        events.push_back(event);
    });
    session->setMessageCallback([&parsedExecutionReports](const ParsedFixMessage& message) {
        if (std::holds_alternative<fix::ExecutionReport>(message)) {
            ++parsedExecutionReports;
        }
    });

    session->changeState(FixClientState::LogonSent);
    session->handleProtocolMessage(
        "8=FIX.4.4\x01" "35=A\x01" "34=1\x01" "49=TEST_SERVER\x01"
        "56=TEST_CLIENT\x01" "10=000\x01");
    session->handleProtocolMessage(
        "8=FIX.4.4\x01" "35=3\x01" "34=2\x01" "58=invalid order\x01"
        "10=000\x01");
    session->handleProtocolMessage(executionReport(3, '0'));
    session->handleProtocolMessage(executionReport(4, '1'));
    session->handleProtocolMessage(executionReport(5, '2'));
    session->handleProtocolMessage(executionReport(6, '4'));
    session->handleProtocolMessage(executionReport(7, '8'));
    session->handleProtocolMessage(withChecksum(
        "8=FIX.4.4\x01" "35=W\x01" "34=8\x01" "262=REQ1\x01" "55=EURUSD\x01"
        "268=1\x01" "269=0\x01" "270=1.2345\x01" "271=100\x01" "290=1\x01"));

    ASSERT_EQ(events.size(), 8U);
    EXPECT_EQ(events[0].type, FixClientEventType::Session);
    EXPECT_EQ(events[1].type, FixClientEventType::Reject);
    const std::array<std::string_view, 5> statuses = {
        "New", "PartiallyFilled", "Filled", "Cancelled", "Rejected"};
    for (std::size_t i = 0; i < statuses.size(); ++i) {
        EXPECT_EQ(events[2 + i].type, FixClientEventType::ExecutionReport);
        EXPECT_NE(events[2 + i].message.find(statuses[i]), std::string::npos);
        EXPECT_NE(events[2 + i].message.find("EURUSD"), std::string::npos);
    }
    EXPECT_EQ(events[7].type, FixClientEventType::MarketData);
    EXPECT_NE(events[7].message.find("EURUSD"), std::string::npos);
    EXPECT_EQ(parsedExecutionReports, 5U);
    for (const auto& event : events) {
        EXPECT_EQ(event.message.find("password"), std::string::npos);
        EXPECT_EQ(event.message.find("35=", 0), std::string::npos);
    }
}

} // namespace fix_client

TEST(ClientEventLogTests, RetainsBoundedThreadSafeSnapshots) {
    client_ui::ClientEventLog eventLog(3);
    std::thread firstProducer([&eventLog]() {
        for (int i = 0; i < 200; ++i) {
            eventLog.append({fix_client::FixClientEventType::Session, "first"});
        }
    });
    std::thread secondProducer([&eventLog]() {
        for (int i = 0; i < 200; ++i) {
            eventLog.append({fix_client::FixClientEventType::TransportError, "second"});
        }
    });
    firstProducer.join();
    secondProducer.join();

    const auto snapshot = eventLog.snapshot();
    EXPECT_LE(snapshot.size(), 3U);
    EXPECT_EQ(eventLog.size(), snapshot.size());
}
