//
// Created by sujal on 30-10-2025.
//

#pragma once
#include "Command.h"
#include "ExecutionPublisher.h"
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

        void submitCommand(std::unique_ptr<Command> command) const;

    private:
        void initPartitions();

        std::optional<common::Instrument> findPartitionForOrder(common::OrderID orderId) const;


    private:
        std::shared_ptr<data::DatabaseWorker> mDatabaseWorker;
        std::shared_ptr<TradeIDGenerator> mTradeIDGenerator;
        std::shared_ptr<ExecutionPublisher> mExecutionPublisher;
        std::unique_ptr<Partition> mPartitions[static_cast<int>(common::Instrument::COUNT)];
    };
}
