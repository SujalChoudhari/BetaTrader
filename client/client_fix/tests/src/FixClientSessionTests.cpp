#include <gtest/gtest.h>
#define private public  // Alternative to friend if header edit is risky, but we used friend
#include "fix_client/FixClientSession.h"
#undef private
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

} // namespace fix_client
