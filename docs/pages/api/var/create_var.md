# `bulk::create_var`

Defined in header `<bulk/variables.hpp>`.

```cpp
template<typename T, typename Hub>
var<T, Hub> create_var(Hub& hub);
```

Alternative method of constructing a variable.

## Template parameters

* `T` - the type of the value stored in the local image of the variable.
* `Hub` - the type of hub to which this variable belongs.

## Parameters

* `hub` - the hub this variable belongs to

## Return value

A newly constructed variable

## Complexity and cost

* **Cost** - `l`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        auto x = bulk::create_var<int>(hub);
    });

    return 0;
}
```
