# `bulk::put`

```cpp
template <typename T, typename Hub>
void put(int processor, T value, var<T, Hub>& the_variable); // 1.

template <typename T, typename Hub>
void put(int processor, T value, array<T, Hub>& the_array, int offset = 0,
         int count = 1); // 2.
```

Put a value in a (remote) image of a *variable* (1.), or *array* (2.).

## Template parameters

* `T` - the value type of the variable/array
* `Hub` - the type of the hub that the variable/array belongs to

## Parameters

* `processor` - the index of the (remote) processor
* `value` - the value to write
* `the_variable` - the target variable
* `the_array` - the target array
* `offset` - the index of the array element to start writing at
* `count` - a number of elements to write

## Complexity and cost

* **Cost**

    1. `sizeof(T) * g`
    2. `(sizeof(T) * count) * g`

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
        bulk::put(hub.next_processor(), s, x); // 1.
        hub.sync();

        std::cout << s << " <- " << x.value() << std::endl;
    });

    return 0;
}
```
