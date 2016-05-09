# `bulk::coarray::coarray`

```cpp
coarray(Hub& hub, int local_size); // 1.
coarray(Hub& hub, int local_size, T default_value); // 2.
```

1. Constructs a coarray of size `p x local_size` and registers it with `hub`.
2. In addition, also initializes the elements as `default_value`.

## Parameters

* `hub` - the hub this variable belongs to
* `local_size` - the size of the coarray image of the local processor
* `default_value` - the initial value of the local coarray elements

## Complexity and cost

* **Cost** - `l`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto xs = bulk::coarray<int, decltype(hub)>(hub, 10); // 1.
        auto ys = bulk::coarray<int, decltype(hub)>(hub, 10, 1); // 2.
    });

    return 0;
}
```
