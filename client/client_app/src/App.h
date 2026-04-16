#pragma once

#include "client_ui/UIManager.h"
#include "client_ui/ConnectionPanel.h"
#include "admin/ExchangeManager.h"
#include "admin/ExchangePanel.h"
#include <asio.hpp>

#include <thread>
#include <memory>

namespace client_app {

/**
 * @class App
 * @brief Coordinates the lifecycle of the network and UI threads.
 */
class App {
public:
    App();
    ~App();

    /**
     * @brief Entry point that runs the application until closure.
     */
    int run();

private:
    client_ui::UIManager mUI;
    client_ui::ConnectionPanel mConnPanel;
    
    admin::ExchangeManager mExchMgr;
    admin::ExchangePanel mExchPanel;
    
    asio::io_context mIoCtx;

    asio::executor_work_guard<asio::io_context::executor_type> mWork;
    std::thread mNetworkThread;

    std::shared_ptr<fix_client::FixClientSession> mFixSession;
};

} // namespace client_app
