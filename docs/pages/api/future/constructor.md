# `bulk::future::future`

```cpp
future(Hub& hub);
```

1. Constructs a future for use in `hub`.

## Parameters

* `hub` - the hub this future belongs to

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/future.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto f = bulk::future<int, decltype(hub)>(hub);
    });

    return 0;
}
```
