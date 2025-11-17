/**
 * @file TradingCore.h
 * @brief Top-level entry point for the trading core service.
 *
 * Declares the `TradingCore` which manages partitions, the database worker,
 * and the lifecycle of the trading engine.
 */

#pragma once
#include "Command.h"
#include "OrderIDGenerator.h"
#include "Partition.h"
#include "TradeIDGenerator.h"
#include "data/DatabaseWorker.h"
#include "common/Order.h"
#include "fix/ExecutionReport.h" // Include the report type
#include "fix/MarketDataSnapshotFullRefresh.h" // New: Include market data snapshot type
#include "fix/MarketDataIncrementalRefresh.h" // New: Include market data incremental type
#include <memory>
#include <functional>

namespace trading_core {
    /**
     * @class TradingCore
     * @brief High-level manager for partitions and command dispatch.
     */
    class TradingCore {
    public:
        // Callback now publishes the fully-formed FIX ExecutionReport
        using ExecutionReportCallback = std::function<void(const fix::ExecutionReport&)>;
        // New: Callback for market data snapshot full refresh
        using MarketDataSnapshotCallback = std::function<void(const fix::MarketDataSnapshotFullRefresh&)>;
        // New: Callback for market data incremental refresh
        using MarketDataIncrementalCallback = std::function<void(const fix::MarketDataIncrementalRefresh&)>;


        TradingCore();

        explicit TradingCore(data::DatabaseWorker* dbWorker,
                             bool autoInitPartitions = true);

        virtual ~TradingCore();

        void start();

        void stop();

        void stopAcceptingCommands();

        void waitAllQueuesIdle() const;

        virtual void submitCommand(std::unique_ptr<Command> command) const;

        Partition* getPartition(common::Instrument instrument) const;

        OrderIDGenerator* getOrderIDGenerator();

        void subscribeToExecutions(ExecutionReportCallback callback);
        // New: Subscribe to market data snapshots
        void subscribeToMarketDataSnapshots(MarketDataSnapshotCallback callback);
        // New: Subscribe to market data incremental refreshes
        void subscribeToMarketDataIncrements(MarketDataIncrementalCallback callback);


        const ExecutionReportCallback& getExecutionReportCallback() const;
        // New: Get market data snapshot callback
        const MarketDataSnapshotCallback& getMarketDataSnapshotCallback() const;
        // New: Get market data incremental callback
        const MarketDataIncrementalCallback& getMarketDataIncrementalCallback() const;


        static TradingCore& getInstance();


#ifndef NDEBUG
        void setPartition(common::Instrument instrument,
                          std::unique_ptr<Partition> partition);
#endif

    private:
        void initPartitions();

        std::optional<common::Instrument>
        findPartitionForOrder(common::OrderID orderId) const;

    private:
        data::DatabaseWorker* mDatabaseWorker = nullptr;
        std::unique_ptr<data::DatabaseWorker> mOwnedDatabaseWorker;
        std::unique_ptr<TradeIDGenerator> mTradeIDGenerator;
        std::unique_ptr<OrderIDGenerator> mOrderIDGenerator;
        std::unique_ptr<Partition>
                mPartitions[static_cast<int>(common::Instrument::COUNT)];
        ExecutionReportCallback mExecutionReportCallback;
        // New: Market data callbacks
        MarketDataSnapshotCallback mMarketDataSnapshotCallback;
        MarketDataIncrementalCallback mMarketDataIncrementalCallback;
    };
} // namespace trading_core
