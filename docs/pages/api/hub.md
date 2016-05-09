# `bulk::hub`

Defined in header `<bulk/hub.hpp>`.

```cpp
template<class Provider>
class hub;
```

`bulk::hub` is the central object that encapsulates the distributed system.

## Template parameters

* `Provider` - a low level implementation of the transport and system primitives. See also [`bulk::provider`](api/provider.md).

## Member types

| **Member type**                     | **Definition**                                     |
|-------------------------------------|----------------------------------------------------|
| `message_container<Tag, Content>` | `Provider::message_container_type<Tag, Content>` |

## Member functions

|                                                       |                                                       |
|-------------------------------------------------------|-------------------------------------------------------|
| **Initialization**                                    |                                                       |
| [`spawn`](hub/spawn.md)                               | spawns a spmd section on a given number of processors |
| **System information**                                |                                                       |
| [`available_processors`](hub/available_processors.md) | returns the number of available processors            |
| [`active_processors`](hub/active_processors.md)       | returns the number of active processors               |
| [`processor_id`](hub/processor_id.md)                 | returns the id of the local processor                 |
| [`next_processor`](hub/next_processor.md)             | returns the id of the next logical processor          |
| [`prev_processor`](hub/prev_processor.md)             | returns the id of the previous logical processor      |
| **Communication and coordination**                    |                                                       |
| [`sync`](hub/sync.md)                                 | performs a bulk-synchronization                       |
| [`send`](hub/send.md)                                 | sends a message to a remote processor                 |
| [`messages`](hub/messages.md)                         | returns the current message container                 |
| **Low-level access**                                  |                                                       |
| [`provider`](hub/provider.md)                         | returns the provider                                  |

## Example


```cpp
#include <iostream>

#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    });

    return 0;
}
```
