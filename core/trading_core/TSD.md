# Trading Core — Technical Specification & Design (TSD)

This document describes the implemented design and practical details of the `core/trading_core` module. It is
meant for developers and traders who want to understand or extend the matching engine, partitioning model, and
hot-path invariants.

## 1 — High-level design patterns

- Partitioned single-writer: one worker thread owns a partition (usually one instrument) and is the only thread
	that mutates the order book and order manager. This eliminates locks on the critical path.
- Command pattern: external inputs are normalized into `Command` objects (`NewOrder`, `ModifyOrder`, `CancelOrder`) and
	enqueued to partition queues.
- Event-driven and asynchronous IO: matching and state mutation are synchronous inside the worker; persistence and
	outbound publishing are performed asynchronously to avoid blocking.
- Test-first and deterministic: the worker exposes deterministic helper methods to drive single steps in unit tests.

## 2 — Class responsibilities (concise)

- TradingCore: boots partitions and shared services (optional owned `DatabaseWorker`, `TradeIDGenerator`) and provides a
	top-level submit API.
- Partition: composes `OrderBook`, `OrderManager`, `Matcher`, `RiskManager`, `WorkerThread`, and repositories. It owns the
	command queue and worker lifecycle.
- WorkerThread: dequeues batches of `Command` objects and runs the deterministic processing pipeline: pre-check → insert/modify/cancel → match → publish → persist.
- OrderManager: owns `common::Order` instances and provides constant-time lookup for cancels/modifies.
- OrderBook: maintains price-level maps (bids sorted descending, asks ascending) with deque-based price levels for FIFO within a price.
- Matcher: performs matching logic and returns `common::Trade` objects for each match.
- RiskManager: pluggable, lightweight pre-checks and post-trade hooks.

## 3 — Threading & concurrency model

1. Producers (gateway or test code) create `Command` objects and push into a partition's SPSC queue.
2. The partition's single `WorkerThread` pops commands in batches and performs all state mutation.
3. Persistence and outbound publish happen via async adapters (e.g., `data::DatabaseWorker`) or lock-free queues consumed by IO workers.

Consequences:

- No locks on the hot matching path.
- Deterministic processing order when commands are timestamped/ingested in consistent order.

## 4 — Internal queues & batching

- `Command` queue: `rigtorp::SPSCQueue<std::unique_ptr<Command>>` with a configurable batch size (default: 64).
- Worker reads up to a batch or until empty, processes commands sequentially to amortize cost.
- Outbound events (trades, exec reports) are aggregated and handed off to IO workers in batches to reduce syscalls.

## 5 — Matching rules & determinism

- Price-time priority: better price first; for identical prices, earlier timestamp first.
- Execution price is the resting order price (common exchange convention).
- Partial fills: allowed — remaining quantity is updated and the partially filled order stays in the book.
- Determinism: timestamps should be assigned once upon ingestion (gateway) to avoid variations between runs. Tests should
	use deterministic timestamps to make results reproducible.

## 6 — Serialization & publishing

- The repo includes an `ExecutionPublisher` helper used to format Execution Reports and Trades. In production you would
	serialize using FlatBuffers (or another zero-copy format) and publish via ZeroMQ or another messaging layer.
- The core only constructs `common::Trade` and `common::Order` objects; serialization is handled downstream by IO workers.

## 7 — Persistence and recovery notes

- `data::DatabaseWorker` accepts lambdas and executes them on a background thread against a `SQLite::Database` instance.
	This keeps disk I/O off the matching thread.
- `TradeIDRepository` provides a persistent backing for trade id counters used by the `TradeIDGenerator`.
- There is not yet a complete snapshot/restore orchestration in `trading_core` — persistence hooks exist for replay.

## 8 — Performance & engineering recommendations

- Avoid heap allocations on the hot path; prefer pre-allocated pools for `Order` entries if needed.
- Minimize virtual calls in inner loops (the `Matcher` is designed to be concrete and inline-friendly).
- Tune batch size and queue capacities based on observed throughput and latency.

## 9 — How to experiment / run small scenarios

1. Build the tests as described in the top-level README.
2. Use unit tests to examine small deterministic scenarios (see `core/trading_core/tests/`). Tests show how to build `NewOrder` commands and observe produced trades.
3. For a manual experiment: create a small `main()` that constructs a `Partition`, enqueues `NewOrder` commands, calls `start()` and optionally uses test helper methods to step the worker.

## 10 — Example processing sequence (concise)

1. Gateway produces `NewOrder` for EURUSD and enqueues it to Partition A.
2. WorkerThread dequeues the command and calls `RiskManager::preCheck`.
3. If accepted, `OrderManager::addOrder` stores the `Order` and `OrderBook::insertOrder` places a pointer at the correct price level.
4. `Matcher::match` compares incoming order vs resting book; for each match it creates a `common::Trade` object.
5. Worker updates order states (remaining qty/status), calls `ExecutionPublisher` to enqueue messages for IO, and submits persistence lambdas to `data::DatabaseWorker`.

---

This TSD is intentionally practical: it focuses on the currently implemented architecture and the points you need to know to run
tests, extend matching logic, or experiment with alternative rules. If you'd like, I can now add a small interactive example
program (CLI) that runs a single partition and accepts JSON orders on stdin for manual experimentation.


