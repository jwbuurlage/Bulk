# `bulk::hub::provider`

```cpp
Provider& provider();
```

Returns the distributed system provider.

## Return value

The distributed provider of the hub.

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        hub.provider().available_processors();
    });

    return 0;
}
```
