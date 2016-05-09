# `bulk::coarray::operator[]`

```cpp
T& operator[](int i);
```

Access the i-th element of the local coarray image

## Returns

A reference to the i-th element of the local image.

## Parameters

* `i` - the element index

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
        xs[2] = 1;
    });

    return 0;
}
```
