# BetaTrader client terminal UI

This guide documents the verified 1440×900 client layouts after the deterministic DockBuilder arrangement was added. The default cockpit keeps the chart central, the order-book workflow on the right, the FIX log and blotter along the bottom, and exchange/simulator operations in a separate bottom tab group.

## Default cockpit

![Live-data BetaTrader cockpit](screenshots/cockpit-default.png)

The primary cockpit image is a live local-session capture, not a placeholder: it starts with about an hour of deterministic simulated 1-minute history, continues with live EURUSD L2 depth, repeated FIX market-data events, an active FIX session, and the running simulator. The previous shorter live frame is retained as [`cockpit-live-3-candles.png`](screenshots/cockpit-live-3-candles.png); the expanded source frame is also available as [`cockpit-backfilled-live.png`](screenshots/cockpit-backfilled-live.png).

The default arrangement contains these nine application panels:

| Panel | Placement | Purpose |
| --- | --- | --- |
| Market Chart | central workspace | Candles and symbol/period controls |
| Order Book (L2) | upper-right | Bid/ask depth, spread, mid-price, and depth scale |
| Order Entry | middle-right | Symbol, price, quantity, order type, TIF, BUY/SELL |
| FIX Connection Control | lower-right | Session configuration, connection lifecycle, and market-data subscription |
| FIX Message Log | bottom-left | Session, FIX, execution, rejection, and market-data events |
| Open Orders | bottom-center tab | Working order state |
| Execution History | bottom-center tab | Completed/rejected/cancelled execution history |
| Exchange Management Console | bottom-right tab | Embedded exchange lifecycle and session/engine statistics |
| Simulator Dashboard | bottom-right tab | Local stochastic simulator controls and status |

## Panel-by-panel captures

The individual captures are crops from the same expanded default cockpit so that every panel title and control region can be inspected without overlap:

- [Market Chart](screenshots/panel-market-chart.png)
- [Order Book](screenshots/panel-order-book.png)
- [Order Entry](screenshots/panel-order-entry.png)
- [FIX Connection Control](screenshots/panel-fix-connection-control.png)
- [FIX Message Log](screenshots/panel-fix-message-log.png)
- [Open Orders / Execution History tabs](screenshots/panel-open-orders-and-execution-history.png)
- [Exchange Management Console / Simulator Dashboard tabs](screenshots/panel-exchange-and-simulator.png)
- [Panel gallery](screenshots/panel-gallery.png)

Live-data panel crops from the earlier verified session remain available above. The expanded backfill capture adds the following six focused views without duplicating them in the root README:

- [Market Chart — backfilled history and live continuation](screenshots/panel-chart-backfilled-live.png)
- [Order Book — live depth](screenshots/panel-order-book-backfilled-live.png)
- [Order Entry](screenshots/panel-order-entry-backfilled-live.png)
- [FIX Connection Control — active subscription](screenshots/panel-fix-connection-backfilled-live.png)
- [FIX Message Log — market-data events](screenshots/panel-fix-message-log-backfilled-live.png)
- [Simulator Dashboard — running local simulation](screenshots/panel-simulator-backfilled-live.png)

## Order-book workflow layouts

These layouts keep the book and order-entry workflow visible while changing the amount of operational workspace exposed. They are preview layouts; the deterministic default cockpit remains the production default.

![Order-book layout gallery](screenshots/book-layout-gallery.png)

- [Book cockpit](screenshots/book-layout-cockpit.png): chart, L2 book, order ticket, FIX controls, and bottom operational tabs.
- [Book operations](screenshots/book-layout-operations.png): more room for exchange/simulator monitoring below the chart.
- [Book operations-heavy](screenshots/book-layout-operations-heavy.png): larger monitoring area for operational workflows.
- [Book-focused detail](screenshots/book-focused-detail.png): enlarged right-side book, order-entry, and connection workflow.
- [Book-focused workflow](screenshots/book-focused-workflow.png): enlarged right-side workflow with the bottom operational tabs visible.

## Capture and verification notes

- Captures are 1440×900 PNGs produced from the native `client_app` under Xvfb with multi-viewport disabled only for deterministic headless geometry. The production multi-viewport setting remains enabled in `UIManager.cpp`.
- `cockpit-layout-only.png` preserves the earlier geometry-only reference. It is not the primary acceptance screenshot because it intentionally has no market-data subscription.
- The live-data capture was produced by starting the embedded exchange, seeding 60 deterministic 1-minute and 36 deterministic 5-minute local-demo candles immediately before the current bucket, starting the simulator, connecting/logging on the local FIX client, subscribing to EURUSD, and visually checking the resulting chart/order-book state.
- The backfill is simulated demo history, not an assertion about external historical market data. Its seeded random walk uses symbol-aware reference prices and stops before the live bucket so live ticks continue naturally.
- The source was checked with the repository CMake preset, the full CTest suite, the focused `ClientWorkingClientSmoke` test, the active-candle regression, the deterministic backfill regression, and the multi-producer database-worker regression. `git diff --check` also passed.
