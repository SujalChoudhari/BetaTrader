//
// Created by sujal on 30-10-2025.
//

#pragma once
#include "Command.h"
#include "Partition.h"
#include "TradeIDGenerator.h"
#include "data/DatabaseWorker.h"
#include <memory> // Required for std::unique_ptr

namespace trading_core {
    class TradingCore {
    public:
        TradingCore();

        ~TradingCore();

        void start() const;

        void stop() const;

        void waitUntilIdle() const;

        void submitCommand(std::unique_ptr<Command> command) const;

    private:
        void initPartitions();

        std::optional<common::Instrument> findPartitionForOrder(common::OrderID orderId) const;


    private:
        std::unique_ptr<data::DatabaseWorker> mDatabaseWorker;
        std::unique_ptr<TradeIDGenerator> mTradeIDGenerator;
        std::unique_ptr<Partition> mPartitions[static_cast<int>(common::Instrument::COUNT)];
    };
}
