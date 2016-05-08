# `bulk::hub`

Defined in header `<bulk/hub.hpp>`.

```cpp
template<class Provider>
class hub;
```

`bulk::hub` is the central object that encapsulates the distributed system.

## Member types

| **Member type** | **Definition**            |
|-----------------|---------------------------|
| `variable_type` | `Provider::variable_type` |
| `array_type`    | `Provider::array_type`    |
| `future_type`   | `Provider::future_type`   |

## Member functions

| **System information**                                |                                            |
|-------------------------------------------------------|--------------------------------------------|
| [`available_processors`](hub/available_processors.md) | returns the number of available processors |
| `active_processors`                                   | returns the number of active processors    |
| `processor_id`                                        | returns the id of the local processor      |

## Example


```cpp
#include <iostream>

#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    });

    return 0;
}
```
