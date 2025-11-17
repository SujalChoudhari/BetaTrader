# FIX Protocol Server (`@fix`)

The `@fix` component is a standalone server responsible for handling client connections over the Financial Information eXchange (FIX) protocol. It acts as the primary gateway for clients to submit orders to the BetaTrader trading engine.

## Overview

This component implements a fully asynchronous, multi-client FIX server using the Asio networking library. It listens for incoming TCP connections, manages client sessions, and translates FIX messages into commands for the `@trading_core`.

## Key Responsibilities

*   **Accepting Connections**: Listens on a configured TCP port for new client connections.
*   **Session Management**: Manages the lifecycle of each client session, including reading and writing data.
*   **Message Processing**:
    *   Deserializes FIX "New Order - Single" (`35=D`) messages into internal `OrderRequest` objects.
    *   Submits new orders to the `@trading_core` for processing.
*   **Execution Reporting**:
    *   Subscribes to execution events from the `@trading_core`.
    *   Serializes `ExecutionReport` objects into FIX "Execution Report" (`35=8`) messages and sends them to the appropriate client.
*   **Logging**: Provides structured, detailed logging for all major events and errors using the application-wide runbook system.

## Building and Running

The FIX server is built as a separate executable target named `fix_server`.

1.  **Build the target**:
    ```bash
    cmake --build . --target fix_server
    ```
2.  **Run the server**:
    ```bash
    ./build/core/fix/fix_server
    ```
The server will start and begin listening for connections on the configured port (default: 12345).

## Further Reading

For a detailed architectural overview, class designs, and the order lifecycle flow, please refer to the [Technical System Design (TSD)](./TSD.md).
