# `bulk::var::var`

```cpp
var(Hub& hub); // 1.
```

1. Constructs a variable and registers it with `hub`.

## Parameters

* `hub` - the hub this variable belongs to

## Complexity and cost

* **Cost** - `l`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto x = bulk::var<int, decltype(hub)>(hub);
    });

    return 0;
}
```
