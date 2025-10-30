# BetaTrader

A complete institutional-grade forex trading ecosystem.
This is a blueprint for a C++ platform that implements the full technology stack, from market simulation to client
execution.

## Overview

This system is designed from the ground up for one purpose: **to learn by doing**.
It demonstrates production-level distributed systems architecture, concurrency, and high-performance C++ patterns.

The platform is organized into five distinct layers, each designed to run as an independent set of processes.

## System Architecture

```
┌──────────────────────┐
│   Venue Simulators   │  Simulated liquidity providers and market data feeds
└──────────┬───────────┘
           │ (Market Data)
┌──────────▼───────────┐
│   Gateway Layer      │  Protocol translation and external connectivity
└──────────┬───────────┘
           │ (FIX Orders / Data)
┌──────────▼───────────┐
│   Core Engine        │  Order matching, risk management, execution
└──────────┬───────────┘
           │ (Trade / Position Events)
┌──────────▼───────────┐
│  Business Services   │  Position management, analytics, strategies
└──────────┬───────────┘
           │ (API Requests)
┌──────────▼───────────┐
│   Client Layer       │  API gateway and administrative interfaces
└──────────────────────┘
```

## Repository Structure

```
BetaTrader/
├── common/                          Shared libraries (types, logging, utils)
├── infrastructure/                  Message bus, database, caching helpers
│
├── venue/                      Simulated external market infrastructure
│   ├── liquidity-provider-sim/      Broker/liquidity provider simulator
│   └── price-feed-sim/              Market data feed generator
│
├── gateway/                    External connectivity layer
│   ├── fix-gateway/                 FIX protocol connectivity
│   └── market-data-gateway/         Market data aggregation
│
├── core/                       Critical path trading engine
│   └── trading_core/                Monolithic core (matching, OMS, risk)
│
├── business/                   Business logic services
│   ├── position-service/            Position tracking and PnL
│   ├── market-maker-service/        Automated quoting strategies
│   └── analytics-service/           Reporting and metrics
│
├── client/                     Client-facing layer
│   ├── client-api-gateway/          REST/WebSocket API
│   └── admin-console/               Operations dashboard (TUI or simple web)
│
└── tools/                           Development and operations utilities
    ├── backtester/                  Historical strategy testing
    └── monitoring/                  System observability
```

## Component Status

| Component              | Status      |
|------------------------|-------------|
| Common Libraries       | In Progress |
| Infrastructure         | Not Started |
| Liquidity Provider Sim | Not Started |
| Price Feed Sim         | Not Started |
| FIX Gateway            | Not Started |
| Market Data Gateway    | Not Started |
| Trading Core           | In Progress |
| Position Service       | Not Started |
| Market Maker Service   | Not Started |
| Analytics Service      | Not Started |
| Client API Gateway     | Not Started |
| Admin Console          | Not Started |

## Core Design Principles (The "Why")

This is the most important part. Understand *why* it's built this way.

1. **Monolithic Core**: The `trading_core` is a single process. This is intentional. The critical path (Order -\> Risk
   Check -\> Match -\> Execution) must happen in-memory. Network calls are a million times slower. We eliminate them.
2. **Gateway Isolation**: The "outside world" (FIX, market data, clients) is slow, messy, and unreliable. The
   `gateway-side` services are buffers. They handle the messy protocols and translate everything into a clean, internal
   message format. If a gateway crashes, it *never* takes the core down with it.
3. **Message-Driven**: Services do *not* call each other directly. They publish messages to a bus (ZeroMQ). The
   `position-service`, for example, subscribes to "Trade" messages from the core. This is called loose coupling. It lets
   you add, remove, or restart services without the rest of the system caring.
4. **Data Locality**: Data lives in three places.
    * **Hot (In-Memory)**: The live order book. Lives and dies inside the `trading_core`.
    * **Warm (Redis)**: Cross-service state that needs to be fast but durable (e.g., current positions).
    * **Cold (TimescaleDB)**: Data that must live forever (e.g., all historical trades, ticks).

-----

## Local Setup & Deployment

This project is 100% local. No Docker. You are the system administrator.

### Step 1: Install Dependencies

You must install these on your host machine.

* **Toolchain**:
    * `C++ Compiler`: A C++23 compliant compiler (GCC 14+, Clang 18+).
    * `CMake`: Version 3.28 or later.
    * `IDE`: CLion (recommended).
* **Services**:
    * `PostgreSQL`: The relational database.
    * `TimescaleDB`: The time-series extension for Postgres.
    * `Redis`: The in-memory cache.
* **Libraries** (install the `dev` packages):
    * `ZeroMQ` (libzmq, cppzmq)
    * `Boost` (Asio, UUID, etc. - v1.83+)
    * `FlatBuffers`
    * `Google Test` & `Google Benchmark`

### Step 2: Build the Project

Your IDE (CLion) will handle this, but you can also build from the terminal.

```bash
# Clone your repository
git clone <your-repo-url> BetaTrader
cd BetaTrader

# Create a build directory
mkdir build && cd build

# Configure (use Debug for development)
# This finds all your libraries and generates the build files
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build all components
# This will compile every target (core, gateways, etc.)
cmake --build . -j$(nproc)
```

### Step 3: Run the Ecosystem (The Orchestra)

You must start components in the correct order. Open a new terminal tab for each persistent process.

1. **Start Background Services**:

    * Run `redis-server`.
    * Run `postgresql`.

2. **Start Venue-Side**:

    * `./build/venue-side/price-feed-sim/price-feed-sim`
    * *This process starts generating fake market data.*

3. **Start Core-Side**:

    * `./build/core-side/trading_core/trading_core`
    * *The core starts, subscribes to no one, and waits for connections.*

4. **Start Gateway-Side**:

    * `./build/gateway-side/market-data-gateway/market-data-gateway`
    * *This connects to the Price Feed (Venue) and the Trading Core, funneling data in.*
    * `./build/gateway-side/fix-gateway/fix-gateway`
    * *This opens a port for FIX clients (like your simulator) to connect to.*

5. **Start Business-Side**:

    * `./build/business-side/position-service/position-service`
    * *This connects to the Core's message bus and starts listening for trade events.*

6. **Start Client-Side**:

    * `./build/client-side/client-api-gateway/client-api-gateway`
    * *This starts your REST/WebSocket server so you can talk to the system.*

-----

## Your Most Important Tool: Testing

An untested trading system is a useless one. It's a black box of silent, corrupting bugs. **Testing is not optional, it
is your primary development tool.**

* **Unit Tests (Google Test)**: These test *one function* at a time. (e.g., "Does my matching logic *really* follow
  Price-Time Priority?"). They are fast and save you *hours* of debugging.
* **Backtester (Your Tool)**: This is your end-to-end test. You feed it 1GB of real tick data and see if your
  `position-service` reports the same PnL as a simple Python script.

### How to Run Tests

From your `build` directory:

```bash
# Build all test targets
cmake --build . --target all_tests

# Run all compiled tests
ctest --output-on-failure
```

## Technology Stack

* **Language**: C++23
* **Build System**: CMake (3.28+)
* **IDE**: CLion
* **Messaging**: ZeroMQ (high-speed, low-latency messaging)
* **Serialization**: FlatBuffers (fastest way to serialize/deserialize data)
* **Time-Series Database**: TimescaleDB (for storing market data and trades)
* **Relational Database**: PostgreSQL (for users, settings, etc.)
* **Cache**: Redis (for warm, cross-service state)
* **Networking**: Boost.Asio
* **Testing**: Google Test (Unit), Google Benchmark (Performance)

## License
GNU General Public License v3.0. See LICENSE file for details.
