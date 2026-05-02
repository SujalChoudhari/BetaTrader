#include <gtest/gtest.h>
#include "fix_client/FixClientSession.h"
#include <asio.hpp>
#include <filesystem>

// Since we added friend class FixClientSessionTests, we can access internals.
namespace fix_client {

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

    void setSessionState(FixClientState state) {
        session->mState = state;
    }

    uint32_t getNextTargetSeqNum() const {
        return session->mSeqStore.getNextTargetSeqNum();
    }

    uint32_t getNextSenderSeqNum() const {
        return session->mSeqStore.getNextSenderSeqNum();
    }

    void setSeqNums(uint32_t in, uint32_t out) {
        session->mSeqStore.setSeqNums(in, out);
    }

    void setHeartbeatInterval(int seconds) {
        session->mHeartbeatInterval = seconds;
    }

    void changeState(FixClientState state) {
        session->changeState(state);
    }

    void handleProtocolMessage(const std::string& msg) {
        session->handleProtocolMessage(msg);
    }

    void startHeartbeatTimer() {
        session->startHeartbeatTimer();
    }

    std::unique_ptr<asio::io_context> ioContext;
    std::shared_ptr<FixClientSession> session;
};

TEST_F(FixClientSessionTests, InitialState) {
    EXPECT_EQ(session->getState(), FixClientState::Disconnected);
}

TEST_F(FixClientSessionTests, StateTransitionsManual) {
    // Testing the changeState helper
    changeState(FixClientState::Connected);
    EXPECT_EQ(session->getState(), FixClientState::Connected);
    
    changeState(FixClientState::Active);
    EXPECT_EQ(session->getState(), FixClientState::Active);
}

TEST_F(FixClientSessionTests, HandleLogonResponse) {
    changeState(FixClientState::LogonSent);
    
    // Simulate a Logon message (35=A) from server
    // 8=FIX.4.4|9=70|35=A|49=TEST_SERVER|56=TEST_CLIENT|34=1|52=20251030-10:00:00|98=0|108=30|10=...
    std::string logonMsg = "8=FIX.4.4\x01" "9=70\x01" "35=A\x01" "49=TEST_SERVER\x01" "56=TEST_CLIENT\x01" "34=1\x01" "10=000\x01";
    
    handleProtocolMessage(logonMsg);
    
    // Should transition to Active
    EXPECT_EQ(session->getState(), FixClientState::Active);
}

TEST_F(FixClientSessionTests, HandleLogoutResponse) {
    changeState(FixClientState::Active);
    
    std::string logoutMsg = "8=FIX.4.4\x01" "35=5\x01" "49=TEST_SERVER\x01" "56=TEST_CLIENT\x01" "34=2\x01" "10=000\x01";
    
    handleProtocolMessage(logoutMsg);
    
    // Should transition to Disconnected/Disconnected after processing logout
    EXPECT_EQ(session->getState(), FixClientState::Disconnected);
}

TEST_F(FixClientSessionTests, HandleHeartbeat) {
    changeState(FixClientState::Active);
    uint32_t initialSeq = getNextTargetSeqNum(); // Should be 1
    
    std::string hbMsg = "8=FIX.4.4\x01" "35=0\x01" "49=TEST_SERVER\x01" "56=TEST_CLIENT\x01" "34=1\x01" "10=000\x01";
    handleProtocolMessage(hbMsg);
    
    // Sequence should have increased
    EXPECT_EQ(getNextTargetSeqNum(), initialSeq + 1);
}

TEST_F(FixClientSessionTests, HandleTestRequest) {
    changeState(FixClientState::Active);
    
    // TestRequest (35=1) with Tag 112 (TestReqID)
    std::string trMsg = "8=FIX.4.4\x01" "35=1\x01" "112=TEST_123\x01" "34=1\x01" "10=000\x01";
    
    // This should trigger sending a Heartbeat with 112=TEST_123.
    // Since we don't have a real socket, it might log an error but we can verify the call happened if we had a mock.
    // For now, let's just ensure it doesn't crash and increases sequence.
    handleProtocolMessage(trMsg);
    EXPECT_EQ(getNextTargetSeqNum(), 2);
}

TEST_F(FixClientSessionTests, SequenceGapHandling) {
    changeState(FixClientState::Active);
    setSeqNums(1, 1);
    
    // Incoming message with SeqNum=10 (Gap! Expected 1)
    std::string gapMsg = "8=FIX.4.4\x01" "35=0\x01" "34=10\x01" "10=000\x01";
    
    handleProtocolMessage(gapMsg);
    
    // In our simplified client, we "sync up" to the gap to avoid death loops,
    // so it should move to 11.
    EXPECT_EQ(getNextTargetSeqNum(), 11);
}

