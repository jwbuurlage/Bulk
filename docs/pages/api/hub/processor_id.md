# `bulk::hub::processor_id`

```cpp
int processor_id() const;
```

Returns the local processor id

## Return value

An `int` containing the id of the local processor.


## Note

Should only be called inside an SPMD section.

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "The local processor id is " << hub.processor_id()
                  << " == " << s << std::endl;
    });

    return 0;
}
```
