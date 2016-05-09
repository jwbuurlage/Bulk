# `bulk::hub::spawn`

```cpp
void spawn(int processors, std::function<void(int, int)> spmd);
```

Start an SPMD section on a given number of processors.

## Parameters

* `processors` - the number of processors to run the spmd section on.
* ` spmd` - the SPMD function that gets run on each (virtual) processor. The local processor id: `hub.processor_id()`, will be bassed as the first argument to `spmd`, while the number of active processors: `hub.active_processors()`, will be passed as the second argument.

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "Processor " << s << "/" << p << std::endl;
    });

    return 0;
}
```