TEST_F(FixClientSessionTests, ResendRequestProcessing) {
    changeState(FixClientState::Active);
    setSeqNums(1, 1);
    
    // Server requests resend (35=2) from 1 to 0 (infinity)
    std::string rrMsg = "8=FIX.4.4\x01" "35=2\x01" "7=1\x01" "16=0\x01" "34=1\x01" "10=000\x01";
    
    handleProtocolMessage(rrMsg);
    
    // Should increase target seq num to 2
    EXPECT_EQ(getNextTargetSeqNum(), 2);
}

TEST_F(FixClientSessionTests, SequenceResetHandle) {
    changeState(FixClientState::Active);
    
    // SequenceReset (35=4) with NewSeqNo (36)
    std::string srMsg = "8=FIX.4.4\x01" "35=4\x01" "36=100\x01" "34=1\x01" "10=000\x01";
    
    handleProtocolMessage(srMsg);
    
    // InSeq should jump to 100
    EXPECT_EQ(getNextTargetSeqNum(), 100);
}

TEST_F(FixClientSessionTests, FatalSequenceTooLow) {
    changeState(FixClientState::Active);
    setSeqNums(100, 100);
    
    // Server sends SeqNum=50 (Expected 100)
    std::string lowSeqMsg = "8=FIX.4.4\x01" "35=0\x01" "34=50\x01" "10=000\x01";
    
    handleProtocolMessage(lowSeqMsg);
    
    // Should disconnect
    EXPECT_EQ(session->getState(), FixClientState::Disconnected);
}

TEST_F(FixClientSessionTests, SendNewOrder) {
    changeState(FixClientState::Active);
    // This should not crash even if socket is not connected
    session->sendNewOrder("EURUSD", '1', 1.10, 1000);
    EXPECT_EQ(getNextSenderSeqNum(), 2);
}

TEST_F(FixClientSessionTests, SendMarketDataRequest) {
    changeState(FixClientState::Active);
    session->sendMarketDataRequest("EURUSD");
    EXPECT_EQ(getNextSenderSeqNum(), 2);
}

TEST_F(FixClientSessionTests, HeartbeatTimerActivation) {
    changeState(FixClientState::Active);
    setHeartbeatInterval(1);
    startHeartbeatTimer();
    // Verify it doesn't crash
}

TEST_F(FixClientSessionTests, SendLogonWithReset) {
    changeState(FixClientState::Connected);
    session->sendLogon(30, true); // forceReset = true
    EXPECT_EQ(getNextSenderSeqNum(), 2);
    EXPECT_EQ(session->getState(), FixClientState::LogonSent);
}

TEST_F(FixClientSessionTests, ApplicationMessageParsing) {
    changeState(FixClientState::Active);
    bool callbackFired = false;
    session->setMessageCallback([&](const ParsedFixMessage&) {
        callbackFired = true;
    });
    
    // Execution Report (35=8) with all required tags
    // 34=1 (Seq), 37=100 (OrderID), 11=200 (ClOrdID), 17=EXEC1 (ExecID), 150=0 (ExecType), 39=0 (Status), 
    // 55=EURUSD (Sym), 54=1 (Side), 38=1000 (Qty), 14=0 (Cum), 151=1000 (Leaves), 60=20231030-10:00:00 (Time)
    std::string erMsg = "8=FIX.4.4\x01" "9=120\x01" "35=8\x01" "34=1\x01" "37=100\x01" "11=200\x01" "17=EXEC1\x01" "150=0\x01" "39=0\x01" "55=EURUSD\x01" "54=1\x01" "38=1000\x01" "14=0\x01" "151=1000\x01" "60=20231030-10:00:00\x01" "10=064\x01";
    
    // Checksum calculation for the above string:
    // Manual calculation or just let it fail once to see the calculated value if I'm lazy, 
    // but let's try to be accurate. 
    // Actually, I'll just use a dummy value and see what the parser logs as calculated.
    // Wait, I can just use a helper to calculate it if I had one.
    // Let's use 10=051 (I'll check the log if it fails).
    handleProtocolMessage(erMsg);
    
    EXPECT_TRUE(callbackFired);
}

TEST_F(FixClientSessionTests, EmptyMsgTypeIgnored) {
    std::string badMsg = "8=FIX.4.4\x01" "34=1\x01" "10=000\x01"; // Missing 35
    handleProtocolMessage(badMsg);
    // Should not crash or change state
}

} // namespace fix_client
