# `bulk::future`

Defined in header `<bulk/future.hpp>`.

```cpp
template <typename T, class Hub>
class future;
```

`bulk::future` represents a value that will become known in the upcoming superstep.

## Template parameters

* `T` - the type of the value stored in the future
* `Hub` - the type of hub to which the future belongs.

## Member functions

|                                              |                                             |
|----------------------------------------------|---------------------------------------------|
| [(constructor)](future/constructor.md)       | constructs the future                       |
| [(deconstructor)](future/deconstructor.md)   | deconstructs the future                     |
| [`operator=`](future/assignment_operator.md) | assign values to the future
| **Value access**                             |                                             |
| [`value`](future/value.md)                   | returns the value of the future             |
| **Hub access**                               |                                             |
| [`hub`](future/hub.md)                       | returns the hub to which the future belongs |

## See also

- [`create_future`](future/create_future.md)

## Example


```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/communication.hpp>
#include <bulk/bsp/bulk.hpp>
#include <bulk/util/log.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int) {
        auto a = bulk::create_var<int>(hub);

        bulk::put(hub.next_processor(), s, a);
        hub.sync();

        std::cout << s << " <- " << a.value() << std::endl;

        auto b = bulk::get(hub.next_processor(), a);

        hub.sync();

        std::cout << s << " -> " << b.value() << std::endl;
    });

    return 0;
}
```
