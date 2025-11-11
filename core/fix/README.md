# FIX Protocol Library (`@fix`)

This library is responsible for handling the Financial Information eXchange (FIX) protocol, the standard for electronic trading. It provides the necessary tools to serialize and deserialize FIX messages, bridging the gap between the raw text-based protocol and the strongly-typed C++ objects used in this application.

## Core Components

- **`BinaryToOrderRequestConverter`**: Deserializes a binary FIX message (specifically, a New Order - Single, `35=D`) into an `OrderRequest` object. This is used to process incoming order requests from clients.

- **`OrderResponseToBinaryConverter`**: Serializes an `OrderResponse` object into a binary FIX message (an Execution Report, `35=8`). This is used to send order status updates to the execution publisher.

## Usage

To use this library, simply include the necessary header files and link against the `fix_lib` target in your `CMakeLists.txt`.

### Deserializing an Order Request

```cpp
#include "fix/BinaryToOrderRequestConverter.h"
#include <vector>

// Assume binaryData contains a raw FIX message
std::vector<char> binaryData = ...;

fix::BinaryToOrderRequestConverter converter;
fix::OrderRequest orderRequest = converter.convert(binaryData);
```

### Serializing an Order Response

```cpp
#include "fix/OrderResponseToBinaryConverter.h"
#include "fix/OrderResponse.h"

// Assume orderResponse is a fully populated OrderResponse object
fix::OrderResponse orderResponse = ...;

fix::OrderResponseToBinaryConverter converter;
std::vector<char> binaryData = converter.convert(orderResponse);
```
