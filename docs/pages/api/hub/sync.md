# `bulk::hub::sync`

```cpp
void sync() const;
```

Performs a global barrier synchronization of the active processors.


## Complexity and cost

* **Cost** - `l`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        hub.sync();
    });

    return 0;
}
```
