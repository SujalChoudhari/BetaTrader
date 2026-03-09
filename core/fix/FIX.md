# FIX Protocol Developer Tag-Value Reference Guide
@page fix_guide FIX Protocol Guide

This guide serves as a quick, technical reference for developers, detailing the most critical and frequently used FIX tags, their required values, and their purpose. Understanding these values is essential for successful message construction, validation, and session management.

## I. Standard Header Fields (Mandatory Structure)

These fields are essential for transport and session identification. They **must** appear in the following fixed sequence: `8` -> `9` -> `35`.

| Tag | Field Name | Purpose & Developer Notes | Required Values | 
| ----- | ----- | ----- | ----- | 
| **8** | **BeginString** | **Protocol Version.** Must be the first field in the message. | **`FIXT.1.1`** (Modern Standard) | 
| **9** | **BodyLength** | **Message length in bytes.** Used for data integrity check; must be the second field. | Calculated byte count of message *after* Tag 9 and *before* Tag 10. | 
| **35** | **MsgType** | **Identifies the message's purpose.** Must be the third field. | **Char Code** (See sections II, III, and IV). | 
| 49 | SenderCompID | Assigned ID of the sending firm. | **String ID** (Must match counterparty's expectation). | 
| 56 | TargetCompID | Assigned ID of the receiving firm. | **String ID** (Counterparty's assigned ID). | 
| **34** | **MsgSeqNum** | **Sequential message number for the session.** Increments by 1 for every message sent. | **Integer** (Starts at `1` for a new session). | 
| 52 | SendingTime | Time message was generated (UTC). | **UTC Timestamp** (`YYYYMMDD-HH:MM:SS.mmm`). | 
| 10 | CheckSum | Three-digit integrity check. Must be the **last field** in the message. | **Three-digit Integer** (Calculated: sum ASCII chars mod 256). | 

## 2. FIX Session Control Messages (MsgType)

Messages used by the **Session Layer** to manage connection, sequence, and recovery.

| MsgType (35) | Message Name | Purpose | Critical Required Fields (in addition to Header) | 
| ----- | ----- | ----- | ----- | 
| **A** | **Logon** | Initiates connection, establishes heartbeating. | **HeartBtInt(108)** (e.g., `30` or `60`) | 
| **0** | **Heartbeat** | Monitors connection status during periods of inactivity. | None. | 
| **2** | **ResendRequest** | Requests retransmission of missed messages due to a sequence gap. | **BeginSeqNo(7)**, **EndSeqNo(16)** | 
| **4** | **SequenceReset** | Used for Gap Fill (skipping messages) or forced sequence synchronization. | **NewSeqNo(36)**, **GapFillFlag(123)** | 
| **5** | **Logout** | Terminates a FIX connection gracefully. | None. | 
| **3** | **Reject** | Issued when a received message violates a **session-level rule** (e.g., CheckSum failure, invalid Tag 35). | **RefSeqNum(45)** (original message's MsgSeqNum), **Text(58)** (reason). | 

### Sequence Recovery and Idempotency Fields

| Tag | Field Name | Context | Critical Values & Action | 
| ----- | ----- | ----- | ----- | 
| 43 | **PossDupFlag** | Used on **retransmitted messages** (in response to MsgType=2). | **`Y`** (Yes, possible duplicate). **Action:** Application must use persistent storage to ensure the transaction is not processed twice. | 
| 123 | **GapFillFlag** | Used in **SequenceReset (35=4)**. | **`Y`** (Gap Fill): Skip messages up to **NewSeqNo(36)**. **`N`** (Reset): Force peer to sync to **NewSeqNo(36)**. | 
| 7 | **BeginSeqNo** | Used in **ResendRequest (35=2)**. | **Integer.** Starting sequence number of the required range. | 
| 16 | **EndSeqNo** | Used in **ResendRequest (35=2)**. | **Integer.** End sequence number of the required range. **`0`** = Request all messages from **BeginSeqNo(7)** onwards. | 

## III. Core Application Messages (Trade Flow)

Messages used by the **Application Layer** to conduct trading business.

### 1. NewOrderSingle (MsgType=D)

| Tag | Field Name | Description | Key Values / Data Type | 
| ----- | ----- | ----- | ----- | 
| **11** | **ClOrdID** | **Client Order ID.** Unique ID assigned by the client for tracking. | **Unique String.** | 
| 55 | Symbol | Identifier of the security being traded. | **String** (e.g., `GOOG`, `GBP/JPY`). | 
| 54 | Side | Buy or Sell. | **`1`** (Buy), **`2`** (Sell), **`5`** (Sell Short). | 
| 60 | TransactTime | Time of order creation/submission (UTC). | **UTC Timestamp.** | 
| 38 | OrderQty | The quantity being ordered. | **Decimal** (Required component). | 
| 40 | OrdType | Type of order to be executed. | **`1`** (Market), **`2`** (Limit), **`3`** (Stop). | 
| 44 | Price | Required for **Limit** orders (`OrdType=2`). | **Decimal** (Price value). | 

### 2. ExecutionReport (MsgType=8)

The primary message for all status updates, fills, and rejections.

| Tag | Field Name | Description | Key Values / Data Type | 
| ----- | ----- | ----- | ----- | 
| **37** | **OrderID** | **Broker/Exchange ID.** Unique identifier that persists across the order's entire life cycle. | **String.** | 
| **17** | **ExecID** | **Event ID.** Unique identifier for this *specific* Execution Report instance/event. | **String.** | 
| **150** | **ExecType** | **The event that *triggered* this report.** | **`0`** (New), **`1`** (Partial Fill), **`2`** (Fill), **`4`** (Canceled), **`8`** (Rejected), **`F`** (Trade/Fill). | 
| **39** | **OrdStatus** | **The *resulting current state* of the order.** | **`0`** (New), **`1`** (Partially Filled), **`2`** (Filled), **`4`** (Canceled), **`8`** (Rejected). | 
| 14 | CumQty | Total quantity **filled so far** for the order. | **Decimal.** | 
| 151 | LeavesQty | Total **remaining quantity** open for execution. | **Decimal.** | 
| 6 | AvgPx | Volume-weighted average price of all fills for this order. | **Decimal.** | 

### 3. BusinessMessageReject (MsgType=j)

Used when an application message is syntactically valid (passes session checks) but fails a **business rule**.

| Tag | Field Name | Description | Key Values / Data Type | 
| ----- | ----- | ----- | ----- | 
| 379 | BusinessRejectReason | Categorical reason for the rejection. | **`0`** (Unknown ID), **`1`** (Unknown Security), **`3`** (Unsupported MsgType). | 
| 371 | RefTagID | The specific field tag (if applicable) that caused the business rule violation. | **Integer Tag ID** (e.g., `40` if OrdType was unsupported). | 
| 58 | Text | Descriptive explanation of the business failure. | **Descriptive String.** | 

## 4. Market Data and Order Book Management

This section covers the request for market data (MsgType=V) and the subsequent data messages used to build and update the order book (MsgType=W and X).

### 1. MarketDataRequest (MsgType=V) - Requesting the Order Book

| Tag | Field Name | Description | Key Values / Data Type | 
| ----- | ----- | ----- | ----- | 
| **262** | **MDReqID** | **Market Data Request ID.** Unique ID assigned by the client to identify *this specific request*. | **Unique String.** | 
| **263** | **SubscriptionRequestType** | Specifies the type of data subscription desired. | **`0`** (Snapshot), **`1`** (Snapshot + Updates), **`2`** (Unsubscribe). | 
| **264** | **MarketDepth** | Number of price levels (depth) required for the order book. | **`0`** (Full Book), **`1`** (Top of Book / Level 1). | 
| **146** | **NoRelatedSym** | **Counter Tag.** Must precede the list of requested symbols. | **Integer** (Number of symbols being requested). | 

**Note on Usage:** `NoRelatedSym(146)` is followed by a repeating group of fields defining the symbols (e.g., `Symbol(55)`) and other security identifiers.

### 2. MarketDataSnapshotFullRefresh (MsgType=W)

Used to transmit the **initial full state** of the order book after a successful request.

| Tag | Field Name | Description | Key Values / Data Type | 
| ----- | ----- | ----- | ----- | 
| 262 | MDReqID | The original ID from the request (Tag 262). | **String.** | 
| **268** | **NoMDEntries** | **Counter Tag.** The number of Bid/Offer/Trade entries included in this message. | **Integer.** | 

### 3. MarketDataIncrementalRefresh (MsgType=X)

Used for **real-time updates** to the order book. Requires specific actions to be processed.

| Tag | Field Name | Description | Key Values / Data Type | 
| ----- | ----- | ----- | ----- | 
| 262 | MDReqID | The original ID from the request (Tag 262). | **String.** | 
| **268** | **NoMDEntries** | **Counter Tag.** The number of updates in this message. | **Integer.** | 

### 4. Market Data Entry Repeating Group (Tags within NoMDEntries(268))

These fields **must** repeat for every entry listed under **NoMDEntries(268)**. The order shown below is mandatory.

| Tag | Field Name | Description | Key Values & Action | 
| ----- | ----- | ----- | ----- | 
| **279** | **MDUpdateAction** | **Action for the entry** (critical for MsgType=X only). | **`0`** (New), **`1`** (Change/Update), **`2`** (Delete). | 
| **269** | **MDEntryType** | Defines whether the entry is a Bid, Offer, or Trade. | **`0`** (Bid), **`1`** (Offer), **`2`** (Trade). | 
| **270** | **MDEntryPx** | The price of the bid, offer, or trade. | **Decimal** (Price value). | 
| **271** | **MDEntrySize** | The size/quantity of the entry. | **Decimal** (Size/Volume). | 
| 273 | MDEntryTime | The time the entry was generated (for timestamping the event). | **Time/UTC Timestamp.** | 
| 1023 | MDEntryPositionNo | Required for maintaining order book depth (e.g., 1st level, 2nd level). | **Integer.** | 

## 5. Generic Repeating Group Constraints

When constructing complex messages (like an Allocation message or an order with multiple legs), repeating groups must adhere to strict sequencing.

1. **Counter First:** The counter tag (`NoXXX`) **must** immediately precede the first instance of the group.

    * *Example:* `NoLegs(555)=2` must come before `LegSecurityID(602)`.

2. **Strict Internal Order:** The fields *within* each repeating instance **must** follow the order defined in the FIX specification. Failure to maintain this order will result in a **Session Reject (35=3)**.

| Sequence Step | Tag Type | Example | Developer Mandate | 
| ----- | ----- | ----- | ----- | 
| **1** | **Counter** | `78=2` (NoAllocs) | Must specify the total count of repetitions. | 
| **2** | **Start of Instance 1** | `79=AllocA` (AllocID) | Must follow the counter immediately. | 
| **3** | **Internal Field 1** | `80=500` (AllocQty) | Must be ordered exactly as per FIX spec. | 
| **4** | **Start of Instance 2** | `79=AllocB` (AllocID) | Instance fields repeat in the same order. | 
| **5** | **Internal Field 2** | `80=500` (AllocQty) | ...and so on for all instances. |