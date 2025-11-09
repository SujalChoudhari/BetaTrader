Welcome, This is BetaTrader, a hands-on learning project and a C++ blueprint that explores how a small,
exchange-like FX trading engine is put together.

What this repository is: a deliberate, step-by-step engineering exercise. It contains the in-memory matching core,
a compact persistence layer, shared types and logging utilities, and a suite of unit tests. The goal is to learn
by building: the system is being constructed incrementally, this is a mammoth project done one piece at a time.

Who this is for: developers, engineers, and curious traders who want a readable, runnable codebase to study
matching semantics, order lifecycle, risk checks, and toolchain practices.

Status: the `common/`, `core/trading_core/` and `core/data/` modules are implemented and covered by unit tests.
Other layers (gateway adapters, venue simulators, business services) are intentionally left as integration points for
future work and experiments.

## Quick summary of what's implemented

- `common/` — shared types (`Order`, `Trade`, `Instrument`, `Types`) and logging helpers (`logging/Logger.h`, runbook macros).
- `core/trading_core/` — in-memory matching engine, `OrderBook`, `Matcher`, `OrderManager`, `Partition` and `WorkerThread`.
- `core/data/` — light persistence layer with `DatabaseWorker` (async SQLite task runner) and repositories: `OrderRepository`, `TradeRepository`, `TradeIDRepository`.
- `vendor/` — included third-party libraries used by the project: `googletest`, `spdlog`, `SPSCQueue`, `SQLiteCpp`.
- `tests/` — unit tests covering the trading core and data repositories (GoogleTest).

Short note for traders and researchers:

- This is a learning and experimentation project, not a production trading system.
- That said, the matching engine and order lifecycle implement realistic exchange-like semantics (price-time priority,
  partial fills, execution reporting) so you can reproduce, test, and reason about execution behavior in a controlled,
  reproducible environment.

If you want to experiment, this repo is a good sandbox, but always treat results as academic/simulated until hardened and
audited for production use.

## Repository layout (important parts)

```
BetaTrader/
├── common/                         Shared types & logging (Order, Trade, Instrument)
├── core/
│   ├── data/                       Persistence layer (SQLite adapter, repositories)
│   └── trading_core/                Matching engine, order manager, worker threads, tests
├── vendor/                          Bundled third-party libs (gtest, spdlog, SQLiteCpp, SPSCQueue)
├── CMakeLists.txt
└── README.md
```

## Build & test (quick)

These steps build the project and run unit tests. They are the minimum required to validate the code locally.

```bash
# from repository root
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
ctest --output-on-failure
```

Notes:
- The repo targets Linux and typical developer toolchains. Some vendored libraries are small copies used for tests.
- If CMake cannot find system dependencies, install the required -dev packages (e.g., `libsqlite3-dev`, `libspdlog-dev`) or adjust `CMAKE_PREFIX_PATH`.

## How to run a small smoke test (developer)

There is no single production binary yet. The tests exercise most of the behavior. To try quick interactions:

1. Build and run unit tests (see above).
2. Open `core/trading_core/tests/TradingCoreTests.cpp` and examine example flows — tests show how `NewOrder`, `ModifyOrder`, and `CancelOrder` are processed.

## Documentation & design (TSDs)

Each submodule contains a `README.md` and `TSD.md` describing architecture and current scope. Start with:

- `core/trading_core/TSD.md` — explains partitioning, matching rules, and threading model.
- `core/data/TSD.md` — explains the persistence choices and schema.

## Notes for forex traders (domain primer)

If you're a trader: this repo models a small exchange-like matching engine. Important concepts implemented:

- Instruments: a small enum of currency pairs lives in `common/include/common/Instrument.h` (e.g., `EURUSD`, `USDJPY`).
- Order types: `Limit` and `Market` orders (`common/include/common/Types.h`).
- Execution rule: price-time priority; execution price is the resting order price.
- Partial fills are supported; trades are emitted as `common::Trade` objects and persisted by the data layer.

If you want to experiment:

1. Write unit tests that create `common::Order` instances and push `NewOrder` commands into a `Partition`.
2. Observe outcomes from `Matcher` and `ExecutionPublisher` (tests/mocks show how to inspect executions).

## Contributing & roadmap

The repository is organized to accept contributions. Suggested near-term roadmap items:

- Add a simple FIX/REST gateway example to inject orders.
- Build a small venue simulator that emits market data and liquidity for integration tests.
- Add a lightweight CLI to start a single-partition TradingCore for manual experimentation.
- Improve persistence adapters (Postgres/Timescale-backed) for real historical storage.

If you're planning to publish, please ensure unit tests pass and consider adding a CONTRIBUTING.md with PR guidelines.

## License
GPL-3.0 — See `LICENSE` for full terms.


