#include "trading_core/Partition.h"
#include "logging/Logger.h"
#include "trading_core/TradingCoreRunbookDefinations.h"
#include <future>

namespace trading_core {
    Partition::Partition(common::Instrument symbol,
                         data::DatabaseWorker* databaseWorker,
                         TradeIDGenerator* tradeIDGenerator)
        : mCommandQueue(262144), mSymbol(symbol),
          mDatabaseWorker(databaseWorker), mTradeIDGenerator(tradeIDGenerator)
    {
        mTradeRepository
                = std::make_unique<data::TradeRepository>(mDatabaseWorker);
        mOrderRepository
                = std::make_unique<data::OrderRepository>(mDatabaseWorker);
        mOrderManager = std::make_unique<OrderManager>();
        mOrderBook = std::make_unique<OrderBook>();
        mMatcher = std::make_unique<Matcher>(mTradeIDGenerator);
        mRiskManager = std::make_unique<RiskManager>(mTradeRepository.get());
        init();
    }

    Partition::Partition(common::Instrument symbol,
                         data::DatabaseWorker* databaseWorker,
                         TradeIDGenerator* tradeIDGenerator,
                         std::unique_ptr<data::TradeRepository> tradeRepository,
                         std::unique_ptr<data::OrderRepository> orderRepository,
                         std::unique_ptr<OrderManager> orderManager,
                         std::unique_ptr<OrderBook> orderBook,
                         std::unique_ptr<Matcher> matcher,
                         std::unique_ptr<RiskManager> riskManager)
        : mCommandQueue(262144), mSymbol(symbol),
          mDatabaseWorker(databaseWorker), mTradeIDGenerator(tradeIDGenerator),
          mTradeRepository(std::move(tradeRepository)),
          mOrderRepository(std::move(orderRepository)),
          mOrderManager(std::move(orderManager)),
          mOrderBook(std::move(orderBook)), mMatcher(std::move(matcher)),
          mRiskManager(std::move(riskManager))
    {
        init();
    }

    void Partition::init()
    {
        auto loadPromise = std::make_shared<std::promise<void>>();
        mReadyFuture = loadPromise->get_future();

        mOrderRepository->loadOrdersForInstrument(
                mSymbol,
                [this, loadPromise](std::vector<common::Order> orders) {
                    for (auto& order: orders) {
                        auto orderUniquePtr
                                = std::make_unique<common::Order>(order);
                        mOrderBook->insertOrder(orderUniquePtr.get());
                        mOrderManager->addOrder(std::move(orderUniquePtr));
                    }
                    LOG_INFO("Loaded {} orders for symbol {}", orders.size(),
                             common::to_string(mSymbol));
                    loadPromise->set_value(); // Signal that loading is complete
                });

        LOG_INFO("Partition for symbol {} constructed and loading started.",
                 common::to_string(mSymbol));
    }

    Partition::~Partition()
    {
        stop();
        LOG_INFO("Partition for symbol {} destroyed.",
                 common::to_string(mSymbol));
    }

    void Partition::start()
    {
        if (mWorker) {
            LOG_WARN("Partition for symbol {} already running. Skipping start.",
                     common::to_string(mSymbol));
            return;
        }
        mWorker = std::make_unique<WorkerThread>(
                mCommandQueue, *mOrderManager, *mOrderBook, *mMatcher,
                *mRiskManager, *mOrderRepository, mTradeIDGenerator,
                mDatabaseWorker);
        mWorker->start();
        LOG_INFO("Worker thread started for partition {}.",
                 common::to_string(mSymbol));
    }

    void Partition::stop()
    {
        if (mWorker) {
            LOG_INFO("Stopping partition for symbol {}",
                     common::to_string(mSymbol));
            mWorker->stop();
            mWorker.reset();
        }
    }

    void Partition::stopAcceptingCommands()
    {
        mAcceptingCommands.store(false, std::memory_order_relaxed);
    }

    void Partition::enqueue(std::unique_ptr<Command> command)
    {
        if (!mAcceptingCommands.load(std::memory_order_relaxed)) {
            LOG_WARN("Partition {} is shutting down, rejecting new command.",
                     common::to_string(mSymbol));
            return;
        }
        if (!mCommandQueue.try_push(std::move(command))) {
            LOG_ERROR(errors::ETRADE11, "Partition {} command queue full.",
                      common::to_string(mSymbol));
        }
    }

    void Partition::waitReady()
    {
        mReadyFuture.get();
    }

    size_t Partition::getQueueSize() const
    {
        return mCommandQueue.size();
    }

    common::Symbol Partition::getSymbol() const
    {
        return mSymbol;
    }

    const data::DatabaseWorker* Partition::getDatabaseWorker() const
    {
        return mDatabaseWorker;
    }

    const TradeIDGenerator* Partition::getTradeIDGenerator() const
    {
        return mTradeIDGenerator;
    }

    const OrderManager* Partition::getOrderManager() const
    {
        return mOrderManager.get();
    }

    const OrderBook* Partition::getOrderBook() const
    {
        return mOrderBook.get();
    }

    const Matcher* Partition::getMatcher() const
    {
        return mMatcher.get();
    }

    const RiskManager* Partition::getRiskManager() const
    {
        return mRiskManager.get();
    }

    const data::OrderRepository* Partition::getOrderRepository() const
    {
        return mOrderRepository.get();
    }

    const WorkerThread* Partition::getWorker() const
    {
        return mWorker.get();
    }
} // namespace trading_core
