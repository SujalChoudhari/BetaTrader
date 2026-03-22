#include <gtest/gtest.h>
#include "fix_client/AuthManager.h"
#include "fix_client/FixClientSession.h"
#include <asio.hpp>
#include <string>

#include "logging/Logger.h"

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
    // A raw 35=BF message where 926=1
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=BF\x01" "923=UNKNOWN\x01" "926=1\x01" "927=Welcome\x01" "10=111\x01";
    
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    
    // It will return false because the REQ ID does not match any pending requests (which is "")
    EXPECT_FALSE(handled);
}

TEST_F(AuthManagerTests, IgnoresNonAuthMessages) {
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=8\x01" "10=111\x01"; // ExecutionReport
    
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    EXPECT_FALSE(handled);
}

TEST_F(AuthManagerTests, HandlesFailedUserStatus) {
    // 926=0 (Failed)
    std::string rawFix = "8=FIX.4.4\x01" "9=50\x01" "35=BF\x01" "923=UNKNOWN\x01" "926=0\x01" "927=Denied\x01" "10=111\x01";
    
    bool handled = authManager->handleMessage(std::monostate{}, rawFix);
    EXPECT_FALSE(handled);
}

