#pragma once

#include "common/Instrument.h"
#include "fix_client/FixClientSession.h"
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>

namespace client_ui {

enum class MarketDataSubscriptionStatus {
    Disconnected,
    Ready,
    Subscribed
};

/**
 * Shared client-side state for the selected market-data symbol and its
 * subscription lifecycle. Panels read this state instead of keeping their
 * own symbol copies.
 */
class MarketDataState {
public:
    MarketDataState() = default;

    std::string symbol() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mSymbol;
    }

    bool selectSymbol(std::string_view symbol)
    {
        try {
            common::from_string(symbol);
        } catch (const std::invalid_argument&) {
            return false;
        }

        std::lock_guard<std::mutex> lock(mMutex);
        if (mSymbol == symbol) return true;
        mSymbol = symbol;
        if (mStatus == MarketDataSubscriptionStatus::Subscribed) {
            mStatus = MarketDataSubscriptionStatus::Ready;
        }
        return true;
    }

    bool canSubscribe(fix_client::FixClientState sessionState) const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return sessionState == fix_client::FixClientState::Active
               && mStatus != MarketDataSubscriptionStatus::Subscribed;
    }

    MarketDataSubscriptionStatus status() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mStatus;
    }

    void markSessionReady()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mStatus == MarketDataSubscriptionStatus::Disconnected) {
            mStatus = MarketDataSubscriptionStatus::Ready;
        }
    }

    void markSubscribed()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStatus = MarketDataSubscriptionStatus::Subscribed;
    }

    void markDisconnected()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStatus = MarketDataSubscriptionStatus::Disconnected;
    }

private:
    mutable std::mutex mMutex;
    std::string mSymbol = "EURUSD";
    MarketDataSubscriptionStatus mStatus = MarketDataSubscriptionStatus::Disconnected;
};

} // namespace client_ui
