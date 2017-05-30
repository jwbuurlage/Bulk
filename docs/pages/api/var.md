# `bulk::var`

Defined in header `<bulk/variable.hpp>`.

```cpp
template <typename T>
class var;
```

`bulk::var` represents a distributed object with an image on each processor. This image is readable and writable from remote processors.

## Template parameters

- `T` - the type of the values stored in the images of the variable.

## Member types

- `value_type`: the type of the distributed data (i.e. `T`)

## Member functions

|                                           |                                               |
|-------------------------------------------|-----------------------------------------------|
| [(constructor)](var/constructor.md)       | constructs the variable                       |
| [(deconstructor)](var/deconstructor.md)   | deconstructs the variable                     |
| [`operator=`](var/assignment_operator.md) | assign values to the variable                 |
| **Value access**                          |                                               |
| [`value`](var/value.md)                   | returns the value of the local variable image |
| [`operator T`](var/T_operator.md)         | implicit cast to value reference              |
| **Communication**                         |                                               |
| [`operator()`](var/paren_operator.md)     | obtain an image to a remote value             |
| [`broadcast`](var/broadcast.md)           | broadcast a value to all remote images        |
| **World access**                          |                                               |
| [`world`](var/world.md)                   | returns the world of the variable             |

## Nested classes

- [`image`](var/image.md): an object providing syntactic sugar for reading and writing to images. 

## Example


```cpp
#include "bulk/bulk.hpp"

#include "set_backend.hpp"

int main() {
    environment env;

    env.spawn(env.available_processors(), [](bulk::world& world) {
        int s = world.processor_id();
        int p = world.active_processors();

        bulk::var<int> a(world);

        a(world.next_processor()) = s;
        world.sync();

        world.log("%d/%d <- %d", s, p, a.value());

        auto b = a(world.next_processor()).get();

        world.sync();

        world.log("%d/%d -> %d", s, p, b.value());
    });

    return 0;
}
```
