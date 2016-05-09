# `bulk::provider`

Defined in header `<bulk/provider.hpp>`.

```cpp
class provider;
```

`bulk::provider` providers low-level transport layer functionality for the hub.

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();
    std::cout << hub.provider().available_processors() << std::endl;

    return 0;
}
```
