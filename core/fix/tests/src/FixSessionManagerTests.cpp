#include <gtest/gtest.h>
#include "fix/FixSessionManager.h"

using namespace fix;

class FixSessionManagerTest : public ::testing::Test {
protected:
    FixSessionManager manager;

    void SetUp() override {
        manager.loadConfig({"TRADER_BOB", "ALICE_FIRM"}); // Load standard dummy clients like "TRADER_BOB"
    }

    void TearDown() override {
    }
};

// 1. Authentication Tests
TEST_F(FixSessionManagerTest, ShouldAcceptValidLogon) {
    uint32_t sessionId = 1;
    bool result = manager.authenticate(sessionId, "TRADER_BOB");
    
    EXPECT_TRUE(result);
    
    SessionState* state = manager.getSessionState(sessionId);
    ASSERT_NE(state, nullptr);
    EXPECT_TRUE(state->isLoggedOn);
}

TEST_F(FixSessionManagerTest, ShouldRejectInvalidLogon) {
    uint32_t sessionId = 2;
    bool result = manager.authenticate(sessionId, "HACKER_EVE");
    
    EXPECT_FALSE(result);
    
    SessionState* state = manager.getSessionState(sessionId);
    EXPECT_EQ(state, nullptr); // State should not be created for invalid logon
}

// 2. Sequence Number Validation Tests
TEST_F(FixSessionManagerTest, ShouldIncrementInSeqNumOnValidMessage) {
    uint32_t sessionId = 3;
    manager.authenticate(sessionId, "ALICE_FIRM"); // Must logon first
    
    // First message should be SeqNum 1 (Logon itself would have been 1, so let's say next is 2)
    SessionState* state = manager.getSessionState(sessionId);
    state->inSeqNum = 1; // Simulate Logon taking SeqNum 1
    
    bool result = manager.validateSequence(sessionId, 2);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(state->inSeqNum, 2);
}

TEST_F(FixSessionManagerTest, ShouldDetectGapOnSequenceTooHigh) {
    uint32_t sessionId = 4;
    manager.authenticate(sessionId, "TRADER_BOB");
    SessionState* state = manager.getSessionState(sessionId);
    state->inSeqNum = 1;

    // Receive message 3 when expecting 2
    bool result = manager.validateSequence(sessionId, 3);
    
    EXPECT_FALSE(result); 
    // State seq num should remain 1 until gap is filled
    EXPECT_EQ(state->inSeqNum, 1);
}

TEST_F(FixSessionManagerTest, ShouldRejectOnSequenceTooLow) {
    uint32_t sessionId = 5;
    manager.authenticate(sessionId, "ALICE_FIRM");
    SessionState* state = manager.getSessionState(sessionId);
    state->inSeqNum = 5;

    // Receive message 2 when expecting 6 (Fatal error in FIX)
    bool result = manager.validateSequence(sessionId, 2);
    
    EXPECT_FALSE(result);
}

// 3. Logout Tests
TEST_F(FixSessionManagerTest, ShouldHandleLogout) {
    uint32_t sessionId = 6;
    manager.authenticate(sessionId, "TRADER_BOB");
    
    manager.handleLogout(sessionId);
    
    SessionState* state = manager.getSessionState(sessionId);
    ASSERT_NE(state, nullptr);
    EXPECT_FALSE(state->isLoggedOn);
}

TEST_F(FixSessionManagerTest, ShouldRejectSequenceValidationForUnauthenticated) {
    // Session not found
    EXPECT_FALSE(manager.validateSequence(99, 1));
    
    // Session found but not logged on
    uint32_t sessionId = 10;
    manager.authenticate(sessionId, "TRADER_BOB");
    manager.handleLogout(sessionId);
    EXPECT_FALSE(manager.validateSequence(sessionId, 5));
}

