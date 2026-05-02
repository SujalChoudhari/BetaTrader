#pragma once

#include <admin/ExchangeManager.h>

namespace admin {

/**
 * @class ExchangePanel
 * @brief ImGui panel for controlling the backend processes.
 */
class ExchangePanel {
public:
    ExchangePanel(ExchangeManager& manager);

    void render();

private:
    ExchangeManager& mManager;
    int mNumAgents = 50;
};

} // namespace admin
