# `bulk::create_coarray`

Defined in header `<bulk/coarray.hpp>`.

```cpp
template<typename T, typename Hub>
future<T, Hub> create_coarray(Hub& hub, local_size);
```

Alternative method of constructing a coarray.

## Template parameters

* `T` - the type of the value stored in the coarray.
* `Hub` - the type of hub to which this coarray belongs.

## Parameters

* `hub` - the hub this coarray belongs to

## Return value

A newly constructed coarray of local size `local_size`

## Complexity and cost

* **Cost** - `l`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/coarray.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto x = bulk::create_coarray<int>(hub, 10);
    });

    return 0;
}
```
