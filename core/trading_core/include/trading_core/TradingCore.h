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
#include <memory> // Required for std::unique_ptr

namespace trading_core {
    /**
     * @class TradingCore
     * @brief High-level manager for partitions and command dispatch.
     */
    class TradingCore {
    public:
        TradingCore();

        explicit TradingCore(data::DatabaseWorker* dbWorker,
                             bool autoInitPartitions = true);

        ~TradingCore();

        void start();

        void stop();

        void stopAcceptingCommands();

        void waitAllQueuesIdle() const;

        void submitCommand(std::unique_ptr<Command> command) const;

        Partition* getPartition(common::Instrument instrument) const;

        OrderIDGenerator* getOrderIDGenerator();

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
    };
} // namespace trading_core
