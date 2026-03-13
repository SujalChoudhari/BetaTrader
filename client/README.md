# Client Application

The Client Application (`client`) is the front-end interface for interacting with the BetaTrader `fix_server`. It acts as a FIX client to connect, authenticate, send orders, and receive real-time execution and market data updates.

## Architecture

The client application is divided into three main modules:

1.  **`client_fix`**: The networking and protocol layer. Handles the TCP connection using ASIO and manages the FIX session state (sequence numbers, logon, heartbeat).
2.  **`client_ui`**: The user interface layer built using **Dear ImGui**. Provides screens for connection management, order entry, and a live order blotter/market data viewer.
3.  **`client_app`**: The application executable that glues the FIX client and UI together, managing the main event loop and dependency injection.

For more detailed technical design, please refer to the [TSD (Technical System Design)](./TSD.md).
