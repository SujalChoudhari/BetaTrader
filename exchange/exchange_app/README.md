# Exchange | App Orchestrator & Daemon

The `exchange_app` module is the top-level assembly for the BetaTrader Exchange server. It acts as the "braid" that weaves together the matching partitions, risk management, and protocol gateways into a single functional daemon.

## Overview

Unlike the micro-modules it contains, `exchange_app` is responsible for global lifecycle management. It handles the instantiation of singleton resources (like `DatabaseWorker` and ID Generators), initializes the instrument-specific matching partitions, and provides the public `submitCommand` interface used by networking gateways.

## Key Responsibilities

*   Initialize the global `DatabaseWorker` for asynchronous persistence.
*   Construct and own the `Partition` array based on the `Instrument` enumeration.
*   Provide a centralized `TradingCore` facade for command dispatch.
*   Orchestrate orderly shutdown and queue draining during maintenance.
*   Maintain the instance singleton for global access.

## Architecture

```mermaid
graph TD
    SERVER(fix_server Executable) --> CORE[exchange_app: TradingCore]
    
    subgraph "exchange_app internals"
        CORE --> P1(exchange_routing: Partition 1)
        CORE --> P2(exchange_routing: Partition 2)
        CORE --> DB(exchange_persistence: DatabaseWorker)
        CORE --> IDG(exchange_state: OrderIDGenerator)
    end
    
    subgraph "Networking Layers"
        FIX(exchange_fix: FIX Server)
        API(exchange_api: REST Server)
    end

    FIX -->|submitCommand| CORE
    API -->|submitCommand| CORE
```

## Class Diagram

```mermaid
classDiagram
    class TradingCore {
        +start()
        +stop()
        +submitCommand(unique_ptr~Command~)
        +getPartition(Instrument) : Partition*
        +TradingCore(dbWorker*, bool autoInit)
        +TradingCore(dbWorkerUptr, authRepoUptr, tradeIDRepoUptr, tradeIDGenUptr, orderIDGenUptr, autoInit)
        -initPartitions()
        -m_partitions: unique_ptr~Partition~[]
        -m_dbWorker: DatabaseWorker*
    }

    class Main {
        +main()
        -setupSignalHandlers()
    }

    Main --> TradingCore : Instantiates
```

## Component Responsibilities

| Component | Description |
| :--- | :--- |
| **`TradingCore`** | The central facade. It routes incoming commands to the specific `Partition` responsible for a given instrument. |
| **`Main`** | Entry point. Parses CLI arguments, sets up logging, and blocks on the `io_context` while the engine runs. |

## Critical Design Conventions

-   **Partition Isolation**: `TradingCore` ensures that a command for `EURUSD` never touches the state of `USDJPY` by routing exclusively to the dedicated partition.
-   **Resource Ownership**: This module is the "parent" of the server stack; objects created here are passed by reference down into the child micro-modules to enforce a clear ownership hierarchy.
