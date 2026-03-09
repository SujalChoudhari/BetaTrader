# Trading Core Component

The Trading Core is the in-memory matching engine and order management system for BetaTrader.

## Overview

This module is the heart of the trading system. It is responsible for managing the entire lifecycle of an order: receiving it, validating it against risk limits, matching it against other orders, and publishing the resulting executions. The architecture is designed for low-latency, high-throughput, and deterministic processing by partitioning instruments across single-threaded, lock-free workers.

## Key Responsibilities

*   Manage order state and provide fast lookups.
*   Implement a price-time priority matching algorithm.
*   Maintain an order book for each instrument.
*   Perform pre-trade risk checks.
*   Publish execution reports (trades and order status updates).
*   Generate unique order and trade identifiers.

## Getting Started

To build the module and run its associated tests, follow the instructions in the main project README and then execute the tests from the build directory:

```bash
# From the build directory
./core/trading_core/tests/TradingCoreTests
./core/trading_core/tests/MatcherTests
```

## Further Reading

For a detailed technical breakdown of the architecture, class designs, and threading model, please refer to the [Technical System Design (TSD)](./TSD.md).
