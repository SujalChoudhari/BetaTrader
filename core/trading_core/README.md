# core/trading_core — module README (what's implemented and how to use)

This folder contains the in-memory trading core: the matching engine, order manager, per-instrument partitions, and tests.

Implemented components (what you can find and use today):

- `OrderBook` — keeps bid and ask books organized by price levels.
- `OrderManager` — stores and indexes live orders for fast lookup and lifecycle operations.
- `Matcher` — matching engine producing `common::Trade` objects following price-time priority.
- `Partition` and `WorkerThread` — partitioning and single-writer worker per partition, fed by SPSC queues.
- `OrderIDGenerator` and `TradeIDGenerator` — id generation helpers (trade id is persisted via `core/data`).
- `ExecutionPublisher` — helper to publish execution reports and trades (static helper used by tests and code paths).
- `RiskManager` — pluggable pre-check/post-trade hooks (lightweight implementation included).
- Unit tests for matcher, order book, trade id generator, risk manager, etc., in `core/trading_core/tests/`.

Why this is useful for experimentation

This module provides a compact, readable implementation of a core exchange-like system and is an ideal place to test
matching rules, latency-sensitive design decisions, and risk checks. The single-writer partition model makes reasoning
about correctness and determinism straightforward.

Getting started (developer)

1. Build and run tests (see top-level README build steps).
2. Run a focused test binary (examples):

```bash
# after building
./build/core/trading_core/tests/OrderBookTests
./build/core/trading_core/tests/MatcherTests
```

3. Browse tests in `core/trading_core/tests/` to see example patterns for creating orders and observing matches.

Key conventions and behaviors

- Matching: price-time priority. Execution price is the resting order’s price.
- Order objects are owned by `OrderManager` — matching and repositories use raw pointers into owned objects.
- Hot path design: prefer references, avoid blocking I/O in `WorkerThread` (I/O/persistence is delegated to `data::DatabaseWorker`).

How to wire up a simple experiment

1. Create a `Partition` in unit test or a tiny main program.
2. Push `NewOrder` commands into the partition's queue with different timestamps/prices/side combinations.
3. Let the `WorkerThread` process the queue (in tests you can call `processNextCommand()` for deterministic single-step behavior).

Where to look in code

- `include/trading_core/OrderBook.h` — order book interface and types.
- `include/trading_core/Matcher.h` — matching behavior.
- `include/trading_core/WorkerThread.h` — command processing and batching.
- `tests/` — many small unit tests showing usages and expected outputs.

Notes for forex traders (practical takeaways)

- The engine implements typical exchange semantics; you can test how limit orders interact with resting liquidity.
- The instrument set is small and defined in `common/include/common/Instrument.h` — add new instruments there and extend tests.

Limitations & TODO (current)

- No live gateway or FIX interface yet — the core is exercised via tests and programmatic APIs.
- No full state snapshot/restore orchestration is implemented; persistence hooks exist via `data/`.
- No production monitoring or metrics exporter in this snapshot.

If you want me to, I can add a small example program (CLI) that starts a single partition and accepts simple JSON orders on stdin for manual experimentation.

