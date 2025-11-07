
# core/data — Technical Specification & Design (current)

This TSD summarizes the actual, implemented design and practical notes for the `core/data` module. It is
intended for developers who will read, extend, or debug the persistence layer.

Implemented pieces (current snapshot):

- `DatabaseWorker` — asynchronous task runner that executes database lambdas on a background thread.
- `OrderRepository`, `TradeRepository`, `TradeIDRepository` — small repository classes that enqueue SQL tasks.
- `Query.h` — the SQL strings for table creation and basic operations.

Design highlights

- Writes are intentionally asynchronous: callers enqueue a std::function that receives a `SQLite::Database&` and returns immediately. This keeps IO off the matching thread.
- The schema is compact: `orders`, `trades`, and a small table for storing the last trade id.
- Tests exercise the repositories and the trade id flow; mocks are used to avoid real DB I/O where appropriate.

Schema (what's created by the current code)

```sql
-- trades table (created by data::query::createTradeTableQuery)
CREATE TABLE IF NOT EXISTS trades (
    trade_id INTEGER PRIMARY KEY,
    symbol TEXT,
    buy_order_id INTEGER,
    sell_order_id INTEGER,
    quantity INTEGER,
    price REAL,
    timestamp INTEGER
);

-- trade_id table (single row store for the current trade id)
CREATE TABLE IF NOT EXISTS trade_id (id INTEGER PRIMARY KEY);

-- orders table
CREATE TABLE IF NOT EXISTS orders (
    order_id INTEGER PRIMARY KEY,
    client_id INTEGER,
    symbol TEXT,
    side TEXT,
    type TEXT,
    price REAL,
    original_quantity INTEGER,
    remaining_quantity INTEGER,
    status TEXT,
    timestamp INTEGER
);
```

Operational notes

- If you need synchronous guarantees for a particular operation, add a future/promise wrapper around the enqueue call and wait in the caller (use sparingly).
- The `DatabaseWorker` uses `rigtorp::SPSCQueue` for task handoff and `std::jthread` to run the worker loop.

Extending the module

- To add a new persisted entity, add SQL to `Query.h`, implement a small repository header+cpp (follow existing pattern), and add tests in `core/data/tests/`.
- To swap SQLite for another backend, implement a small adapter that provides the same task signature and execute it from `DatabaseWorker` (or replace `DatabaseWorker` with a backend-agnostic worker).

