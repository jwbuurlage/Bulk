# `bulk::queue`

Defined in header `<bulk/messages.hpp>`.

```cpp
template <typename T, typename... Ts>
class queue;
```

`bulk::queue` is an inbox for receiving messages passed by processors.

## Template parameters

- `T` - the type of the message content
- `Ts` - a parameter pack, optionally containing extra message content

_Note:_ content components of array type, e.g. `T[]` are also supported. They
are represented by a `std::vector<T>`.

## Member types

- `message_type`: the underlying type used by the messages. Equal to:
    - `T` if `sizeof...(Ts) == 0`,
    - `std::tuple<T, Ts...>` otherwise.
- `iterator`: the type of the iterator used for the local inbox

## Member functions

|                                           |                                               |
|-------------------------------------------|-----------------------------------------------|
| [(constructor)](var/constructor.md)       | constructs the queue                          |
| [(deconstructor)](var/deconstructor.md)   | deconstructs the queue                        |
| [`operator=`](var/assignment_operator.md) | assign the queue                              |
| **Communication**                         |                                               |
| [`operator()`](var/paren_operator.md)     | obtain a sender to a remote queue             |
| **Container**                             |                                               |
| [`begin`](queue/begin.md)                 | obtain an iterator to the start               |
| [`end`](queue/end.md)                     | obtain an iterator to the end                 |
| [`size`](queue/size.md)                   | obtain the number of messages                 |
| [`empty`](queue/empty.md)                 | check if the queue is empty                   |
| **World access**                          |                                               |
| [`world`](queue/world.md)                 | returns the world of the queue                |

## Nested classes

- [`sender`](queue/sender.md): an object providing syntactic sugar for sending messages

## Example


```cpp
#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        auto raw_queue = bulk::queue<int>(world);
        raw_queue(world.next_processor()).send(1);
        raw_queue(world.next_processor()).send(2);
        raw_queue(world.next_processor()).send(123);

        auto tuple_queue = bulk::queue<int, int, int>(world);
        tuple_queue(world.next_processor()).send(1, 2, 3);
        tuple_queue(world.next_processor()).send(4, 5, 6);

        world.sync();

        // read queue
        for (auto x : raw_queue) {
            world.log("the first queue received a message: %d", x);
        }

        for (auto [i, j, k] : tuple_queue) {
            world.log("the second queue received a tuple: (%d, %d, %d)",
                      i, j, k);
        }
    });

    return 0;
}
```
