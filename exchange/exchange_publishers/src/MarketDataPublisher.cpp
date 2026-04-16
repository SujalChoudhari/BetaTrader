#include <exchange_publishers/MarketDataPublisher.h>
#include "logging/Logger.h"
#include <algorithm>
#include <mutex>

namespace trading_core {

    void MarketDataPublisher::addSubscription(common::Symbol symbol, common::SessionID sessionId)
    {
        std::unique_lock lock(mMutex);
        auto& sessions = mSubscriptions[symbol];
        if (std::find(sessions.begin(), sessions.end(), sessionId) == sessions.end()) {
            sessions.push_back(sessionId);
            LOG_INFO("MarketDataPublisher: Adding subscription for Symbol {} from SessionID {}",
                     common::to_string(symbol), sessionId);
        }
    }

    void MarketDataPublisher::removeSubscription(common::Symbol symbol, common::SessionID sessionId)
    {
        std::unique_lock lock(mMutex);
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
        std::unique_lock lock(mMutex);
        for (auto it = mSubscriptions.begin(); it != mSubscriptions.end(); ) {
            auto& sessions = it->second;
            sessions.erase(std::remove(sessions.begin(), sessions.end(), sessionId), sessions.end());
            if (sessions.empty()) {
                it = mSubscriptions.erase(it);
            } else {
                ++it;
            }
        }
        LOG_INFO("MarketDataPublisher: Removing all subscriptions for SessionID {}", sessionId);
    }

    void MarketDataPublisher::publishSnapshot(const fix::MarketDataSnapshotFullRefresh& snapshot)
    {
        std::vector<SnapshotCallback> general;
        std::vector<SnapshotCallback> session;
        {
            std::shared_lock lock(mMutex);
            general = mGeneralSnapshotCallbacks;
            session = mSessionSnapshotCallbacks;
        }

        for (const auto& cb : general) {
            if (cb) cb(snapshot);
        }

        if (snapshot.targetSessionID != 0) {
            for (const auto& cb : session) {
                if (cb) cb(snapshot);
            }
        }
    }

    void MarketDataPublisher::publishIncremental(const fix::MarketDataIncrementalRefresh& refresh)
    {
        std::vector<IncrementalCallback> general;
        std::vector<IncrementalCallback> session;
        std::vector<common::SessionID> targetSessions;

        {
            std::shared_lock lock(mMutex);
            general = mGeneralIncrementalCallbacks;
            session = mSessionIncrementalCallbacks;
            
            auto it = mSubscriptions.find(refresh.symbol);
            if (it != mSubscriptions.end()) {
                targetSessions = it->second;
            }
        }

        // 1. Broadcast to general subscribers (OHLC Aggregator, etc.)
        for (const auto& cb : general) {
            if (cb) cb(refresh);
        }

        // 2. Broadcast to session-specific subscribers
        if (!session.empty() && !targetSessions.empty()) {
            for (auto sessionId : targetSessions) {
                fix::MarketDataIncrementalRefresh targetedRefresh = refresh;
                targetedRefresh.targetSessionID = sessionId;
                for (const auto& cb : session) {
                    if (cb) cb(targetedRefresh);
                }
            }
        }
    }

    void MarketDataPublisher::subscribeToSnapshots(SnapshotCallback callback)
    {
        std::unique_lock lock(mMutex);
        mSessionSnapshotCallbacks.push_back(std::move(callback));
    }

    void MarketDataPublisher::subscribeToIncrementals(IncrementalCallback callback)
    {
        std::unique_lock lock(mMutex);
        mSessionIncrementalCallbacks.push_back(std::move(callback));
    }

    void MarketDataPublisher::addGeneralIncrementalSubscriber(IncrementalCallback callback)
    {
        std::unique_lock lock(mMutex);
        mGeneralIncrementalCallbacks.push_back(std::move(callback));
    }

    void MarketDataPublisher::addGeneralSnapshotSubscriber(SnapshotCallback callback)
    {
        std::unique_lock lock(mMutex);
        mGeneralSnapshotCallbacks.push_back(std::move(callback));
    }

} // namespace trading_core
