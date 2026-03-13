# Data Persistence Component

The Data module provides a persistence layer for the BetaTrader system.

## Overview

This module is responsible for asynchronously writing core business data, such as trades and orders, to a local SQLite database. It is designed to be a separate, non-blocking service that the `trading_core` can use for persistence without incurring I/O latency on its critical path. The architecture is built on an asynchronous worker pattern, where database operations are submitted to a queue and executed on a dedicated background thread.

## Key Responsibilities

*   Provide an asynchronous mechanism for database writes.
*   Manage a dedicated database connection on a background thread.
*   Offer high-level repositories for persisting `Order`, `Trade` objects, and FIX session sequence numbers.
*   Ensure trade ID uniqueness across system restarts.

## Getting Started

To build the module and run its associated tests, follow the instructions in the main project README and then execute the tests from the build directory:

```bash
# From the build directory
./core/data/tests/DataTests
```

## Further Reading

For a detailed technical breakdown of the architecture, class designs, and database schema, please refer to the [Technical System Design (TSD)](./TSD.md).
