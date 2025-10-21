# **Functional Specification Document (FSD) — BetaTrader Trading Core**

---

## 1. Purpose and Context

The **Trading Core** is the *brain* of the BetaTrader ecosystem.
Its job is to decide, with precision and speed, *what happens when a trade order enters the system*.

Every order to buy or sell a financial instrument (like EUR/USD) flows through this core.
The core decides whether the order is valid, whether it can be executed, and at what price.
When a match happens between a buyer and a seller, it produces a **trade** — a confirmed transaction.

Unlike the rest of the system (gateways, APIs, analytics), which deal with communication or presentation, the Trading Core handles the *critical path* of execution.
It operates fully **in-memory** for nanosecond-to-microsecond-level speed.

You can think of it as a **real-time micro-exchange**: a private marketplace where all client orders are matched.

---

## 2. Functional Overview

### 2.1 Key Responsibilities

1. **Accept and process trading orders** from clients.
2. **Check and enforce trading rules and risk limits** before accepting any order.
3. **Match compatible buy and sell orders** according to price-time priority.
4. **Generate and distribute trade events** to other parts of the system.
5. **Maintain the state of each instrument’s order book** (who is bidding, who is offering).
6. **Handle cancellations, modifications, and rejections** safely.
7. **Persist and recover state** after restarts without losing consistency.
8. **Provide metrics and control commands** for monitoring and operations.

---

## 3. Conceptual Explanation

In a typical trading system, multiple participants (brokers, algorithms, market-makers) send *orders* to buy or sell.
An order could say, for example:

> “Buy 1 million EUR/USD at 1.0850 or better.”

Each instrument (e.g., EUR/USD, GBP/USD) has its own *order book*, which is just two sorted lists:

* **Bids** (buyers): highest prices first.
* **Asks** (sellers): lowest prices first.

When a new order comes in:

* If it finds a compatible opposite order (a buyer at or above the seller’s price), they are *matched* — a **trade** is executed.
* If not, the new order waits in the order book until a future match.

This process is called **matching**, and the Trading Core implements it deterministically, with precise time priority.

---

## 4. Functional Requirements

### 4.1 Order Handling

* Receive normalized orders from the gateway layer (already parsed from FIX or API).
* Supported types:

    * **Limit Order:** execute at a given price or better.
    * **Market Order:** execute immediately at the best available price.
    * **Cancel/Replace:** cancel or modify existing orders.
* Validate each order:

    * Syntax (price > 0, quantity > 0).
    * Instrument availability.
    * Client authorization.
    * System state (e.g., trading hours, instrument enabled).

### 4.2 Risk Management

* Before accepting an order:

    * Check account margin or available balance.
    * Enforce maximum order size or position exposure.
    * Prevent self-trading (client cannot buy and sell against itself).
* On violation, reject the order and log the reason.

### 4.3 Order Book Management

* Maintain one **in-memory order book per instrument**.
* Support efficient operations:

    * Add new order.
    * Modify existing order.
    * Cancel order.
    * Query top-of-book (best bid/ask).
* Preserve **price-time priority**:

    * Better price first.
    * If equal price, earlier order first.

### 4.4 Matching Engine

* Match incoming orders to resting ones in the opposite book.
* Execute trades at the price of the resting order.
* Support:

    * Partial fills (order partially matched).
    * Full fills (order completely executed).
    * Multiple matches in one event cycle.
* Generate trade records immediately.

### 4.5 Execution Reporting

For every order event, produce a structured message:

* New Order Accepted
* Order Rejected
* Order Filled (partial/full)
* Order Canceled
* Trade Executed

Each message is published through the internal message bus (ZeroMQ).

### 4.6 Event Publishing

All important results are sent out as asynchronous events:

* **Trades** → Position and Analytics services.
* **Execution Reports** → Gateways → Clients.
* **Order Book Snapshots** → Market Data Gateway.
  This design ensures that the core never blocks waiting for another service.

### 4.7 Persistence and Recovery

* Live data: stored in RAM (order books).
* Snapshot data: periodically persisted to Redis (open orders, positions).
* Historical data: written to TimescaleDB (for reporting and backtesting).
* On restart, the core replays the last known state from Redis.

### 4.8 Monitoring and Control

Expose lightweight commands (via local TCP or TUI):

* `show_book EURUSD`
* `cancel_all <client_id>`
* `status` → number of active orders, CPU, latency.
  Emit metrics continuously for external monitoring.

