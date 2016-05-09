# `bulk::future::value`

```cpp
T& value();
```

Returns a reference to the value held by the future

## Return value

A reference to the value

## Note

This becomes value after the next global synchronisation upon the initialization of the value of the future using e.g. `bulk::get`.

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto x = bulk::create_var<int>(hub);
        x.value() = 1234;
        hub.sync();

        auto f = bulk::get(hub.next_processor(), x);
        hub.sync();

        std::cout << f.value() << std::endl;
    });

    return 0;
}
```
