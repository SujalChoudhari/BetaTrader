#pragma once

#include "common/Types.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include <functional>
#include <map>
#include <vector>
#include <shared_mutex>

namespace trading_core {

    /**
     * @class MarketDataPublisher
     * @brief Manages market data subscriptions and broadcasts updates.
     *
     * This class handles internal subscriptions and routes snapshots and 
     * incremental updates to various interested parties (FIX sessions, UI, etc.).
     */
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
        
        void addGeneralIncrementalSubscriber(IncrementalCallback callback);
        void addGeneralSnapshotSubscriber(SnapshotCallback callback);

    private:
        mutable std::shared_mutex mMutex;
        std::map<common::Symbol, std::vector<common::SessionID>> mSubscriptions;
        std::vector<SnapshotCallback> mSessionSnapshotCallbacks;
        std::vector<IncrementalCallback> mSessionIncrementalCallbacks;
        std::vector<SnapshotCallback> mGeneralSnapshotCallbacks;
        std::vector<IncrementalCallback> mGeneralIncrementalCallbacks;
    };

} // namespace trading_core
