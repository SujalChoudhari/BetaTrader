#pragma once

#include "common/Types.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include <functional>
#include <map>
#include <vector>

namespace trading_core {

    class MarketDataPublisher {
    public:
        using SnapshotCallback = std::function<void(const fix::MarketDataSnapshotFullRefresh&)>;
        using IncrementalCallback = std::function<void(const fix::MarketDataIncrementalRefresh&)>;

        void addSubscription(common::Symbol symbol, common::SessionID sessionId);
        void removeSubscription(common::Symbol symbol, common::SessionID sessionId);
        void removeSubscription(common::SessionID sessionId);

        virtual void publishSnapshot(const fix::MarketDataSnapshotFullRefresh& snapshot);
        virtual void publishIncremental(const fix::MarketDataIncrementalRefresh& refresh);

        void subscribeToSnapshots(SnapshotCallback callback);
        void subscribeToIncrementals(IncrementalCallback callback);

    private:
        std::map<common::Symbol, std::vector<common::SessionID>> mSubscriptions;
        SnapshotCallback mSnapshotCallback;
        IncrementalCallback mIncrementalCallback;
    };

} // namespace trading_core
