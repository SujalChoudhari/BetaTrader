#include <exchange_publishers/MarketDataPublisher.h>
#include "logging/Logger.h"
#include <algorithm>

namespace trading_core {

    void MarketDataPublisher::addSubscription(common::Symbol symbol, common::SessionID sessionId)
    {
        auto& sessions = mSubscriptions[symbol];
        if (std::find(sessions.begin(), sessions.end(), sessionId) == sessions.end()) {
            sessions.push_back(sessionId);
            LOG_INFO("MarketDataPublisher: Adding subscription for Symbol {} from SessionID {}",
                     common::to_string(symbol), sessionId);
        }
    }

    void MarketDataPublisher::removeSubscription(common::Symbol symbol, common::SessionID sessionId)
    {
        auto it = mSubscriptions.find(symbol);
        if (it != mSubscriptions.end()) {
            auto& sessions = it->second;
            sessions.erase(std::remove(sessions.begin(), sessions.end(), sessionId), sessions.end());
            LOG_INFO("MarketDataPublisher: Removing subscription for Symbol {} from SessionID {}",
                     common::to_string(symbol), sessionId);
            if (sessions.empty()) {
                mSubscriptions.erase(it);
            }
        }
    }

    void MarketDataPublisher::removeSubscription(common::SessionID sessionId)
    {
        for (auto it = mSubscriptions.begin(); it != mSubscriptions.end(); ) {
            auto& sessions = it->second;
            sessions.erase(std::remove(sessions.begin(), sessions.end(), sessionId), sessions.end());
            LOG_INFO("MarketDataPublisher: Removing all subscriptions for SessionID {}", sessionId);
            if (sessions.empty()) {
                it = mSubscriptions.erase(it);
            } else {
                ++it;
            }
        }
    }

    void MarketDataPublisher::publishSnapshot(const fix::MarketDataSnapshotFullRefresh& snapshot)
    {
        if (mSnapshotCallback) {
            LOG_INFO("MarketDataPublisher: Publishing snapshot for Symbol {} to SessionID {}",
                     common::to_string(snapshot.symbol), snapshot.targetSessionID);
            mSnapshotCallback(snapshot);
        }
    }

    void MarketDataPublisher::publishIncremental(const fix::MarketDataIncrementalRefresh& refresh)
    {
        if (mIncrementalCallback) {
            auto it = mSubscriptions.find(refresh.symbol);
            if (it != mSubscriptions.end()) {
                LOG_INFO("MarketDataPublisher: Publishing incremental for Symbol {}", common::to_string(refresh.symbol));
                for (auto sessionId : it->second) {
                    fix::MarketDataIncrementalRefresh targetedRefresh = refresh;
                    targetedRefresh.targetSessionID = sessionId;
                    mIncrementalCallback(targetedRefresh);
                }
            }
        }
    }

    void MarketDataPublisher::subscribeToSnapshots(SnapshotCallback callback)
    {
        mSnapshotCallback = callback;
    }

    void MarketDataPublisher::subscribeToIncrementals(IncrementalCallback callback)
    {
        mIncrementalCallback = callback;
    }

} // namespace trading_core
