# Trading Core Module (`core/trading_core`)

This module contains the in-memory trading core, which is the heart of the BetaTrader system. It is responsible for order management, matching, and execution.

## Architecture: Partitioned Single-Writer Model

The `trading_core` is built on a **partitioned, single-writer** architecture. This design is crucial for achieving low latency and high throughput while ensuring deterministic behavior.

*   **Partitioning**: The system is divided into multiple `Partitions`, with each partition responsible for a specific set of instruments (e.g., `EURUSD`).
*   **Single Writer**: Each `Partition` has a dedicated `WorkerThread` that processes commands sequentially from a lock-free queue. This thread is the **only writer** for that partition's state (e.g., its order book), which eliminates the need for locks on the critical path.

This architecture provides the following benefits:
*   **Concurrency**: Multiple instruments can be processed in parallel across different partitions.
*   **Low Latency**: By avoiding locks in the command processing loop, the system minimizes contention and latency.
*   **Determinism**: Since each partition processes commands one at a time, the system's behavior is deterministic and easily testable.

```mermaid
graph TD
    subgraph Partition 1 (EURUSD)
        Q1[Command Queue] --> W1(WorkerThread 1)
        W1 --> OB1{OrderBook}
        W1 --> M1(Matcher)
    end

    subgraph Partition 2 (USDJPY)
        Q2[Command Queue] --> W2(WorkerThread 2)
        W2 --> OB2{OrderBook}
        W2 --> M2(Matcher)
    end

    ClientA[Client A] --> Q1
    ClientB[Client B] --> Q2
```

## Key Components

| Component | Description | Responsibilities |
| :--- | :--- | :--- |
| **`Partition`** | A logical unit of processing for a set of instruments. | Owns the `WorkerThread` and the command queue. |
| **`WorkerThread`** | A dedicated thread for a single partition. | Dequeues and processes commands; orchestrates all state changes. |
| **`OrderBook`** | A data structure that stores resting limit orders. | Maintains sorted bid and ask levels for a single instrument. |
| **`OrderManager`** | A repository for all live orders within a partition. | Stores orders and provides fast lookups by Order ID. |
| **`Matcher`** | The core matching engine. | Implements the price-time priority matching algorithm; produces trades. |
| **`RiskManager`** | A component for pre-trade risk checks. | Validates orders against configurable risk limits (e.g., notional value). |
| **`ExecutionPublisher`** | A component for distributing execution reports. | Publishes trades and order status updates to downstream consumers. |
| **`ID Generators`** | Utility classes for creating unique IDs. | `OrderIDGenerator` and `TradeIDGenerator` ensure unique identifiers. |

## How to Use This Module

The `trading_core` is primarily exercised through its unit tests. The tests provide clear examples of how to interact with the system.

### Building and Running Tests

1.  Follow the build instructions in the main [README.md](../../README.md).
2.  Run the tests for this module:

```bash
# After building, from the build directory
./core/trading_core/tests/TradingCoreTests
./core/trading_core/tests/MatcherTests
./core/trading_core/tests/OrderBookTests
```

### Exploring the Code

*   **Test Files**: The best place to start is by reading the test files in the `tests/` directory. `TradingSystemTests.cpp` is particularly useful for understanding end-to-end order flow.
*   **Header Files**: The header files in `include/trading_core/` provide the public interface for each component.
*   **TSD**: For a detailed technical breakdown, read the [Trading Core TSD](./TSD.md).

### A Simple Experiment

To experiment with the matching logic:
1.  Create a new test case in `MatcherTests.cpp`.
2.  Create a `Matcher` and an `OrderBook`.
3.  Add some resting limit orders to the `OrderBook`.
4.  Create a new incoming order and pass it to the `Matcher`.
5.  Assert that the resulting trades match your expectations.

## Key Design Conventions

*   **Ownership**: The `OrderManager` owns all `Order` objects. Other components operate on raw pointers to these objects.
*   **No Blocking I/O**: The `WorkerThread` and all components it calls directly **must not** perform blocking I/O. All persistence is delegated to the `data` module.
*   **Performance**: The critical path (the `WorkerThread`'s command processing loop) is optimized by using references and avoiding heap allocations where possible.

## Limitations and Future Work

*   **No Live Gateway**: The system is currently only driven by programmatic commands in tests. A FIX or REST gateway would be a valuable addition.
*   **No State Recovery**: While trades are persisted, there is no mechanism to restore the system's state from the database on startup.
*   **No Monitoring**: There are no production-grade monitoring or metrics exporters.
