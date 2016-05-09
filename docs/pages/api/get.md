# `bulk::get`

```cpp
template <typename T, typename Hub>
future<T, Hub> get(int processor, var<T, Hub>& the_variable); // 1.
```

Get a value from a (remote) image of a *variable* (1.).

## Template parameters

* `T` - the value type of the variable/array
* `Hub` - the type of the hub that the variable/array belongs to

## Parameters

* `processor` - the index of the (remote) processor
* `the_variable` - the source variable

## Complexity and cost

* **Cost**

    1. `sizeof(T) * g`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/communication.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int) {
        auto x = bulk::create_var<int>(hub);
        auto y = bulk::get(hub.next_processor(), x); // 1.

        hub.sync();

        std::cout << s << " <- " << y.value() << std::endl;
    });

    return 0;
}
```
