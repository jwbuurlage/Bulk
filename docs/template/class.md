# `bulk::class`

Defined in header `<bulk/class.hpp>`.

```cpp
class hub;
```

`bulk::class` has a description

## Member types

| **Member type** | **Definition** |
|-----------------|----------------|
| `value_type`    | T              |

## Member functions

| **FooBat**            |          |
|-----------------------|----------|
| [`foo`](class/foo.md) | does foo |

## Example

```cpp
#include <iostream>

#include <bulk/bsp/class.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    });

    return 0;
}
```
