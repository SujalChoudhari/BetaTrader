# core/data — persistence layer (current scope)

This directory contains the small data/persistence layer used by the trading core. It focuses on
simple, reliable local persistence using SQLite and an asynchronous worker pattern.

Implemented components (what is present today):

- `data/DatabaseWorker.h/cpp` — an asynchronous task queue that executes lambdas against a `SQLite::Database` instance on a background thread.
- `data/OrderRepository.h/cpp` — convenience wrappers to persist `common::Order` objects.
- `data/TradeRepository.h/cpp` — wrapper to persist `common::Trade` objects.
- `data/TradeIDRepository.h/cpp` — simple persistent store for the current trade id counter.
- `data/Query.h` — SQL statements used by repositories.
- Unit tests in `core/data/tests/` that exercise repository behavior.

Goals and constraints:

- Keep the persistence layer simple and testable.
- Avoid blocking the matching path: writes are enqueued and performed asynchronously.
- Schema is intentionally minimal (orders, trades, trade_id).

How to run the data unit tests

From repository root:

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc) --target core_data_tests
ctest -R core_data_tests --output-on-failure
```

If the `core_data_tests` target is not present (depending on CMake configuration) run `ctest --output-on-failure` to run all tests.

How the code is organized

- Repositories accept and serialize `common::Order`/`common::Trade` and enqueue database lambdas to `DatabaseWorker::enqueue`.
- `DatabaseWorker` uses `rigtorp::SPSCQueue` and a `std::jthread` to run tasks; this keeps disk I/O off the matching threads.

Extending the data layer

- Add new repository classes (e.g., `PositionRepository`) following existing patterns and tests.
- Replace the backing DB layer by adding an adapter instead of changing repository APIs if you want to support Postgres later.

Limitations (current)

- Schema migrations are manual; there is no migration framework in this repo yet.
- The design intentionally favors clarity over feature-completeness; it's a small, readable codebase for experimentation.

