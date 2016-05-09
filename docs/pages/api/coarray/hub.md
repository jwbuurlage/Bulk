# `bulk::coarray::hub`

```cpp
Hub& hub();
```

Returns a reference to the hub the coarray belongs to

## Return value

A reference to the hub

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/coarray.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto xs = bulk::create_coarray<int>(hub, 10);
        std::cout << xs.hub().available_processors()
                  << " == " << hub.available_processors() << std::endl;
    });

    return 0;
}
```
