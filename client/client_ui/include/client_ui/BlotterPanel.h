#pragma once

#include "blotter/BlotterModel.h"

namespace client_ui {

    /**
     * Renders the thread-safe client order and execution snapshots.
     * This panel is read-only and does not expose order mutation controls.
     */
    class BlotterPanel {
    public:
        void render(const client_blotter::BlotterModel& model) const;
    };

} // namespace client_ui
