# BetaTrader: A C++ FX Trading Engine Blueprint

Welcome to BetaTrader, a hands-on learning project and a C++ blueprint that explores how a small, exchange-like FX trading engine is put together.

This repository is a deliberate, step-by-step engineering exercise. It contains an in-memory matching core, a compact persistence layer, shared types, logging utilities, and a comprehensive suite of unit tests. The goal is to learn by building: the system is being constructed incrementally, one piece at a time.

This project is for developers, engineers, and curious traders who want a readable, runnable codebase to study matching semantics, order lifecycle, risk checks, and modern C++ development practices.

**Status**: The `common`, `core/trading_core`, and `core/data` modules are implemented and covered by unit tests. Other layers (gateway adapters, venue simulators, business services) are intentionally left as integration points for future work and experiments.

## Project Goals

*   **Learn by Building**: Provide a practical, open-source example of a trading system's core components.
*   **Readability and Simplicity**: Prioritize clear, modern C++ code over premature optimization or overly complex designs.
*   **Test-Driven Development**: Emphasize a strong testing culture with extensive unit tests for core logic.
*   **Extensibility**: Design a modular architecture that allows for future expansion with new features, such as different gateways or persistence backends.

## Implemented Modules

| Module | Description | Key Components |
| :--- | :--- | :--- |
| `common/` | Shared data structures, types, and utilities used across the project. | `Order`, `Trade`, `Instrument`, `Logger`, `Runbook` |
| `core/trading_core/` | The heart of the system: an in-memory, partitioned matching engine. | `Partition`, `WorkerThread`, `OrderBook`, `Matcher`, `OrderManager` |
| `core/data/` | A lightweight persistence layer for storing trades and orders. | `DatabaseWorker`, `OrderRepository`, `TradeRepository` |
| `vendor/` | Third-party libraries used for testing, logging, and data storage. | `googletest`, `spdlog`, `SPSCQueue`, `SQLiteCpp` |
| `tests/` | Unit tests for `trading_core` and `data` modules. | GoogleTest-based tests for all core logic. |

## Repository Layout

```
BetaTrader/
├── CMakeLists.txt
├── README.md
├── SystemDesign.md
├── common/             # Shared types & logging (Order, Trade, Instrument)
├── core/
│   ├── data/           # Persistence layer (SQLite adapter, repositories)
│   └── trading_core/   # Matching engine, order manager, worker threads, tests
└── vendor/             # Bundled third-party libs (gtest, spdlog, SQLiteCpp, SPSCQueue)
```

## Getting Started: Build and Test

These steps will build the project and run all unit tests, which is the primary way to validate the codebase.

```bash
# From the repository root
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
ctest --output-on-failure
```

**Notes:**
*   The project targets a Linux-like environment with a modern C++ compiler.
*   If CMake fails to find dependencies like SQLite3 or spdlog, install the development packages (e.g., `libsqlite3-dev`, `libspdlog-dev`) or provide their paths to CMake.

## How to Explore the Code

Since there is no single executable entry point yet, the best way to understand the system is through its tests.

1.  **Build and run the unit tests** as described above.
2.  **Explore the tests**: The test files in `core/trading_core/tests/` and `core/data/tests/` are excellent starting points.
    *   `TradingSystemTests.cpp` shows end-to-end order processing flows.
    *   `MatcherTests.cpp` demonstrates the price-time priority matching logic.
    *   `OrderRepositoryTests.cpp` shows how orders are persisted and retrieved.

## System Design and Architecture

For a deeper understanding of the architecture, refer to the technical system design (TSD) documents:

*   **[SystemDesign.md](SystemDesign.md)**: High-level overview of the entire system.
*   **[core/trading_core/TSD.md](core/trading_core/TSD.md)**: Detailed design of the matching engine, partitioning, and threading model.
*   **[core/data/TSD.md](core/data/TSD.md)**: Design of the persistence layer, database schema, and asynchronous worker.

## For Forex Traders: A Domain Primer

This repository models a small, exchange-like matching engine. Here are the key concepts implemented:

*   **Instruments**: A simple `enum` of currency pairs is defined in `common/include/common/Instrument.h` (e.g., `EURUSD`, `USDJPY`).
*   **Order Types**: `Limit` and `Market` orders are supported (`common/include/common/Types.h`).
*   **Matching Algorithm**: The matching engine uses a standard **price-time priority** algorithm. The execution price is the price of the resting order on the book.
*   **Fills**: Partial fills are supported. Each fill generates a `common::Trade` object, which is then published and persisted.

To experiment with trading logic, you can:
1.  Write a new unit test in `TradingSystemTests.cpp`.
2.  Create `common::Order` objects and wrap them in `NewOrder` commands.
3.  Push the commands into a `Partition` and use the mock `ExecutionPublisher` to inspect the resulting trades and order statuses.

## Contributing and Roadmap

This project is open for contributions. Some potential areas for future work include:

*   **Gateways**: Implement a simple FIX or REST gateway to submit orders from external clients.
*   **Market Data**: Build a simulator to feed market data into the system for more realistic testing.
*   **CLI Tool**: Create a command-line interface to interact with the trading core manually.
*   **Advanced Persistence**: Add support for a more robust database like PostgreSQL or a time-series database.

If you plan to contribute, please ensure all unit tests pass and consider updating the relevant documentation.

## License

This project is licensed under the GPL-3.0 License. See the `LICENSE` file for details.
