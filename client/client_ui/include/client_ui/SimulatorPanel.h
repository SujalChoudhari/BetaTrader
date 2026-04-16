#pragma once

#include "simulator/StochasticSimulator.h"
#include <memory>

namespace client_ui {

    /**
     * @class SimulatorPanel
     * @brief Controls for the background stochastic bot simulator.
     */
    class SimulatorPanel {
    public:
        SimulatorPanel();
        ~SimulatorPanel();

        void render(simulator::StochasticSimulator* simulator);

    private:
        int mTargetBots = 1000;
        float mIntensity = 1.0f;
        int mTargetSymbolIdx = 0; // 0 = ALL, 1+ = Instruments
    };

} // namespace client_ui
