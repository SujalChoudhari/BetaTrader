# FIX Protocol Server (`@fix`)

The `@fix` component is a standalone server responsible for handling client connections over the Financial Information eXchange (FIX) protocol. It acts as the primary gateway for clients to submit orders to the BetaTrader trading engine.

## Overview

This component implements a fully asynchronous, multi-client FIX server using the Asio networking library. It listens for incoming TCP connections, manages client sessions, and translates FIX messages into commands for the `@trading_core`.

## Key Responsibilities

*   **Accepting Connections**: Listens on a configured TCP port for new client connections.
*   **Session Management**: Manages the lifecycle of each client session via `FixSessionManager`, including authentication and sequence number validation.
*   **Message Processing**:
    *   Deserializes FIX messages into internal request objects.
    *   Uses `OutboundMessageBuilder` for consistent, protocol-compliant binary FIX serialization (BodyLength, Checksum).
    *   Submits commands to the `@trading_core`.
*   **Execution Reporting**:
    *   Subscribes to execution events and routes them to the correct session.

## Key Components

- `FixServer`: The network entry point.
- `FixSession`: The per-client state machine.
- `FixSessionManager`: Logic for Logon, sequence numbers, and auth.
- `OutboundMessageBuilder`: Utility for building tags and calculating checksums.
- `FixUtils`: Shared parsing helpers.

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

## Further Reading

For a detailed architectural overview, class designs, and the session/order lifecycle flows, please refer to the [Technical System Design (TSD)](./TSD.md).

## Future Enhancements and TODOs

*   **Heartbeat Management**: Automatically send TestRequests and track latency.
*   **Sequence Number Persistence**: Load/Save session sequence numbers to `core/data`.
*   **Market Data Integration**: Connect `MarketDataRequest` handlers to the actual engine snapshots.
