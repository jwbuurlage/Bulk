# `bulk::coarray::operator()`

```cpp
image operator()(int t);
```

Obtain an object encapsulating the image of the coarray.

## Parameters

* `t` - the image index

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
        auto xs_image = xs(0);
    });

    return 0;
}
```
