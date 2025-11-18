#pragma once

#include "common/Time.h"
#include "common/Types.h" // For common::Price and common::Quantity
#include "common_fix/Protocol.h" // For MDEntryType
#include "common_fix/Types.h"
#include <chrono>
#include <string>
#include <vector>

namespace fix {

    struct MarketDataEntry {
        MDEntryType entryType; // FIX Tag 269
        common::Price price{};
        common::Quantity size{};
        common::Timestamp entryTime; // MDEntryTime (Tag 273)
        uint32_t entryPosition{}; // MDEntryPositionNo (Tag 290)
    };

    struct MarketDataSnapshotFullRefresh {
        fix::CompID targetSessionID; // Added for routing to specific session
        std::string mdReqID; // Market Data Request ID (FIX Tag 262)
        fix::Symbol symbol; // Symbol (FIX Tag 55)
        std::vector<MarketDataEntry> entries;
    };

} // namespace fix
