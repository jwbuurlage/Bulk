# `bulk::create_future`

Defined in header `<bulk/future.hpp>`.

```cpp
template<typename T, typename Hub>
future<T, Hub> create_future(Hub& hub);
```

Alternative method of constructing a future.

## Template parameters

* `T` - the type of the value stored in the local image of the future.
* `Hub` - the type of hub to which this future belongs.

## Parameters

* `hub` - the hub this future belongs to

## Return value

A newly constructed future

## Complexity and cost

* **Cost** - `l`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/future.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto x = bulk::create_future<int>(hub);
    });

    return 0;
}
```
