# `bulk::class::function`

```cpp
return_type function(args);
```

Returns foo bar

## Parameters

* `foo` - a `type` of the foobar
* `bar` - a `type` of the bar foo

## Return value

A `return_type` containing foobar

## Complexity and cost

* **Complexity** - `O(n^2)`.
* **Cost** - `2g + l`

## Example

```cpp
#include <iostream>

#include <bulk/hub.hpp>
#include <bulk/bsp/bulk.hpp>


int main() {
    auto hub = bulk::hub<bulk::bsp::provider>();

    hub.spawn(hub.available_processors(), [&hub](int s, int p) {
        std::cout << "Hello, world " << s << "/" << p << std::endl;
    });

    return 0;
}
```
