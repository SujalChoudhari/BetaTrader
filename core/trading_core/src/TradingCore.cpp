//
// Created by sujal on 30-10-2025.
//

#include "trading_core/TradingCore.h"
#include "data/Constant.h"
#include "trading_core/CancelOrder.h"
#include "trading_core/ModifyOrder.h"
#include "trading_core/NewOrder.h"
#include "trading_core/TradingCoreRunbookDefinations.h"

namespace {
    trading_core::TradingCore* g_instance = nullptr;
}

namespace trading_core {
    TradingCore::TradingCore()
    {
        mOwnedDatabaseWorker
                = std::make_unique<data::DatabaseWorker>(data::databasePath);
        mDatabaseWorker = mOwnedDatabaseWorker.get();
        mTradeIDGenerator = std::make_unique<TradeIDGenerator>(mDatabaseWorker);
        mOrderIDGenerator = std::make_unique<OrderIDGenerator>(mDatabaseWorker);
        initPartitions();
        g_instance = this;
    }

    TradingCore::TradingCore(data::DatabaseWorker* dbWorker,
                             const bool autoInitPartitions)
        : mDatabaseWorker(dbWorker)
    {
        if (mDatabaseWorker) {
            mTradeIDGenerator = std::make_unique<TradeIDGenerator>(mDatabaseWorker);
            mOrderIDGenerator = std::make_unique<OrderIDGenerator>(mDatabaseWorker);
        }
        if (autoInitPartitions) { initPartitions(); }
        g_instance = this;
    }

    TradingCore::~TradingCore()
    {
        stop();
        g_instance = nullptr;
    };

    void TradingCore::start()
    {
        for (const auto& mPartition: mPartitions) {
            if (mPartition) {
                mPartition->waitReady();
                mPartition->start();
            }
        }
    }

    void TradingCore::stop()
    {
        for (const auto& mPartition: mPartitions) {
            if (mPartition) { mPartition->stop(); }
        }
    }

    void TradingCore::stopAcceptingCommands()
    {
        for (const auto& partition: mPartitions) {
            if (partition) { partition->stopAcceptingCommands(); }
        }
    }

    void TradingCore::waitAllQueuesIdle() const
    {
        while (true) {
            size_t total_queue_size = 0;
            for (const auto& partition: mPartitions) {
                if (partition) total_queue_size += partition->getQueueSize();
            }
            if (mDatabaseWorker)
                total_queue_size += mDatabaseWorker->getQueueSize();

            if (total_queue_size == 0) { break; }

            std::this_thread::yield();
        }
    }

    void TradingCore::submitCommand(std::unique_ptr<Command> command) const
    {
        if (command->getType() == CommandType::NewOrder) {
            const auto newOrder = dynamic_cast<NewOrder*>(command.get());
            if (!newOrder) {
                LOG_ERROR(errors::ETRADE3, "Invalid NewOrder cast");
                return;
            }
            auto instrument = newOrder->getOrder()->getSymbol();
            mPartitions[static_cast<int>(instrument)]->enqueue(
                    std::move(command));
            return;
        }

        if (command->getType() == CommandType::ModifyOrder) {
            const auto order = dynamic_cast<ModifyOrder*>(command.get());
            if (!order) {
                LOG_ERROR(errors::ETRADE3, "Invalid ModifyOrder cast");
                return;
            }
            auto instrumentOpt = findPartitionForOrder(order->getOrderId());
            if (!instrumentOpt) {
                LOG_ERROR(errors::ETRADE2, "Modify Order ID not found");
                return;
            }
            mPartitions[static_cast<int>(*instrumentOpt)]->enqueue(
                    std::move(command));
            return;
        }

        if (command->getType() == CommandType::CancelOrder) {
            const auto order = dynamic_cast<CancelOrder*>(command.get());
            if (!order) {
                LOG_ERROR(errors::ETRADE3, "Invalid CancelOrder cast");
                return;
            }
            auto instrumentOpt = findPartitionForOrder(order->getOrderId());
            if (!instrumentOpt) {
                LOG_ERROR(errors::ETRADE2, "Cancel Order ID not found");
                return;
            }
            mPartitions[static_cast<int>(*instrumentOpt)]->enqueue(
                    std::move(command));
            return;
        }

        LOG_ERROR(errors::ETRADE3, "Unknown command type");
    }

    void TradingCore::initPartitions()
    {
        for (int i = 0; i < static_cast<int>(common::Instrument::COUNT); ++i) {
            auto instrument = static_cast<common::Instrument>(i);
            mPartitions[i] = std::make_unique<Partition>(
                    instrument, mDatabaseWorker, mTradeIDGenerator.get());
        }
    }

    Partition* TradingCore::getPartition(common::Instrument instrument) const
    {
        return mPartitions[static_cast<int>(instrument)].get();
    }

    OrderIDGenerator* TradingCore::getOrderIDGenerator()
    {
        return mOrderIDGenerator.get();
    }

    void TradingCore::subscribeToExecutions(ExecutionReportCallback callback) {
        mExecutionReportCallback = callback;
    }

    void TradingCore::subscribeToMarketDataSnapshots(MarketDataSnapshotCallback callback) {
        mMarketDataSnapshotCallback = callback;
    }

    void TradingCore::subscribeToMarketDataIncrements(MarketDataIncrementalCallback callback) {
        mMarketDataIncrementalCallback = callback;
    }

    const TradingCore::ExecutionReportCallback& TradingCore::getExecutionReportCallback() const {
        return mExecutionReportCallback;
    }

    const TradingCore::MarketDataSnapshotCallback& TradingCore::getMarketDataSnapshotCallback() const {
        return mMarketDataSnapshotCallback;
    }

    const TradingCore::MarketDataIncrementalCallback& TradingCore::getMarketDataIncrementalCallback() const {
        return mMarketDataIncrementalCallback;
    }

    TradingCore& TradingCore::getInstance() {
        return *g_instance;
    }

    std::optional<common::Instrument>
    TradingCore::findPartitionForOrder(common::OrderID orderId) const
    {
        for (const auto& partition: mPartitions) {
            if (partition
                && partition->getOrderManager()->containsOrderById(orderId)) {
                return partition->getSymbol();
            }
        }
        return std::nullopt;
    }

#ifndef NDEBUG
    void TradingCore::setPartition(common::Instrument instrument,
                                   std::unique_ptr<Partition> partition)
    {
        mPartitions[static_cast<int>(instrument)] = std::move(partition);
    }
#endif
} // namespace trading_core
