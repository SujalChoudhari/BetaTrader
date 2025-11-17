#pragma once

#include <string>
#include <vector>
#include "fix/Types.h" // For fix::Symbol

namespace fix
{

struct MarketDataRequest
{
    std::string mdReqID;
    char subscriptionRequestType; // '0' = Snapshot, '1' = Snapshot + Updates, '2' = Unsubscribe
    int marketDepth;              // '0' = Full Book, '1' = Top of Book / Level 1
    std::vector<fix::Symbol> symbols; // Using fix::Symbol
};

} // namespace fix
