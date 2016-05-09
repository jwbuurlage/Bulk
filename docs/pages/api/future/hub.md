# `bulk::future::hub`

```cpp
Hub& hub();
```

Returns a reference to the hub the future belongs to

## Return value

A reference to the hub

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/future.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto f = bulk::create_future<int>(hub);
        std::cout << f.hub().available_processors()
                  << " == " << hub.available_processors() << std::endl;
    });

    return 0;
}
```
