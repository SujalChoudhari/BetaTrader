#include "trading_core/TradingCore.h"
#include "trading_core/NewOrder.h"
#include "trading_core/OrderIDGenerator.h"
#include "common/Order.h"
#include "logging/Logger.h"
#include <iostream>
#include <chrono>
#include <memory>
#include <vector>
#include <thread>
#include <random>

using namespace trading_core;
using namespace common;
using namespace std::chrono;

// Helper to create a new order with some variation
std::shared_ptr<Order> create_order() {
    // Use a random device and generator for better randomness
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> instrument_dist(0, static_cast<int>(Instrument::COUNT) - 1);
    static std::uniform_real_distribution<> price_dist(0.0, 0.01); // Small price variation

    Instrument random_instrument = static_cast<Instrument>(instrument_dist(gen));

    return std::make_shared<Order>(
        OrderIDGenerator::nextId(),
        random_instrument,
        "STRESS_CLIENT_" + std::to_string(OrderIDGenerator::getId()), // Unique client ID per order for logging
        (OrderIDGenerator::getId() % 2 == 0) ? OrderSide::Buy : OrderSide::Sell, // Alternate sides
        OrderType::Limit,
        100.0,
        1.2000 + price_dist(gen), // Add some price variation
        system_clock::now()
    );
}

int main() {
    // Initialize logger
    logging::Logger::Init("stress_test", "logs/stress_test.log");

    // Create and start the trading core
    std::cout << "Initializing Trading Core..." << std::endl;
    auto tradingCore = std::make_unique<TradingCore>();
    tradingCore->start();
    std::cout << "Trading Core started." << std::endl;

    // --- Stress Test Parameters ---
    const int num_orders_to_submit = 100;


    std::vector<std::thread> submission_threads;

    // Start the timer
    auto start_time = high_resolution_clock::now();

    for (int j = 0; j < num_orders_to_submit; ++j) {
        auto order = create_order();
        std::this_thread::sleep_for(50ms);
        auto newOrderCmd = std::make_unique<NewOrder>(
            order->getClientId(),
            order->getTimestamp(),
            order
        );
        tradingCore->submitCommand(std::move(newOrderCmd));
        std::this_thread::yield(); // Pace the submission to be more realistic
    }

    // Wait for all submission threads to complete
    for (auto &t: submission_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Stop the timer
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    std::cout << "All orders have been submitted." << std::endl;

    // This sleep is important to allow the worker threads in the partitions
    // to process the enqueued commands before we shut down.
    // In a real scenario, you'd want a more sophisticated way to know when processing is done.
    std::cout << "Waiting for 10 seconds to allow orders to be processed..." << std::endl;
    std::this_thread::sleep_for(1s);

    // Stop the trading core
    std::cout << "Stopping Trading Core..." << std::endl;
    tradingCore->stop();
    std::cout << "Trading Core stopped." << std::endl;

    // --- Report Results ---
    double duration_sec = duration.count() / 1000.0;
    double orders_per_sec = num_orders_to_submit / duration_sec;

    std::cout << "\n--- Stress Test Results ---" << std::endl;
    std::cout << "Total Orders Submitted: " << num_orders_to_submit << std::endl;
    std::cout << "Total Submission Time: " << duration.count() << " ms" << std::endl;
    std::cout << "Submission Rate: " << static_cast<int>(orders_per_sec) << " orders/sec" << std::endl;
    std::cout << "---------------------------\n" << std::endl;

    return 0;
}
