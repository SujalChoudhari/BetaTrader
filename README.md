# BetaTrader C++ FX Trading Engine Blueprint

Welcome to BetaTrader, a hands-on learning project and a C++ blueprint that explores how a small, exchange-like FX trading engine is put together.

This repository is a deliberate, step-by-step engineering exercise. It contains an in-memory matching core, a compact persistence layer, shared types, logging utilities, and a comprehensive suite of unit tests. The goal is to learn by building: the system is being constructed incrementally, one piece at a time.

This project is for developers, engineers, and curious traders who want a readable, runnable codebase to study matching semantics, order lifecycle, risk checks, and modern C++ development practices.

**Status**: The `common`, `core/trading_core`, and `core/data` modules are implemented and covered by unit tests. Other layers (gateway adapters, venue simulators, business services) are intentionally left as integration points for future work and experiments.

## Project Goals

*   **Learn by Building**: Provide a practical, open-source example of a trading system's core components.
*   **Readability and Simplicity**: Prioritize clear, modern C++ code over premature optimization or overly complex designs.
*   **Test-Driven Development**: Emphasize a strong testing culture with extensive unit tests for core logic.
*   **Extensibility**: Design a modular architecture that allows for future expansion with new features, such as different gateways or persistence backends.

| Module | Description | Key Components |
| :--- | :--- | :--- |
| `common/` | Shared data structures, types, and utilities used across the project. | `Order`, `Trade`, `Instrument`, `Logger`, `Runbook` |
| `core/trading_core/` | The heart of the system: an in-memory, partitioned matching engine. | `Partition`, `WorkerThread`, `OrderBook`, `Matcher`, `OrderManager`, `ExecutionPublisher` |
| `core/data/` | A lightweight persistence layer for storing trades, orders, and sequences. | `DatabaseWorker`, `OrderRepository`, `TradeRepository`, `AuthRepository`, `SequenceRepository` |
| `core/fix/` | FIX Gateway for client connectivity and session management. | `FixServer`, `FixSession`, `FixSessionManager`, `OutboundMessageBuilder` |
| `vendor/` | Third-party libraries used for testing, logging, and data storage. | `googletest`, `spdlog`, `SPSCQueue`, `SQLiteCpp` |

## Repository Layout

```
BetaTrader/
├── CMakeLists.txt
├── CMakePresets.json   # Build & Coverage Presets
├── README.md
├── common/             # Shared types & logging
├── core/
│   ├── data/           # Persistence (SQLite, AuthRepository)
│   ├── fix/            # FIX Gateway (Session Management, Converters)
│   └── trading_core/   # Matching Engine, Partitioning
├── tools/              # Helper tools (coverage_reporter.py)
└── vendor/             # Bundled third-party libs
```

## Getting Started: Build and Test

These steps will build the project and run all unit tests.

### Standard Build
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
ctest --output-on-failure
```

### Coverage Build
```bash
# Using CMake Presets
cmake --preset coverage
cmake --build build/coverage -j$(nproc)
# Generate and run coverage report
cd build/coverage && make coverage
# View summary
python3 ../../tools/coverage_reporter.py
```

## How to Explore the Code

1.  **Build and run the unit tests** as described above.
2.  **Explore the tests**:
    *   `FixEndToEndTests.cpp`: Shows the full FIX lifecycle (Logon, Order, MD, Logout).
    *   `TradingCoreTests.cpp`: Detailed tests for engine-level partitioning and command processing.
    *   `MatcherTests.cpp`: Price-time priority matching logic details.
    *   `AuthRepositoryTests.cpp`: Verification of database-backed client authentication.

## System Components

BetaTrader is divided into several high-level components. Each component contains its own overview and technical specification:

### Trading Core
*   [Trading Core Overview](core/trading_core/README.md): Matching engine and order management details.
*   [Technical Specification](core/trading_core/TSD.md): Detailed architectural breakdown and threading model.

### FIX Gateway
*   [FIX Gateway Overview](core/fix/README.md): Networking and session management details.
*   [Technical Specification](core/fix/TSD.md): Protocol implementation and gateway architecture.
*   [FIX Protocol Reference Guide](core/fix/FIX.md): Quick reference for tag-value pairs.

### Data Persistence
*   [Data Persistence Overview](core/data/README.md): Repository and background worker details.
*   [Technical Specification](core/data/TSD.md): Schema design and persistence mechanics.

## Forex Trading Domain Concepts

This repository models a small, exchange-like matching engine. Here are the key concepts implemented:

*   **Instruments**: A simple `enum` of currency pairs is defined in `common/include/common/Instrument.h` (e.g., `EURUSD`, `USDJPY`).
*   **Order Types**: `Limit` and `Market` orders are supported (`common/include/common/Types.h`).
*   **Matching Algorithm**: The matching engine uses a standard **price-time priority** algorithm. The execution price is the price of the resting order on the book.
*   **Fills**: Partial fills are supported. Each fill generates a `common::Trade` object, which is then published and persisted.
*   **Execution Monitoring**: Real-time trade executions and order rejections are formatted and dumped directly to standard output, making it easy to track engine activity.

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
