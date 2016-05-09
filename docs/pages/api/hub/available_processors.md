# `bulk::hub::available_processors`

```cpp
int available_processors() const;
```

Returns the total number of processors available on the system.

## Return value

An `int` containing the number of available processors.

## Example

```cpp
#include <iostream>

#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    std::cout << hub.available_processors() << std::endl;

    return 0;
}
```
