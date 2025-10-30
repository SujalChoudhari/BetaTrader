//
// Created by sujal on 30-10-2025.
//

#include <gtest/gtest.h>
#include "trading_core/Partition.h"
#include <chrono>
#include "common/Order.h"
#include "trading_core/Command.h"
#include "trading_core/NewOrder.h"

class DummyOrder : public common::Order {
public:
    DummyOrder()
        : Order(1,
                common::Instrument::USDCAD,
                "C1",
                common::OrderSide::Buy,
                common::OrderType::Limit,
                100,
                10.0,
                std::chrono::system_clock::now()) {
    }
};


using namespace trading_core;

class PartitionTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_shared<data::DatabaseWorker>(":memory:");
        tradeGen = std::make_shared<TradeIDGenerator>(db);
        execPub = std::make_shared<ExecutionPublisher>();
    }

    void TearDown() override {
        // Explicit cleanup in reverse order
        execPub.reset();
        tradeGen.reset();
        db.reset();

        // Give threads time to fully exit and cleanup
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));
    }

    data::DatabaseWorkerPtr db;
    std::shared_ptr<TradeIDGenerator> tradeGen;
    std::shared_ptr<ExecutionPublisher> execPub;
};

TEST_F(PartitionTestFixture, StartStopNoCrash) {
    {
        Partition partition(common::Instrument::USDCAD, db, tradeGen, execPub);
        partition.start();
        // Wait for worker thread to initialize
        std::this_thread::sleep_for(std::chrono_literals::operator ""s(1));
    }
    // Partition destructor is called here
    SUCCEED();
}

TEST_F(PartitionTestFixture, EnqueueAndDrop) {
    Partition partition(common::Instrument::USDCAD, db, tradeGen, execPub);
    partition.start();

    // Wait for worker thread to initialize
    std::this_thread::sleep_for(std::chrono_literals::operator ""s(1));

    // Fill the queue
    for (int i = 0; i < 4096; ++i) {
        partition.enqueue(std::make_unique<NewOrder>("C1", std::chrono::system_clock::now(), std::make_shared<DummyOrder>()));
    }

    // This should drop the command (queue full)
    EXPECT_NO_THROW(partition.enqueue(std::make_unique<NewOrder>("C1", std::chrono::system_clock::now(), std::make_shared<DummyOrder>())));

    partition.stop();
}
