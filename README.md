# BetaTrader | High-Performance FX Trading Engine {#mainpage}

Welcome to BetaTrader, a project and a C++ blueprint that explores how a small, exchange-like FX trading engine is put together.

This repository is a deliberate, step-by-step engineering exercise. It contains an in-memory matching core, a compact persistence layer, shared types, logging utilities, and a comprehensive suite of unit tests. The goal is to learn by building: the system is being constructed incrementally, one piece at a time.

This project is for developers, engineers, and curious traders who want a readable, runnable codebase to study matching semantics, order lifecycle, risk checks, and modern C++ development practices.

**Status**: The `common`, `core/trading_core`, `core/data`, and `core/fix` modules are all implemented and rigorously covered by unit tests. The system now features a robust lock-free matching core, a decoupled asynchronous SQLite persistence layer, and a functional FIX gateway handling Logon, Heartbeats, sequence recovery, and order routing.

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
| `client/` | Frontend suite including a Trader UI and a Load Simulator. | `client_fix`, `client_ui`, `client_app`, `client_simulator` |
| `vendor/` | Third-party libraries used for testing, logging, and data storage. | `googletest`, `spdlog`, `SPSCQueue`, `SQLiteCpp` |

## Repository Layout

```
BetaTrader/
├── CMakeLists.txt
├── CMakePresets.json   # Build & Coverage Presets
├── README.md
├── client/             # Frontend UI & Simulator
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

### FIX Gateway
*   [FIX Gateway Overview](core/fix/README.md): Networking and session management details.
*   [FIX Protocol Reference Guide](core/fix/FIX.md): Quick reference for tag-value pairs.

### Client Application (In Progress)
*   [Client Overview](client/README.md): Trader terminal and simulator details.

### Data Persistence
*   [Data Persistence Overview](core/data/README.md): Repository and background worker details.

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

## Roadmap & Future Scope

This project is a continuous engineering exercise. With the solid foundation of a functional matching core, asynchronous persistence, and a robust FIX gateway now established, here are the most logical next steps for future expansion:

### 1. Build a FIX Client App / Simulator (In Progress)
Develop a standalone client application (`client`) that provides a high-performance Trader UI (using ImGui) and a headless Load Simulator. This allows for both manual trading and system benchmarking at scale.

### 2. A REST / WebSocket API Gateway
While FIX is ideal for high-performance institutional trading, REST and WebSockets are the standard for retail platforms and web UIs. Building a secondary HTTP/WS gateway alongside the `FixServer` that translates JSON requests into `trading_core::Command` objects would instantly open the door to building a frontend interface (like React).

### 3. Market Data Simulator (Feed Handler)
An exchange is more dynamic with an active order book. Building a background service or a mock liquidity provider that connects to the engine and continuously publishes random or historically-replayed limit orders and market data updates would populate the books and simulate a live market environment.

### 4. Advanced Risk Management Implementation
Currently, the `RiskManager` is a foundation awaiting extension. Implementing real pre-trade risk checks such as tracking a client's net open positions, calculating available margin, or implementing "fat finger" checks (e.g., rejecting orders drastically away from the last traded price) would significantly mature the system.

If you plan to contribute to any of these areas, please ensure all unit tests pass and consider updating the relevant `.md` documentation files.

## License

This project is licensed under the GPL-3.0 License. See the `LICENSE` file for details.
