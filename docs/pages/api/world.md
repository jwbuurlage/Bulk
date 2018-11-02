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
| [`rank`](world/rank.md)                           | returns the id of the local processor            |
| [`next_rank`](world/next_rank.md)                 | returns the id of the next logical processor     |
| [`prev_rank`](world/prev_rank.md)                 | returns the id of the previous logical processor |
| **Communication and coordination**                |                                                  |
| [`sync`](world/sync.md)                           | performs a bulk synchronization                  |
| [`barrier`](world/barrier.md)                     | performs a bulk barrier                          |
| **Logging**                                       |                                                  |
| [`log`](world/log.md)                      | log a message                                    |
| [`log_once`](world/log_once.md)            | log a message only with the first processor |

## Example

```cpp
#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.rank();
        int p = world.active_processors();

        world.log("Hello, world! %d/%d", s, p);
    });

    return 0;
}
```
