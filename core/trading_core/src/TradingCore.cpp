//
// Created by sujal on 30-10-2025.
//

#include "trading_core/TradingCore.h"

#include "data/Constant.h"
#include "data/DataRunBookDefinations.h"
#include "trading_core/TradingCoreRunbookDefinations.h"

namespace trading_core {
    TradingCore::TradingCore() {
        mDatabaseWorker = std::make_unique<data::DatabaseWorker>(data::databasePath);
        mExecutionPublisher = std::make_shared<ExecutionPublisher>();
        mTradeIDGenerator = std::make_shared<TradeIDGenerator>(mDatabaseWorker);
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

    void TradingCore::submitCommand(Command *command) const {
        // Use unique_ptr for RAII
        std::unique_ptr<Command> cmd(command);

        if (cmd->getType() == CommandType::NewOrder) {
            const auto newOrder = dynamic_cast<NewOrder *>(cmd.get());
            if (!newOrder) {
                LOG_ERROR(errors::ETRADE3, "Invalid NewOrder cast");
                return; // cmd auto-deleted
            }
            auto instrument = newOrder->getOrder()->getSymbol();
            mPartitions[static_cast<int>(instrument)]->enqueue(cmd.release());
            return;
        }

        if (cmd->getType() == CommandType::ModifyOrder) {
            const auto order = dynamic_cast<ModifyOrder *>(cmd.get());
            if (!order) {
                LOG_ERROR(errors::ETRADE3, "Invalid ModifyOrder cast");
                return;
            }
            auto instrumentOpt = findPartitionForOrder(order->getOrderId());
            if (!instrumentOpt) {
                LOG_ERROR(errors::ETRADE2, "Modify Order ID not found");
                return;
            }
            mPartitions[static_cast<int>(*instrumentOpt)]->enqueue(cmd.release());
            return;
        }


        if (cmd->getType() == CommandType::CancelOrder) {
            const auto order = dynamic_cast<CancelOrder *>(cmd.get());
            if (!order) {
                LOG_ERROR(errors::ETRADE3, "Invalid CancelOrder cast");
                return;
            }
            auto instrumentOpt = findPartitionForOrder(order->getOrderId());
            if (!instrumentOpt) {
                LOG_ERROR(errors::ETRADE2, "Cancel Order ID not found");
                return;
            }
            mPartitions[static_cast<int>(*instrumentOpt)]->enqueue(cmd.release());
            return;
        }


        LOG_ERROR(errors::ETRADE3, "Unknown command type");
    }

    void TradingCore::initPartitions() {
        for (int i = 0; i < static_cast<int>(common::Instrument::COUNT); ++i) {
            auto instrument = static_cast<common::Instrument>(i);
            mPartitions[i] = std::make_unique<Partition>(
                instrument,
                mDatabaseWorker,
                mTradeIDGenerator,
                mExecutionPublisher
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
