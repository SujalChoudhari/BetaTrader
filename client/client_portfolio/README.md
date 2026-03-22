# Client | Portfolio & Risk

The `client_portfolio` module calculates real-time profit and loss (PnL), margin utilization, and aggregate positional exposure. 

## Overview

A trader needs to know their current risk exposure instantly. This module connects the historical fill data maintained by `client_blotter` with the live tick prices observed by `client_orderbook` to calculate dynamic metrics such as Volume Weighted Average Price (VWAP) and Unrealized PnL.

## Key Responsibilities

*   Automatically aggregate buy and sell executions into net position quantities.
*   Calculate and maintain the rolling Average Entry Price for positions.
*   Subscribe to Top-Of-Book (TOB) market data feeds to determine Mark-to-Market PnL.
*   Provide unified risk metrics for UI visualization.

## Architecture

```mermaid
graph TD
    classDef logic fill:#0b0c10,stroke:#c5c6c7,stroke-width:2px,color:white;
    classDef ui fill:#2b303a,stroke:#4caf50,stroke-width:2px,color:white;

    subgraph "Data Sources"
        BLOTTER[client_blotter]:::logic
        OB[client_orderbook]:::logic
    end

    subgraph "client_portfolio Module"
        RISK[PortfolioModel]:::logic
        MATH(PnL Calculator):::logic
    end

    BLOTTER -->|Fills/Executions| RISK
    OB -->|Pricing Updates| MATH
    RISK -->|Avg Entry| MATH
    MATH -.->|Unrealized PnL| UI(client_app UI):::ui
```

## Class Diagram

```mermaid
classDiagram
    class PortfolioModel {
        +processFill(ExecutionReport)
        +updateMarkPrice(Symbol, Price)
        +getOpenPositions() : vector~Position~
        +getRealizedPnL() : double
        +getUnrealizedPnL() : double
        -m_positions: map~Symbol, Position~
    }
    
    class Position {
        +symbol: Instrument
        +netQty: int64_t
        +avgEntryPrice: double
        +markPrice: double
        +realizedPnL: double
        +unrealizedPnL: double
        +calculateUnrealized()
    }

    PortfolioModel *--> Position : Holds
```

## Component Responsibilities

| Component | Description |
| :--- | :--- |
| **`PortfolioModel`** | The state container capturing net positions across all instruments. |
| **`Position`** | Represents the netted risk exposure for a single `Instrument`. Maintains its own ongoing PnL math state. |
| **`processFill()`** | Nets quantities. Triggers Realized PnL logic if the fill is offsetting an existing position, or recalculates `avgEntryPrice` if it is an expanding position. |

## Critical Design Conventions

-   **Zero-State Assumption**: The module derives all aggregate data from execution events. Reconnecting or restarting the client requires a database load or FIX sync to re-establish the baseline average prices.
-   **Lock-Free Polling**: All math is processed on the intake thread so the UI simply pulls atomic or mutex-protected pre-calculated values when drawing the dashboard.
