# Client Application - Technical System Design (TSD)

## Overview

The BetaTrader Client Application is designed to simulate a trader's terminal or automated trading system connecting to the `fix_server`. It leverages a modular architecture to separate the FIX protocol logic from the graphical user interface.

## Modules

### 1. `client_fix` (FIX Engine)
-   **Responsibilities**: Asynchronous TCP connections, FIX message parsing/building, session management (Logon, sequence numbers, heartbeats).
-   **Dependencies**: ASIO for networking, `common` and `common_fix` for shared utilities and message definitions.
-   **Key Components**:
    -   `FixClientSession`: Manages state and ASIO socket.
    -   `ClientMessageBuilder`: Constructs Outbound FIX messages (NewOrderSingle, CancelRequest, etc.).

### 2. `client_ui` (User Interface)
-   **Responsibilities**: Rendering the graphical interface, capturing user input, displaying real-time data.
-   **Technology**: **Dear ImGui** (Immediate Mode GUI), likely using an OpenGL/GLFW or SDL2 backend depending on platform support.
-   **Key Components**:
    -   `ConnectionPanel`: Form to input IP/Port and credentials, and a connect/disconnect button.
    -   `OrderEntryPanel`: Input fields for Symbol, Side, Quantity, Price, and a Submit button.
    -   `OrderBlotter`: A tabular view of active/historical orders based on `ExecutionReport`s received.

### 3. `client_app` (Application Glue)
-   **Responsibilities**: Initialization, main event loop integration.
-   **Key Components**:
    -   `main.cpp`: Initializes ASIO context, ImGui context, instantiates `client_fix` and `client_ui`, and runs the application loop.

## Integration with Core
The application will utilize shared FIX protocol definitions and utilities moved to the `common` directory from `core/fix` to ensure DRY compliance and guaranteed protocol compatibility between server and client.
