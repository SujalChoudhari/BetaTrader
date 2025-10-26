# **core/data**

---

## 1. Overview

`core/data` is a subsystem of the **BetaTrader** core library.
It provides a unified data-access layer between the trading engine and persistent storage systems (e.g., SQLite).
The goal is to abstract away database logic and expose clean, type-safe APIs for reading and writing trading-related data.

---

## 2. Purpose

* Serve as a **bridge** between in-memory trading components and persistent data sources.
* Enable the trading engine to **store and recover** runtime state (orders, trades, positions, configurations).
* Maintain **data integrity** and **transactional consistency** between core modules and the database.

---

## 3. Functional Requirements

| ID   | Requirement          | Description                                                                                      |
| ---- | -------------------- | ------------------------------------------------------------------------------------------------ |
| FR-1 | Database Abstraction | The module must provide a database-agnostic interface for basic CRUD operations.                 |
| FR-2 | SQLite Integration   | Implement a concrete adapter using SQLite for local persistence.                                 |
| FR-3 | Schema Management    | Automatically create and migrate tables for orders, trades, and other entities.                  |
| FR-4 | Serialization        | Convert core types (`Order`, `Trade`, `OrderBookState`, etc.) into DB-friendly formats and back. |
| FR-5 | Thread Safety        | All write operations must be thread-safe.                                                        |
| FR-6 | Error Handling       | Database errors must be logged and surfaced via return codes or exceptions.                      |
| FR-7 | Connection Lifecycle | Manage DB connections efficiently, allowing reuse or scoped transactions.                        |
| FR-8 | State Persistence    | Allow modules (e.g., `OrderIDGenerator`, `Matcher`) to save and restore their current state.     |

---

## 4. Non-Functional Requirements

| ID    | Category    | Requirement                                                                          |
| ----- | ----------- | ------------------------------------------------------------------------------------ |
| NFR-1 | Performance | Database I/O should not block matching or order book updates.                        |
| NFR-2 | Reliability | Must gracefully handle file corruption or missing schema.                            |
| NFR-3 | Portability | Should compile cleanly on Linux, macOS, and Windows.                                 |
| NFR-4 | Scalability | Designed to support future replacement of SQLite with PostgreSQL or another backend. |
| NFR-5 | Testability | Mockable interfaces for unit testing without a real database.                        |

---

## 5. System Components

### 5.1 DataManager

* Central façade for all database interactions.
* Exposes typed methods such as:

  ```cpp
  void saveOrder(const Order&);
  std::vector<Order> loadOpenOrders();
  void saveTrade(const Trade&);
  ```
* Internally delegates to a database adapter (e.g., `SQLiteAdapter`).

### 5.2 IDStateRepository

* Handles saving and loading of incremental ID generators:

  ```cpp
  void saveOrderID(uint64_t id);
  uint64_t loadOrderID();
  ```

### 5.3 SQLiteAdapter

* Low-level database adapter built on top of SQLite C/C++ APIs.
* Responsible for:

    * Connection pooling
    * Transaction boundaries
    * SQL execution and statement binding

---

## 6. Data Flow

```
Order/Trade -> DataManager -> SQLiteAdapter -> SQLite DB file
SQLite DB file -> SQLiteAdapter -> DataManager -> Core modules
```

---

## 7. Interfaces

### Header Example: `DataManager.h`

```cpp
class DataManager {
public:
    explicit DataManager(const std::string& dbPath);
    void initSchema();

    void saveOrder(const Order& order);
    std::vector<Order> loadOrders();
    void saveTrade(const Trade& trade);
    std::vector<Trade> loadTrades();

    void saveState(const std::string& key, uint64_t value);
    uint64_t loadState(const std::string& key);

private:
    std::unique_ptr<SQLiteAdapter> adapter;
};
```

---

## 8. Future Extensions

* Add `PostgresAdapter` for remote DB support.
* Add caching layer for frequently accessed order/trade data.
* Integrate event-based replication for distributed trading nodes.
* Support configuration persistence (risk limits, user preferences).

---

## 9. Testing Strategy

* Use **GoogleTest** for unit and integration tests.
* Mock database interfaces for logic tests.
* Include a small embedded SQLite database for end-to-end verification.
* Validate:

    * Schema creation
    * Data insertion/retrieval
    * Recovery after restart


---

## 10. Dependencies

* **SQLite3**
* **GoogleTest** (for testing)
* **spdlog** (for logging)
* **nlohmann/json** (for optional serialization)

