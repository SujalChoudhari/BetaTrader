# Technical Specification Document: FIX Protocol Library

## 1. Overview

This document outlines the technical details of the FIX protocol library (`@fix`). The library's primary responsibility is to handle the serialization and deserialization of FIX messages, specifically for `OrderRequest` (New Order - Single, `35=D`) and `OrderResponse` (Execution Report, `35=8`).

## 2. Message Formats

The library supports the following FIX message formats. The `SOH` character (`\x01`) is used as the field delimiter.

### 2.1. New Order - Single (`35=D`)

This message is deserialized into an `OrderRequest` object.

| Tag | Field Name        | C++ Type                | Notes                               |
|-----|-------------------|-------------------------|-------------------------------------|
| 8   | BeginString       | `std::string`           | Expected: `FIX.4.4`                 |
| 9   | BodyLength        | `size_t`                | Calculated and validated            |
| 35  | MsgType           | `char`                  | Expected: `D`                       |
| 49  | SenderCompID      | `fix::CompID`           |                                     |
| 56  | TargetCompID      | `fix::CompID`           |                                     |
| 34  | MsgSeqNum         | `fix::SequenceNumber`   |                                     |
| 52  | SendingTime       | `common::Timestamp`     |                                     |
| 11  | ClientOrderID     | `fix::ClientOrderID`    |                                     |
| 55  | Symbol            | `fix::Symbol`           |                                     |
| 54  | Side              | `common::OrderSide`     | `1`=Buy, `2`=Sell                   |
| 40  | OrdType           | `common::OrderType`     | `1`=Market, `2`=Limit               |
| 38  | OrderQty          | `fix::Quantity`         |                                     |
| 44  | Price             | `fix::Price`            | Required for Limit orders           |
| 59  | TimeInForce       | `common::TimeInForce`   | `0`=Day, `1`=GTC, etc.              |
| 99  | StopPrice         | `fix::Price`            | Optional                            |
| 63  | SettlType         | `uint8_t`               | Optional                            |
| 64  | SettlDate         | `uint32_t`              | Optional                            |
| 10  | CheckSum          | `int`                   | Calculated and validated            |

### 2.2. Execution Report (`35=8`)

This message is serialized from an `OrderResponse` object.

| Tag | Field Name        | C++ Type                | Notes                               |
|-----|-------------------|-------------------------|-------------------------------------|
| 8   | BeginString       | `std::string`           | `FIX.4.4`                           |
| 9   | BodyLength        | `size_t`                | Calculated                          |
| 35  | MsgType           | `char`                  | `8`                                 |
| 49  | SenderCompID      | `fix::CompID`           |                                     |
| 56  | TargetCompID      | `fix::CompID`           |                                     |
| 34  | MsgSeqNum         | `fix::SequenceNumber`   |                                     |
| 52  | SendingTime       | `common::Timestamp`     | Current UTC time                    |
| 37  | ExchangeOrderID   | `fix::ExchangeOrderID`  |                                     |
| 11  | ClientOrderID     | `fix::ClientOrderID`    |                                     |
| 17  | ExecutionID       | `std::string`           |                                     |
| 39  | OrderStatus       | `common::OrderStatus`   | `0`=New, `1`=PartiallyFilled, etc.  |
| 58  | Text              | `std::string`           | Optional                            |
| 55  | Symbol            | `fix::Symbol`           |                                     |
| 54  | Side              | `common::OrderSide`     | `1`=Buy, `2`=Sell                   |
| 38  | OrderQty          | `fix::Quantity`         |                                     |
| 14  | CumQty            | `fix::Quantity`         |                                     |
| 151 | LeavesQty         | `fix::Quantity`         |                                     |
| 31  | LastPrice         | `fix::Price`            |                                     |
| 32  | LastQty           | `fix::Quantity`         |                                     |
| 60  | TransactTime      | `common::Timestamp`     |                                     |
| 10  | CheckSum          | `int`                   | Calculated                          |

## 3. Implementation Details

### 3.1. Deserialization (`BinaryToOrderRequestConverter`)

The `convert` method will perform the following steps:

1.  **Validation**:
    - The checksum is validated by summing the ASCII values of all characters from `8=` to the start of the checksum field (`10=`) and comparing the result modulo 256 with the value in the checksum field.
    - The body length is validated by comparing the value of tag `9` with the actual length of the message body.
2.  **Tokenization**: The raw message is split by the `SOH` delimiter into a `std::map<int, std::string_view>`.
3.  **Population**: The map is used to populate the `OrderRequest` object. Helper functions are used to convert string values to enums and other types.

### 3.2. Serialization (`OrderResponseToBinaryConverter`)

The `convert` method will perform the following steps:

1.  **Body Construction**: A `std::stringstream` is used to build the message body. This is done first to calculate the body length.
2.  **Header Construction**: The header is built, including the calculated body length and a new timestamp.
3.  **Checksum Calculation**: The checksum is calculated for the combined header and body.
4.  **Final Assembly**: The header, body, and checksum are combined into the final FIX message.

## 4. Assumptions

- The FIX version is `FIX.4.4`.
- The input messages are well-formed to a reasonable extent. The validation logic is designed to catch common errors, but it is not exhaustive.
- The character encoding is ASCII.
