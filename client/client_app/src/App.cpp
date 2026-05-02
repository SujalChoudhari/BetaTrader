#include "App.h"
#include "logging/Logger.h"
#include <iostream>

namespace client_app {

App::App() : mWork(asio::make_work_guard(mIoCtx)), mExchMgr(), mExchPanel(mExchMgr) {

    mNetworkThread = std::thread([this]() {
        try {
            mIoCtx.run();
        } catch (const std::exception& e) {
            LOG_CRITICAL("Network thread exception: {}", e.what());
        }
    });
}

App::~App() {
    mWork.reset();
    mIoCtx.stop();

    if (mNetworkThread.joinable()) {
        mNetworkThread.join();
    }
    mUI.stop();
}

int App::run() {
    logging::Logger::Init("client_app", "logs/client_app.log", true, true);
    LOG_INFO("Starting BetaTrader Client...");

    if (!mUI.start("BetaTrader Client Dashboard", 1280, 720)) {
        return 1;
    }

    while (!mUI.shouldClose()) {
        mUI.beginFrame();
        mUI.renderDockspace();

        // Render Panels
        mConnPanel.render(mFixSession, mIoCtx);
        mExchPanel.render();


        mUI.endFrame();
    }

    LOG_INFO("Shutting down BetaTrader Client...");
    return 0;
}

} // namespace client_app
