# `bulk::hub::next_processor`

```cpp
int next_processor() const;
```

Returns the id of the next logical processor.

## Return value

An `int` containing the id of the next processor

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "Next processor: " << hub.next_processor()
                  << " == " << (s + 1) % p << std::endl;
    });

    return 0;
}
```
