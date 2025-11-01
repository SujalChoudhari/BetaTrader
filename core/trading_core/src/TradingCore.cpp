//
// Created by sujal on 30-10-2025.
//

#include "trading_core/TradingCore.h"

#include "data/Constant.h"
#include "data/DataRunBookDefinations.h"
#include "trading_core/TradingCoreRunbookDefinations.h"
#include "trading_core/NewOrder.h"
#include "trading_core/ModifyOrder.h"
#include "trading_core/CancelOrder.h"

namespace trading_core {
    TradingCore::TradingCore() {
        mDatabaseWorker = std::make_unique<data::DatabaseWorker>(data::databasePath);
        mTradeIDGenerator = std::make_unique<TradeIDGenerator>(mDatabaseWorker.get());
        initPartitions();
    }

    TradingCore::~TradingCore() {
        stop();
    };

    void TradingCore::start() const {
        for (const auto &mPartition: mPartitions) {
            mPartition->start();
        }
    }

    void TradingCore::stop() const {
        for (const auto &mPartition: mPartitions) {
            mPartition->stop();
        }
    }

    void TradingCore::waitUntilIdle() const {
        while (true) {
            size_t total_queue_size = 0;
            for (const auto &partition : mPartitions) {
                total_queue_size += partition->getQueueSize();
            }
            total_queue_size += mDatabaseWorker->getQueueSize();

            if (total_queue_size == 0) {
                break;
            }

            std::this_thread::yield();
        }
    }

    void TradingCore::submitCommand(std::unique_ptr<Command> command) const {
        if (command->getType() == CommandType::NewOrder) {
            const auto newOrder = dynamic_cast<NewOrder *>(command.get());
            if (!newOrder) {
                LOG_ERROR(errors::ETRADE3, "Invalid NewOrder cast");
                return;
            }
            auto instrument = newOrder->getOrder()->getSymbol();
            mPartitions[static_cast<int>(instrument)]->enqueue(std::move(command));
            return;
        }

        if (command->getType() == CommandType::ModifyOrder) {
            const auto order = dynamic_cast<ModifyOrder *>(command.get());
            if (!order) {
                LOG_ERROR(errors::ETRADE3, "Invalid ModifyOrder cast");
                return;
            }
            auto instrumentOpt = findPartitionForOrder(order->getOrderId());
            if (!instrumentOpt) {
                LOG_ERROR(errors::ETRADE2, "Modify Order ID not found");
                return;
            }
            mPartitions[static_cast<int>(*instrumentOpt)]->enqueue(std::move(command));
            return;
        }


        if (command->getType() == CommandType::CancelOrder) {
            const auto order = dynamic_cast<CancelOrder *>(command.get());
            if (!order) {
                LOG_ERROR(errors::ETRADE3, "Invalid CancelOrder cast");
                return;
            }
            auto instrumentOpt = findPartitionForOrder(order->getOrderId());
            if (!instrumentOpt) {
                LOG_ERROR(errors::ETRADE2, "Cancel Order ID not found");
                return;
            }
            mPartitions[static_cast<int>(*instrumentOpt)]->enqueue(std::move(command));
            return;
        }


        LOG_ERROR(errors::ETRADE3, "Unknown command type");
    }

    void TradingCore::initPartitions() {
        for (int i = 0; i < static_cast<int>(common::Instrument::COUNT); ++i) {
            auto instrument = static_cast<common::Instrument>(i);
            mPartitions[i] = std::make_unique<Partition>(
                instrument,
                mDatabaseWorker.get(),
                mTradeIDGenerator.get()
            );
        }
    }

    std::optional<common::Instrument> TradingCore::findPartitionForOrder(common::OrderID orderId) const {
        for (const auto &partition: mPartitions) {
            if (partition->getOrderManager()->containsOrderById(orderId)) {
                return partition->getSymbol();
            }
        }
        return std::nullopt;
    }
}
