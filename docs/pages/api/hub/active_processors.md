# `bulk::hub::active_processors`

```cpp
int active_processors() const;
```

Returns the total number of active processors in an SPMD section.

## Return value

An `int` containing the number of active processors.

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    if (hub.available_processors() > 2) {
        hub.spawn(2, [&hub](int s, int p) {
            std::cout << hub.active_processors() << std::endl;
        });
    }

    return 0;
}
```
