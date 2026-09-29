# BetaTrader | High-Performance FX Trading Engine {#mainpage}

Welcome to BetaTrader, a project and a C++ blueprint that explores how a small, exchange-like FX trading engine is put together.

This repository is a deliberate, step-by-step engineering exercise. It contains an in-memory matching core, a compact persistence layer, shared types, logging utilities, and a comprehensive suite of unit tests. The goal is to learn by building: the system is being constructed incrementally, one piece at a time.

This project is for developers, engineers, and curious traders who want a readable, runnable codebase to study matching semantics, order lifecycle, risk checks, and modern C++ development practices.

**Status**: The `common`, `exchange` suite (matching, persistence, fix gateway), and `client_fix` modules are all implemented and rigorously covered by unit tests. The system features a robust lock-free matching core, a decoupled asynchronous SQLite persistence layer, a functional FIX gateway with thread-safe CompID-based session management and persistent sequence numbers, and a working Dear ImGui client application with an embedded local exchange.

## Project Goals

*   **Learn by Building**: Provide a practical, open-source example of a trading system's core components.
*   **Readability and Simplicity**: Prioritize clear, modern C++ code over premature optimization or overly complex designs.
*   **Test-Driven Development**: Emphasize a strong testing culture with extensive unit tests for core logic.
*   **Extensibility**: Design a modular architecture that allows for future expansion with new features, such as different gateways or persistence backends.

| Module | Description | Key Components |
| :--- | :--- | :--- |
| `common/` | Shared data structures, types, and utilities used across the project. | `Order`, `Trade`, `Instrument`, `Logger`, `Runbook` |
| `exchange/` | The core trading engine suite (Modular). | `exchange_app`, `exchange_matching`, `exchange_fix`, `exchange_persistence` |
| `client/` | Frontend suite including a Trader UI and a Load Simulator. | `client_fix`, `client_ui`, `client_admin`, `client_app`, `client_simulator` |
| `vendor/` | Third-party libraries used for testing, logging, data storage, and UI. | `googletest`, `spdlog`, `SPSCQueue`, `SQLiteCpp`, `GLFW`, `ImGui`, `ImPlot` |

## Repository Layout