---

## 5. Non-Functional Requirements

| Category         | Description                                                                      |
| ---------------- | -------------------------------------------------------------------------------- |
| **Performance**  | End-to-end order processing latency ≤ 100 µs.                                    |
| **Concurrency**  | Independent processing threads per instrument partition.                         |
| **Scalability**  | Scales horizontally by instrument or by client segment.                          |
| **Reliability**  | Crash recovery without trade duplication or loss.                                |
| **Determinism**  | Given same inputs and timestamps, output must be identical.                      |
| **Security**     | Isolated from external gateways; communicates only through internal message bus. |
| **Auditability** | Every state change logged with timestamp and sequence number.                    |

---

## 6. Interfaces

### 6.1 Inputs

| Message Type       | Description                              |
| ------------------ | ---------------------------------------- |
| `NewOrder`         | Request to enter a buy/sell order.       |
| `CancelOrder`      | Request to cancel an existing order.     |
| `ModifyOrder`      | Change price or size.                    |
| `MarketDataUpdate` | Update of external reference price feed. |

### 6.2 Outputs

| Message Type      | Description                                                         |
| ----------------- | ------------------------------------------------------------------- |
| `ExecutionReport` | Confirmation of order state (accepted, filled, canceled, rejected). |
| `TradeEvent`      | Result of a match between two orders.                               |
| `RiskAlert`       | Notification of risk breach.                                        |
| `BookSnapshot`    | Current state of best bids/asks.                                    |

All serialized using **FlatBuffers**, transmitted through **ZeroMQ PUB/SUB** channels.

---

## 7. Example Workflow (Narrative)

1. **Market data arrives:**
   The price feed says EUR/USD is trading at 1.0850 / 1.0852.

2. **Client submits an order:**
   “Buy 1M EUR/USD at 1.0851.”
   The order enters the Trading Core via the gateway.

3. **Validation:**
   The core checks the client’s limits and the order parameters.

4. **Matching:**

    * The system looks into the sell side of the order book.
    * Finds a sell order at 1.0850 (better price).
    * Matches them.
    * Generates a trade: buy at 1.0850 for 1M.

5. **Reporting:**

    * Sends `ExecutionReport` (filled).
    * Sends `TradeEvent` (to position/analytics).
    * Updates internal state (order removed, book updated).

6. **Persistence:**
   Trade record is asynchronously written to the database.

This entire cycle typically completes in microseconds.

---

## 8. Data Structures (Simplified)

```cpp
struct Order {
    std::string order_id;
    std::string client_id;
    std::string symbol;
    Side side;        // BUY / SELL
    OrderType type;   // LIMIT / MARKET
    double price;
    double quantity;
    Timestamp ts;
};

struct Trade {
    std::string trade_id;
    std::string symbol;
    double price;
    double quantity;
    std::string buy_order_id;
    std::string sell_order_id;
    Timestamp ts;
};
```

---

## 9. Error Handling

* Invalid parameters → reject order.
* Insufficient margin → reject with risk alert.
* Unknown order ID on cancel → reject with message.
* Internal exception → rollback and log crash context.
* Gateway reconnection → resend last known book state.

---

## 10. Testing Requirements

| Test Type             | Objective                                             |
| --------------------- | ----------------------------------------------------- |
| **Unit Tests**        | Validate order book logic, matching, and risk checks. |
| **Integration Tests** | End-to-end order → trade → event flow.                |
| **Performance Tests** | Measure latency and throughput under load.            |
| **Persistence Tests** | Verify recovery consistency from Redis snapshot.      |
| **Replay Tests**      | Reprocess logs to confirm deterministic results.      |

---

## 11. Future Enhancements

* Multi-symbol matching across instruments (cross-currency).
* Plug-in strategies (internal market maker).
* Distributed matching via partitioning.
* FIX engine integration for external routing.
* GPU-accelerated backtesting.

---

## 12. Summary

The **Trading Core** is a high-speed, deterministic engine that:

* Acts like an exchange inside BetaTrader.
* Guarantees fairness (price-time priority).
* Ensures all orders are valid and risk-compliant.
* Publishes clear, atomic events for downstream consumers.

Its success metric is *correctness under load*:
every microsecond counts, but correctness never fails.

---

Would you like me to now produce the **Technical Specification Document (TSD)** — including class hierarchy, thread model, internal queues, and design patterns used (Reactor, Command, Publisher-Subscriber)?
