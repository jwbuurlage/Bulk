# `bulk::world`

Defined in header `<bulk/world.hpp>`.

```cpp
class world;
```

`bulk::world` represents the world of a processor and its place within it, by providing information and mechanisms to communicate with other processors, or obtain information about the local processor.

## Member functions

|                                                   |                                                  |
|---------------------------------------------------|--------------------------------------------------|
| **System information**                            |                                                  |
| [`active_processors`](world/active_processors.md) | returns the number of active processors          |
| [`processor_id`](world/processor_id.md)           | returns the id of the local processor            |
| [`next_processor`](world/next_processor.md)       | returns the id of the next logical processor     |
| [`prev_processor`](world/prev_processor.md)       | returns the id of the previous logical processor |
| **Communication and coordination**                |                                                  |
| [`sync`](world/sync.md)                           | performs a bulk synchronization                  |
| [`barrier`](world/barrier.md)                     | performs a bulk barrier                          |

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