```
BetaTrader/
├── CMakeLists.txt
├── CMakePresets.json   # Build & Coverage Presets
├── README.md
├── client/             # Frontend UI & Simulator
├── common/             # Shared types & logging
├── exchange/           # Modular Exchange Backend
│   ├── exchange_app/           # Orchestration
│   ├── exchange_fix/           # FIX Gateway
│   ├── exchange_matching/      # Matching Engine
│   ├── exchange_persistence/   # Persistence layer
│   └── ...
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

### Generated API Documentation

The API and architecture site is generated from the tracked `Doxyfile`,
`docs/doxygen-mainpage.md`, module Markdown files, and the existing Doxygen
header/footer assets.
Generated HTML is intentionally ignored from Git:

```bash
sudo apt-get install doxygen graphviz   # Ubuntu/Debian, if not installed
doxygen Doxyfile
xdg-open docs/html/index.html              # or use a local file browser
```

The generated landing page contains the repository map, local-demo data-flow
diagram, native-terminal runbook, and links into the module documentation.

## How to Explore the Code

1.  **Build and run the unit tests** as described above.
2.  **Explore the tests**:
    *   `FixEndToEndTests.cpp`: Shows the full FIX lifecycle (Logon, Order, MD, Logout).
    *   `TradingCoreTests.cpp`: Detailed tests for engine-level partitioning and command processing.
    *   `MatcherTests.cpp`: Price-time priority matching logic details.
    *   `AuthRepositoryTests.cpp`: Verification of database-backed client authentication.

## System Components

BetaTrader is divided into several high-level components. Each component contains its own overview and technical specification:

### Exchange Infrastructure
*   [Exchange Overview](./exchange/README.md): Central hub for the modular exchange backend.
*   [Matching & Logic](./exchange/exchange_matching/README.md): High-performance orderbook and matcher details.
*   [Data Persistence](./exchange/exchange_persistence/README.md): Asynchronous SQLite repository details.

### FIX Gateway & Connectivity
*   [FIX Gateway Overview](./exchange/exchange_fix/README.md): Networking and session management details.
*   [FIX Protocol Reference Guide](./exchange/exchange_fix/FIX.md): Quick reference for tag-value pairs.

### Client Application
*   [Client Overview](./client/README.md): Trader terminal and simulator details.
*   [Client UI visual guide](./docs/client-ui/README.md): Verified default cockpit, every panel, and order-book-focused layouts.

#### Client terminal screenshots

![Live-data BetaTrader cockpit](./docs/client-ui/screenshots/cockpit-default.png)

The default cockpit keeps the chart central, the order-book workflow on the right, and FIX/blotter/operations panels in dedicated bottom dock groups. This primary frame is data-backed: it starts with about an hour of simulated EURUSD candle history, continues with live L2 depth, active FIX market-data events, and a running local simulator. The focused panel captures and alternate layouts are available in the [client UI visual guide](./docs/client-ui/README.md).

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

### 1. Client Terminal & Simulator (Implemented)
The Dear ImGui client application (`client_app`) now provides a deterministic multi-panel trading cockpit with an embedded local exchange, FIX connection/session management, L2 order-book visualization, guarded order entry, charting, open-orders and execution-history blotters, exchange administration, and a stochastic simulator. See the [client UI visual guide](./docs/client-ui/README.md) for the verified layouts and screenshots.

### 2. A REST / WebSocket API Gateway
While FIX is ideal for high-performance institutional trading, REST and WebSockets are the standard for retail platforms and web UIs. Building a secondary HTTP/WS gateway alongside the `FixServer` that translates JSON requests into `trading_core::Command` objects would instantly open the door to building a frontend interface (like React).

### 3. Historical Replay and Market-Data Workflows
The embedded simulator provides local order flow for development. A future replay/feed-handler service could add deterministic historical sessions and richer market-data scenarios for the chart and order book.

### 4. Advanced Risk Management Implementation
Currently, the `RiskManager` is a foundation awaiting extension. Implementing real pre-trade risk checks such as tracking a client's net open positions, calculating available margin, or implementing "fat finger" checks (e.g., rejecting orders drastically away from the last traded price) would significantly mature the system.

If you plan to contribute to any of these areas, please ensure all unit tests pass and consider updating the relevant `.md` documentation files.

## License

This project is licensed under the GPL-3.0 License. See the `LICENSE` file for details.

### Measured In-Process Benchmark (2026-09-29)

The following is a dated, synthetic measurement of the in-process
`TradingCore` matching path. The benchmark receipt targeted `origin/main` at
commit `9ae29db5462c55d56115d40e866d6ca6a0e0db03`.

The workload used four fresh 1,000,000-order runs with paired opposite IOC
orders across all nine instruments, unique sender IDs, and logging I/O
disabled. `completed_orders_per_second` counts 1,000,000 `NEW` execution
reports after queue drain and worker shutdown. This is an in-process matching-
core measurement, not a FIX/TCP/network measurement or a production-server
capacity claim.

| Completed-order throughput | Orders/s |
| :--- | ---: |
| Run 1 | 67,781 |
| Run 2 | 69,455 |
| Run 3 | 76,356 |
| Run 4 | 73,657 |
| Mean | 71,812 |
| Median | 71,556 |
| Best observed | 76,356 |

Additional measured results:

- Matched trades: 500,000 per run; mean 35,906 trades/s.
- Execution-report latency p50: 2.53–2.89 ms across runs; median 2.76 ms.
- Execution-report latency p95: 14.4–151.8 ms across runs; median 43.0 ms.
- Execution-report latency p99: 30.6–194.2 ms across runs; median 58.3 ms.
- Peak RSS: 564.3–572.4 MiB; mean 568.9 MiB.
- Process CPU time: 25.9–28.8 s per 1M-order run.
- Enqueue/submission rate: 347k–547k orders/s; this is not completed throughput.

The host safety envelope was 2 logical CPUs, 7.8 GiB RAM, and no swap.
Benchmark processes ran at nice level 10 with a 4 GiB virtual-memory limit, a
3 GiB RSS watchdog, a 30 s CPU limit, and a 45 s wall timeout; no guard
tripped. Therefore, 50k orders/s is below this measured in-process result,
but this does not establish 50k FIX/network requests/s or production capacity.
The latency tail and host limits above are evidence, not a guaranteed capacity
or SLA.

The checked-in stress driver was not the harness used for these figures. Its
independently discovered logger-lifetime issue was tracked in [#27](https://github.com/SujalChoudhari/BetaTrader/issues/27);
these figures do not claim that the existing stress driver is fixed.
