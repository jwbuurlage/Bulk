# `bulk::var::hub`

```cpp
Hub& hub();
```

Returns a reference to the hub the variable belongs to

## Return value

A reference to the hub

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto x = bulk::create_var<int>(hub);
        std::cout << x.hub().available_processors()
                  << " == " << hub.available_processors() << std::endl;
    });

    return 0;
}
```
