# `bulk::var::value`

```cpp
T& value();
```

Returns a reference to the value held by the local image of the variable.

## Return value

A reference to the value

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto x = bulk::create_var<int>(hub);
        bulk::sync();
        std::cout << x.value() << std::endl;
    });

    return 0;
}
```
