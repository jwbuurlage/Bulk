# `bulk::var::operator=`

```cpp
void operator=(var<T, Hub>&& other); // 1.
```

1. Move assignment operator. Replaces the target variable `*this` with the source variable `other`, and invalidates the source.

## Parameters

* `other` - another variable to move away from

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
        auto x = bulk::create_var<int>(hub);
        decltype(x) y = std::move(x); // 1.
    });

    return 0;
}
```
