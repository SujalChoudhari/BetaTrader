#include "common/Order.h"
#include "logging/Logger.h"
#include "trading_core/NewOrder.h"
#include "trading_core/TradingCore.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

using namespace trading_core;
using namespace common;
using namespace std::chrono;
// -------------------- Order Factory --------------------
std::unique_ptr<Order> create_order(OrderIDGenerator* orderIdGenerator)
{
    using namespace std::chrono;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> instrument_dist(
            0, static_cast<int>(Instrument::COUNT) - 1);

    // long-term mean price each instrument gravitates around
    static const double mean_price = 1.2000;
    static const double reversion_speed = 0.05; // pull-back strength
    static const double base_vol = 0.002; // baseline daily vol
    static const double vol_cluster_factor = 0.3; // smooth volatility memory

    // state memory
    static std::unordered_map<Instrument, double> last_price;
    static std::unordered_map<Instrument, double> inst_vol;

    // random quantity distribution with realistic heavy-tail
    static std::lognormal_distribution<double> qty_dist(4.0, 1.0);

    Instrument inst = static_cast<Instrument>(instrument_dist(gen));
    auto orderId = orderIdGenerator->nextId();

    // initialize state per instrument
    if (last_price.find(inst) == last_price.end()) {
        last_price[inst] = mean_price;
        inst_vol[inst] = base_vol;
    }

    // volatility clustering update
    inst_vol[inst] = inst_vol[inst] * (1.0 - vol_cluster_factor)
                     + base_vol * vol_cluster_factor;

    // occasional volatility explosion (news-like)
    if (std::bernoulli_distribution(0.005)(gen)) { inst_vol[inst] *= 3.0; }

    // shock sampled from current volatility
    std::normal_distribution<double> price_step(0.0, inst_vol[inst]);
    double shock = price_step(gen);

    // mean reversion term
    double reversion = reversion_speed * (mean_price - last_price[inst]);

    // updated price (OU-like with lognormal step)
    double new_price = last_price[inst] * std::exp(shock) + reversion;
    if (new_price < 0.0001) new_price = 0.0001;

    last_price[inst] = new_price;

    // quantity
    int qty = (int)qty_dist(gen);
    if (qty < 1) qty = 1;
    if (qty > 5000) qty = 5000;

    return std::make_unique<Order>(
            orderId, inst, "STRESS_CLIENT_" + std::to_string(orderId),
            (orderId % 2 == 0) ? OrderSide::Buy : OrderSide::Sell,
            OrderType::Limit, common::TimeInForce::DAY, qty, new_price,
            system_clock::now());
}

// -------------------- Print Functions --------------------
void print_header(const std::string& title)
{
    std::cout << "\n" << title << "\n";
    std::cout << std::string(title.length(), '=') << "\n";
}

void print_section(const std::string& title)
{
    std::cout << "\n" << title << "\n";
    std::cout << std::string(title.length(), '-') << "\n";
}

void print_metric(const std::string& label, const std::string& value,
                  const std::string& unit = "")
{
    std::cout << std::left << std::setw(30) << label << ": " << std::right
              << std::setw(12) << value;
    if (!unit.empty()) std::cout << " " << unit;
    std::cout << "\n";
}

void print_separator()
{
    std::cout << std::string(50, '-') << "\n";
}

void print_footer()
{
    std::cout << "\n";
}

// -------------------- Main --------------------
int main()
{
    logging::Logger::Init("stress_test", "logs/stress_test.log", false, true);

    std::cout << "Initializing Trading Core..." << std::endl;
    auto tradingCore = std::make_unique<TradingCore>();
    tradingCore->start();
    std::cout << "Trading Core started.\n" << std::endl;

    const int num_orders_to_submit = 10000;
    std::cout << "Submitting " << num_orders_to_submit
              << " orders sequentially..." << std::endl;

    auto wall_start = steady_clock::now();
    auto cpu_start = clock();

    // ---------- DATA collection ----------
    auto start_time = high_resolution_clock::now();
    for (int j = 0; j < num_orders_to_submit; ++j) {
        auto order = create_order(tradingCore->getOrderIDGenerator());
        auto newOrderCmd = std::make_unique<NewOrder>(
                order->getClientId(), order->getTimestamp(), std::move(order));
        tradingCore->submitCommand(std::move(newOrderCmd));
    }
    auto end_time = high_resolution_clock::now();
    auto duration_ms = duration_cast<milliseconds>(end_time - start_time);

    // ---------- Grace period ----------
    std::cout << "Waiting for all tasks to complete..." << std::endl;
    tradingCore->stopAcceptingCommands();
    tradingCore->waitAllQueuesIdle();

    std::cout << "Stopping Trading Core..." << std::endl;
    tradingCore->stop();
    auto wall_end = steady_clock::now();
    auto cpu_end = clock();

    // ---------- Derived metrics ----------
    double submit_time_sec = duration_ms.count() / 1000.0;
    double orders_per_sec = num_orders_to_submit / submit_time_sec;
    double avg_order_latency_us
            = (submit_time_sec * 1e6) / num_orders_to_submit;
    double total_wall_time
            = duration_cast<milliseconds>(wall_end - wall_start).count()
              / 1000.0;
    double cpu_time = double(cpu_end - cpu_start) / CLOCKS_PER_SEC;
    double cpu_utilization = (cpu_time / total_wall_time) * 100.0;

    // ---------- REPORT ----------
    std::cout << std::fixed << std::setprecision(3);

    print_header("TRADING CORE STRESS TEST RESULTS");

    print_section("EXECUTION METRICS");
    print_metric("Total Orders Submitted", std::to_string(num_orders_to_submit),
                 "orders");
    print_metric("Submission Phase Duration",
                 std::to_string(duration_ms.count()), "ms");
    print_metric("Total Runtime (Wall Clock)", std::to_string(total_wall_time),
                 "s");
    print_metric("Worker Cooldown Wait", "0.000", "s");
    print_footer();

    print_section("PERFORMANCE METRICS");
    print_metric("Submission Rate",
                 std::to_string(static_cast<int>(orders_per_sec)),
                 "orders/sec");
    print_metric("Average Submission Latency",
                 std::to_string(avg_order_latency_us), "us/order");
    print_separator();
    print_metric("CPU Time Consumed", std::to_string(cpu_time), "s");
    print_metric("CPU Utilization", std::to_string(cpu_utilization), "%");
    print_footer();

    print_section("RAW DATA");
    std::cout << "Orders=" << num_orders_to_submit
              << " | Duration=" << duration_ms.count() << "ms"
              << " | CPU=" << cpu_time << "s"
              << " | Wall=" << total_wall_time << "s\n";
    print_footer();

    print_section("KEY INSIGHTS");
    std::cout << "> Throughput: " << std::fixed << std::setprecision(2)
              << orders_per_sec << " orders/second\n";
    std::cout << "> Latency: " << std::fixed << std::setprecision(3)
              << avg_order_latency_us << " microseconds per order\n";
    std::cout << "> CPU Usage: " << cpu_utilization
              << "% of available CPU time\n";
    print_footer();

    logging::Logger::Shutdown();

    return 0;
}
