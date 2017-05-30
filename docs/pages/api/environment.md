# `bulk::environment`

Defined in header `<bulk/environment.hpp>`.

```cpp
class environment;
```

`bulk::environment` encodes the environment of a parallel layer, and provides information on the system.

## Member functions

|                                                               |                                                       |
|---------------------------------------------------------------|-------------------------------------------------------|
| **Initialization**                                            |                                                       |
| [`spawn`](environment/spawn.md)                               | spawns a spmd section on a given number of processors |
| **System information**                                        |                                                       |
| [`available_processors`](environment/available_processors.md) | returns the number of available processors            |

## Example

```cpp
#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.processor_id();
        int p = world.active_processors();

        world.log("Hello, world %d/%d", s, p);
    });

    return 0;
}
```
