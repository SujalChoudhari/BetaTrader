#pragma once

#include "admin/ExchangeManager.h"
#include <string>
#include <vector>

namespace admin {

/**
 * @class ExchangePanel
 * @brief ImGui panel for controlling and monitoring the internal exchange state.
 */
class ExchangePanel {
public:
    explicit ExchangePanel(ExchangeManager& manager);
    ~ExchangePanel();

    /**
     * @brief Renders the exchange control dashboard.
     */
    void render();

private:
    ExchangeManager& mManager;
    int mPort = 8088;
    
    // UI persistent states
    bool mAutoRefresh = true;
    float mRefreshTimer = 0.0f;

    void renderControlSection();
    void renderSessionSection();
    void renderEngineSection();
};

} // namespace admin
