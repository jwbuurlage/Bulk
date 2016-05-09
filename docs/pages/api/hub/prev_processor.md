
# `bulk::hub::prev_processor`

```cpp
int prev_processor() const;
```

Returns the id of the previous logical processor.

## Return value

An `int` containing the id of the previous processor

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "Previous processor: " << hub.prev_processor()
                  << " == " << (s + p - 1) % p << std::endl;
    });

    return 0;
}
```
