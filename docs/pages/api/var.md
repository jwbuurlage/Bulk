# `bulk::var`

Defined in header `<bulk/variable.hpp>`.

```cpp
template <typename T, class Hub>
class var;
```

`bulk::var` represents a distributed object with an image for each processor, that is readable and writable from remote processors.

## Template parameters

* `T` - the type of the value stored in the local image of the variable.
* `Hub` - the type of hub to which this variable belongs.

## Member functions

|                                           |                                               |
|-------------------------------------------|-----------------------------------------------|
| [(constructor)](var/constructor.md)       | constructs the variable                       |
| [(deconstructor)](var/deconstructor.md)   | deconstructs the variable                     |
| [`operator=`](var/assignment_operator.md) | assign values to the variable
| **Value access**                          |                                               |
| [`value`](var/value.md)                   | returns the value of the local variable image |
| **Hub access**                            |                                               |
| [`hub`](var/hub.md)                       | returns the hub to which the variable belongs |

## See also

- [`create_var`](var/create_var.md)

## Example


```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/variable.hpp>
#include <bulk/communication.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int) {
        auto x = bulk::create_var<int>(hub);

        bulk::put(hub.next_processor(), s, x);
        hub.sync();

        std::cout << s << " <- " << x.value() << std::endl;
    });

    return 0;
}
```
