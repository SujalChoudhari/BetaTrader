# FIX Protocol Library (`@fix`)

The FIX Protocol Library provides tools for serializing and deserializing Financial Information eXchange (FIX) messages.

## Overview

This library acts as a translation layer between the raw, text-based FIX protocol and the strongly-typed C++ objects used within the BetaTrader application. It is specifically focused on handling order-related messages required for the trading core.

## Key Responsibilities

*   Deserialize FIX "New Order - Single" (`35=D`) messages into `OrderRequest` objects.
*   Serialize `OrderResponse` objects into FIX "Execution Report" (`35=8`) messages.
*   Perform basic FIX message validation (checksum, body length).

## Getting Started

To use the library, include the necessary headers and link against the `fix_lib` target in CMake.

```cpp
#include "fix/BinaryToOrderRequestConverter.h"
#include "fix/OrderResponseToBinaryConverter.h"

// Example usage (see TSD for details)
```

## Further Reading

For detailed message formats, class designs, and implementation specifics, please refer to the [Technical System Design (TSD)](./TSD.md).
