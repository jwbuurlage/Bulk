# `bulk::world`

Defined in header `<bulk/world.hpp>`.

```cpp
template<class WorldProvider>
class world;
```

`bulk::world` encodes the world of a processor and its place within it, by providing information and mechanism to communicate with other processors, and obtain information about the local processor.

## Template parameters

* `WorldProvider` - a low level implementation of the local transport and system primitives. See also [`bulk::provider`](api/provider.md).

## Member types

| **Member type**                   | **Definition**                                   |
|-----------------------------------|--------------------------------------------------|
| `message_container<Tag, Content>` | `Provider::message_container_type<Tag, Content>` |
| `var_type<T>`                     | `Provider::var_type<T>`                          |
| `future_type<T>`                  | `Provider::future_type<T>`                       |
| `array_type<T>`                   | `Provider::array_type<T>`                        |
| `coarray_type<T>`                 | `Provider::coarray_type<T>`                      |

## Member functions

|                                                   |                                                  |
|---------------------------------------------------|--------------------------------------------------|
| **System information**                            |                                                  |
| [`active_processors`](world/active_processors.md) | returns the number of active processors          |
| [`processor_id`](world/processor_id.md)           | returns the id of the local processor            |
| [`next_processor`](world/next_processor.md)       | returns the id of the next logical processor     |
| [`prev_processor`](world/prev_processor.md)       | returns the id of the previous logical processor |
| **Communication and coordination**                |                                                  |
| [`sync`](world/sync.md)                           | performs a bulk-synchronization                  |
| [`send`](world/send.md)                           | sends a message to a remote processor            |
| [`messages`](world/messages.md)                   | returns the current message container            |
| **Low-level access**                              |                                                  |
| [`provider`](world/provider.md)                   | returns the provider                             |
