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

## Future Enhancements and TODOs

This section outlines planned improvements and outstanding tasks for the FIX server component.

*   **`Main.cpp`**:
    *   Make the server port configurable (e.g., via command line arguments or a config file).
*   **`FixServer.cpp`**:
    *   Implement logic to remove sessions from `mMarketDataSubscriptions` when a session is unregistered.
*   **`FixSession.cpp`**:
    *   Refine `handleFixMessage` to dispatch to specific handlers based on `MsgType` more robustly.
    *   Send Reject (35=3) for malformed messages or missing `MsgType` during message parsing.
    *   Extract actual `OrderType` (Tag 40) and `TimeInForce` (Tag 59) from FIX message for `NewOrderSingle` requests.
    *   Send `BusinessMessageReject` (35=j) for parsing failures in `NewOrderSingle` requests.
    *   Add cases for other common FIX message types: Logon (A), Logout (5), Heartbeat (0), Test Request (1), Resend Request (2), Sequence Reset (4), Reject (3), Business Message Reject (j).
    *   Send Reject (35=3) for unsupported `MsgType`.
    *   Send Reject (35=3) or `BusinessMessageReject` (35=j) depending on error context in `handleFixMessage` exception handling.
    *   Send `BusinessMessageReject` (35=j) if neither `OrderID` nor `ClOrdID` is present in `CancelOrderRequest`.
    *   Send `BusinessMessageReject` (35=j) for invalid `OrderID`/`ClOrdID` format in `CancelOrderRequest`.
    *   Send `BusinessMessageReject` (35=j) for parsing failures in `CancelOrderRequest`.
    *   Implement actual submission of `trading_core::ModifyOrder` command, ensuring constructor arguments align with `fix::ModifyOrder` data.
    *   Send `BusinessMessageReject` (35=j) for parsing failures in `ModifyOrderRequest`.
    *   Integrate with an actual market data system to provide real data for `MarketDataRequest`.
    *   Implement continuous incremental market data updates (e.g., via a timer or a dedicated publisher) for subscribed sessions.
    *   Implement actual unsubscription logic, removing the session from market data distribution lists.
    *   Send `BusinessMessageReject` (35=j) for parsing failures in `MarketDataRequest`.
*   **`BinaryToOrderRequestConverter.cpp`**:
    *   Consider moving helper functions (`splitToMap`, `charToOrderSide`) to a common utility or making them private static members if only used by this class.
*   **`ExecutionReportToBinaryConverter.cpp`**:
    *   Consider making helper functions (`orderStatusToChar`, `orderSideToChar`, `timestampToString`) private static members of `ExecutionReportToBinaryConverter`.
*   **`BinaryToMarketDataRequestConverter.cpp`**:
    *   Implement a robust FIX message parsing utility (e.g., using a tag-value map approach similar to `BinaryToOrderRequestConverter`). The current implementation is a simplified skeleton and needs to be made production-ready.
*   **`BinaryToCancelOrderRequestConverter.cpp`**:
    *   Implement a robust FIX message parsing utility.
    *   Decide if `ClOrdID`, `OrderID`, `Symbol`, `Side` are mandatory for `CancelOrder`. If so, return `nullopt` if missing.
    *   Implement proper `TransactTime` (60) parsing from FIX message.
*   **`BinaryToModifyOrderRequestConverter.cpp`**:
    *   Implement a robust FIX message parsing utility.
    *   Decide if `ClOrdID`, `OrigClOrdID`, `OrderID`, `Symbol`, `Side`, `OrderQty`, `OrdType`, `Price` are mandatory for `ModifyOrder`. If so, return `nullopt` if missing.
    *   Implement proper `TransactTime` (60) parsing from FIX message.
*   **`MarketDataIncrementalRefreshToBinaryConverter.cpp`**:
    *   Implement robust FIX message serialization, potentially using a dedicated FIX message builder class, to ensure correctness of header, body, and trailer.
    *   Correctly calculate `BodyLength` (Tag 9) based on FIX specification.
    *   Add `SenderCompID` (Tag 49), `TargetCompID` (Tag 56), `MsgSeqNum` (Tag 34) from the session context.
    *   Implement a proper timestamp to string conversion utility for FIX.
*   **`MarketDataSnapshotFullRefreshToBinaryConverter.cpp`**:
    *   Implement robust FIX message serialization, potentially using a dedicated FIX message builder class, to ensure correctness of header, body, and trailer.
    *   Correctly calculate `BodyLength` (Tag 9) based on FIX specification.
    *   Add `SenderCompID` (Tag 49), `TargetCompID` (Tag 56), `MsgSeqNum` (Tag 34) from the session context.
    *   Implement a proper timestamp to string conversion utility for FIX.
