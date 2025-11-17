#pragma once

#include <string>
#include <vector>
#include "fix/Types.h" // For fix::CompID, fix::Price, fix::Quantity, fix::Symbol

namespace fix
{

struct MarketDataEntry
{
    char entryType; // '0' = Bid, '1' = Offer, '2' = Trade (FIX Tag 269)
    fix::Price price;
    fix::Quantity size;
    // Additional fields like MDEntryTime, MDEntryPositionNo can be added here
};

struct MarketDataSnapshotFullRefresh
{
    fix::CompID targetSessionID; // Added for routing to specific session
    std::string mdReqID;         // Market Data Request ID (FIX Tag 262)
    fix::Symbol symbol;          // Symbol (FIX Tag 55)
    std::vector<MarketDataEntry> entries;
};

} // namespace fix
