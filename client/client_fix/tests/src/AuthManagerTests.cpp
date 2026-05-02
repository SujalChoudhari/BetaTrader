#include <gtest/gtest.h>
#include "fix_client/AuthManager.h"
#include "fix_client/FixClientSession.h"
#include <asio.hpp>
#include <string>

#include "logging/Logger.h"

namespace fix_client {

// Test setup requires an io_context to construct FixClientSession
class AuthManagerTests : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        logging::Logger::Init("auth_test_logger", "logs/auth_test.log", true, false);
    }

    void SetUp() override {
        ioContext = std::make_unique<asio::io_context>();
        session = std::make_shared<fix_client::FixClientSession>(*ioContext, "CLIENT", "SERVER");
        authManager = std::make_unique<fix_client::AuthManager>(session);
    }

    void setSessionState(fix_client::FixClientState state) {
        session->mState = state;
    }

    std::unique_ptr<asio::io_context> ioContext;
    std::shared_ptr<fix_client::FixClientSession> session;
    std::unique_ptr<fix_client::AuthManager> authManager;
};

TEST_F(AuthManagerTests, FailsIfSessionNotActive) {
    bool callbackFired = false;
    
    // Default state is Disconnected
    authManager->authenticate("user", "pass", [&](bool success, const std::string& msg) {
        callbackFired = true;
        EXPECT_FALSE(success);
        EXPECT_EQ(msg, "Session not active");
    });
    
    EXPECT_TRUE(callbackFired);
}

TEST_F(AuthManagerTests, HandlesSuccessfulAuthResponse) {
    bool callbackFired = false;
    std::string testReqId = "AUTH_123";
    
    // 1. Force session to Active for test
    setSessionState(fix_client::FixClientState::Active);
    
    authManager->setNextRequestId(testReqId);
    
    authManager->authenticate("user", "pass", [&](bool success, const std::string& msg) {
        callbackFired = true;
        EXPECT_TRUE(success);
        EXPECT_EQ(msg, "Welcome");
    });
    
    // 2. Simulate response
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=BF\x01" "923=" + testReqId + "\x01" "926=1\x01" "927=Welcome\x01" "10=111\x01";
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    
    EXPECT_TRUE(handled);
    EXPECT_TRUE(callbackFired);
}

TEST_F(AuthManagerTests, MismatchedRequestIdIgnored) {
    authManager->authenticate("user", "pass", nullptr);
    
    // A raw 35=BF message where 923 is different
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=BF\x01" "49=SERVER\x01" "56=CLIENT\x01" "34=1\x01" "52=20231030-10:00:00\x01" "923=WRONG_ID\x01" "926=1\x01" "10=111\x01";
    
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    EXPECT_FALSE(handled);
}

TEST_F(AuthManagerTests, FailsOnNonBFMessage) {
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=0\x01" "10=111\x01"; // Heartbeat
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    EXPECT_FALSE(handled);
}

TEST_F(AuthManagerTests, FailsOnMissingUserStatus) {
    // 35=BF but missing 926
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=BF\x01" "923=UNKNOWN\x01" "10=111\x01";
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    EXPECT_FALSE(handled);
}

TEST_F(AuthManagerTests, FailsOnMissingRequestIdTag) {
    // 35=BF but missing 923
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=BF\x01" "926=1\x01" "10=111\x01";
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    EXPECT_FALSE(handled);
}

TEST_F(AuthManagerTests, FailsOnMissingMsgTypeTag) {
    // Missing 35
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "923=REQ_1\x01" "10=111\x01";
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    EXPECT_FALSE(handled);
}

} // namespace fix_client

